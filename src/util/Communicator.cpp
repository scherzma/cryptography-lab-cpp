#include "Communicator.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>
#include <cstring>
#include <memory>

namespace Network {

Communicator::Communicator(const std::string &hostname, std::uint16_t port)
    : m_fd(-1)
    , m_read_buffer(BUFFER_SIZE) {  // Pre-allocate read buffer
    connect_to_server(hostname, port);
}

Communicator::~Communicator() {
    if (m_fd >= 0) {
        close(m_fd);
    }
}

void Communicator::cork_socket(bool enable) const {
#ifdef TCP_CORK
    int option_value = enable ? 1 : 0;
    setsockopt(m_fd, IPPROTO_TCP, TCP_CORK, &option_value, sizeof(option_value));
#elif defined(TCP_NOPUSH)
    int option_value = enable ? 1 : 0;
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_NOPUSH, &option_value, sizeof(option_value)) < 0) {
        std::cerr << "Warning: Could not set TCP_NOPUSH option. Performance might be affected." << std::endl;
    }
#endif
}


void Communicator::connect_to_server(const std::string &hostname, uint16_t port) {
    struct addrinfo hints{}, *raw_resolved = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // Prefer IPv4 for slightly better performance
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME | AI_PASSIVE;

    getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &raw_resolved);
    std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> resolved(raw_resolved, &freeaddrinfo);

    for (struct addrinfo *attempt = resolved.get(); attempt != nullptr; attempt = attempt->ai_next) {
        int s = socket(attempt->ai_family, attempt->ai_socktype, attempt->ai_protocol);
        if (s < 0) continue;

        if (connect(s, attempt->ai_addr, attempt->ai_addrlen) == 0) {
            m_fd = s;
            optimize_socket();
            return;
        }
        close(s);
    }
}

void Communicator::optimize_socket() {
    // Disable Nagle's algorithm
    set_tcp_nodelay();

    // Enable TCP Quick ACK
    int quick_ack = 1;
    setsockopt(m_fd, IPPROTO_TCP, TCP_QUICKACK, &quick_ack, sizeof(quick_ack));

    // Enable TCP Fast Open if available
    #ifdef TCP_FASTOPEN
    int fastopen = 1;
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_FASTOPEN, &fastopen, sizeof(fastopen)) < 0) {
        // TCP Fast Open might not be available - ignore error
    }
    #endif

    optimize_socket_buffers();
}

void Communicator::optimize_socket_buffers() {
    int send_size = BUFFER_SIZE;
    int recv_size = BUFFER_SIZE;

    setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &send_size, sizeof(send_size));
    setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, &recv_size, sizeof(recv_size));
}

void Communicator::set_tcp_nodelay() const {
    int flag = 1;
    setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

void Communicator::send_initial_block(const std::vector<uint8_t>& block) {
    write_all(block.data(), block.size());
}

void Communicator::send_q_blocks(uint16_t l, const std::vector<uint8_t>& q_blocks) {
    uint8_t length_field[2];
    length_field[0] = static_cast<uint8_t>(l & 0xFF);
    length_field[1] = static_cast<uint8_t>((l >> 8) & 0xFF);

    struct iovec iov[2];
    iov[0].iov_base = length_field;
    iov[0].iov_len = 2;
    iov[1].iov_base = const_cast<uint8_t*>(q_blocks.data());
    iov[1].iov_len = q_blocks.size();

    write_all_iov(iov, 2);
}

    void Communicator::send_multiple_q_blocks(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>>& blocks) {
    // Enable TCP_CORK
    cork_socket(true);

    std::vector<struct iovec> iovecs;
    iovecs.reserve(blocks.size() * 2);

    std::vector<uint8_t> length_fields;
    length_fields.reserve(blocks.size() * 2);

    for (const auto& [l, q_block] : blocks) {
        length_fields.push_back(static_cast<uint8_t>(l & 0xFF));
        length_fields.push_back(static_cast<uint8_t>((l >> 8) & 0xFF));

        struct iovec length_iov = {
            .iov_base = &length_fields[length_fields.size() - 2],
            .iov_len = 2
        };
        struct iovec data_iov = {
            .iov_base = const_cast<uint8_t*>(q_block.data()),
            .iov_len = q_block.size()
        };

        iovecs.push_back(length_iov);
        iovecs.push_back(data_iov);
    }

    write_all_iov(iovecs.data(), iovecs.size());

    // Disable TCP_CORK
    cork_socket(false);
}

std::vector<uint8_t> Communicator::receive_responses(size_t num_responses) {
    // Ensure buffer is large enough
    if (num_responses > m_read_buffer.size()) {
        m_read_buffer.resize(num_responses);
    }

    read_all(m_read_buffer.data(), num_responses);
    return std::vector<uint8_t>(m_read_buffer.begin(), m_read_buffer.begin() + num_responses);
}

    void Communicator::write_all(const void* buf, size_t count) const {
    // Enable TCP_CORK (or TCP_NOPUSH)
    cork_socket(true);

    const auto* ptr = static_cast<const uint8_t*>(buf);
    size_t total_written = 0;

    while (total_written < count) {
        ssize_t n = ::write(m_fd, ptr + total_written, count - total_written);
        if (n > 0) {
            total_written += n;
        }
    }

    // Disable TCP_CORK (or TCP_NOPUSH)
    cork_socket(false);
}

void Communicator::write_all_iov(struct iovec* iov, size_t iov_count) const {
    size_t total_bytes = 0;
    for (size_t i = 0; i < iov_count; ++i) {
        total_bytes += iov[i].iov_len;
    }

    size_t written = 0;
    while (written < total_bytes) {
        ssize_t n = ::writev(m_fd, iov, iov_count);
        if (n <= 0) {
            if (errno == EINTR) continue;
        }
        written += n;

        auto remaining = static_cast<size_t>(n);
        for (size_t i = 0; i < iov_count && remaining > 0; ++i) {
            if (iov[i].iov_len <= remaining) {
                remaining -= iov[i].iov_len;
                iov[i].iov_len = 0;
                iov[i].iov_base = nullptr;
            } else {
                iov[i].iov_base = static_cast<uint8_t*>(iov[i].iov_base) + remaining;
                iov[i].iov_len -= remaining;
                remaining = 0;
            }
        }
    }
}

void Communicator::read_all(void* buf, size_t count) const {
    auto* ptr = static_cast<uint8_t*>(buf);
    size_t total_read = 0;

    while (total_read < count) {
        ssize_t n = ::read(m_fd, ptr + total_read, count - total_read);
        if (n <= 0) {
            if (errno == EINTR) continue;
        }
        total_read += n;
    }
}

} // namespace Network