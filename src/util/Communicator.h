#ifndef NETWORK_COMMUNICATOR_H
#define NETWORK_COMMUNICATOR_H

#include <string>
#include <vector>
#include <cstdint>
#include <sys/uio.h>

namespace Network {

    class Communicator {
    public:
        Communicator(const std::string& hostname, std::uint16_t port);
        ~Communicator();

        // Disable copy operations
        Communicator(const Communicator&) = delete;
        Communicator& operator=(const Communicator&) = delete;

        // Main communication methods
        void send_initial_block(const std::vector<uint8_t>& block);
        void send_q_blocks(uint16_t l, const std::vector<uint8_t>& q_blocks);
        void send_multiple_q_blocks(const std::vector<std::pair<uint16_t, std::vector<uint8_t>>>& blocks);
        std::vector<uint8_t> receive_responses(size_t num_responses);
        void cork_socket(bool enable) const;

    private:
        void connect_to_server(const std::string& hostname, uint16_t port);
        void optimize_socket();
        void set_tcp_nodelay() const;
        void optimize_socket_buffers();
        void write_all(const void* buf, size_t count) const;
        void write_all_iov(struct iovec* iov, size_t iov_count) const;
        void read_all(void* buf, size_t count) const;


        int m_fd;
        static constexpr size_t BUFFER_SIZE = 256 * 16 + 64;  // 256KB buffer size
        alignas(64) std::vector<uint8_t> m_read_buffer;    // Aligned read buffer
    };

} // namespace Network

#endif // NETWORK_COMMUNICATOR_H