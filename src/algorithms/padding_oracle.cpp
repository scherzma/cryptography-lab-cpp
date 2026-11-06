#include "padding_oracle.h"
#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <algorithm>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "util/Communicator.h"

const int NUM_THREADS = 6;

class ThreadSafeQueue {
private:
    std::queue<std::pair<size_t, std::vector<uint8_t>>> queue;
    std::mutex mutex;
    std::condition_variable cond;
    bool done;

public:
    ThreadSafeQueue() : done(false) {}

    void push(size_t block_index, const std::vector<uint8_t>& block) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push({block_index, block});
        cond.notify_one();
    }

    bool pop(size_t& block_index, std::vector<uint8_t>& block) {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.empty() && done) return false;
        if (queue.empty()) {
            cond.wait(lock, [this] { return !queue.empty() || done; });
            if (queue.empty()) return false;
        }

        block_index = queue.front().first;
        block = std::move(queue.front().second);
        queue.pop();
        return true;
    }

    void finish() {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
        cond.notify_all();
    }
};

std::array<std::array<uint8_t, 16>, 256> gen_ivs(int current_byte, const std::vector<uint8_t>& known_iv) {
    std::array<std::array<uint8_t, 16>, 256> ivs = {};
    for (int i = 0; i < 256; ++i) {
        ivs[i][current_byte] = static_cast<uint8_t>(i);
        for (size_t k = 0; k < known_iv.size(); ++k) {
            ivs[i][current_byte + 1 + k] = known_iv[k];
        }
    }
    return ivs;
}

std::vector<std::array<uint8_t, 16>> gen_alterations(const std::vector<uint8_t>& candidates, int current_byte, const std::vector<uint8_t>& known_iv) {
    std::vector<std::array<uint8_t, 16>> ivs(candidates.size());
    for (size_t i = 0; i < candidates.size(); ++i) {
        ivs[i][current_byte] = candidates[i];
        ivs[i][current_byte - 1]++;
        for (size_t k = 0; k < known_iv.size(); ++k) {
            ivs[i][current_byte + 1 + k] = known_iv[k];
        }
    }
    return ivs;
}

std::vector<uint8_t> get_candidates(const std::vector<uint8_t>& responses) {
    std::vector<uint8_t> candidates;
    candidates.reserve(responses.size());
    for (size_t i = 0; i < responses.size(); ++i) {
        if (responses[i] == 0x01) {
            candidates.push_back(static_cast<uint8_t>(i));
        }
    }
    return candidates;
}

void worker_thread(ThreadSafeQueue& work_queue,
                  std::vector<std::pair<size_t, std::vector<uint8_t>>>& results,
                  std::mutex& results_mutex,
                  const std::string& hostname,
                  int port,
                  const std::vector<uint8_t>& previous_blocks) {
    size_t block_index;
    std::vector<uint8_t> block;

    while (work_queue.pop(block_index, block)) {
        std::vector<uint8_t> previous_block(
            previous_blocks.begin() + block_index * 16,
            previous_blocks.begin() + (block_index + 1) * 16
        );

        auto decrypted_block = attack_block(hostname, port, block, previous_block);

        std::lock_guard<std::mutex> lock(results_mutex);
        results.emplace_back(block_index, decrypted_block);
    }
}

const std::array<uint8_t, 16> PADDING_VALUES = {
    0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
    0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
};

std::vector<uint8_t> attack_block(const std::string& hostname, int port,
                                const std::vector<uint8_t>& block,
                                const std::vector<uint8_t>& previous_block) {
    Network::Communicator comm(hostname, port);
    comm.send_initial_block(block);

    std::array<uint8_t, 16> plaintext_block = {};
    std::vector<uint8_t> x_values;
    x_values.reserve(16);

    for (int current_byte = 15; current_byte >= 0; --current_byte) {
        std::vector<uint8_t> known_iv;
        known_iv.reserve(15 - current_byte);
        for (size_t i = 0; i < 15 - current_byte; ++i) {
            known_iv.push_back(x_values[i] ^ static_cast<uint8_t>(16 - current_byte));
        }

        uint8_t padding_value = static_cast<uint8_t>(16 - current_byte);
        auto ivs = gen_ivs(current_byte, known_iv);

        std::vector<uint8_t> iv_flat;
        iv_flat.reserve(256 * 16);
        for (const auto& iv : ivs) {
            iv_flat.insert(iv_flat.end(), iv.begin(), iv.end());
        }

        comm.send_q_blocks(256, iv_flat);
        std::vector<uint8_t> responses = comm.receive_responses(256);
        std::vector<uint8_t> candidates = get_candidates(responses);

        if (candidates.size() > 1) {
            auto alterations = gen_alterations(candidates, current_byte, known_iv);

            std::vector<uint8_t> alt_flat;
            alt_flat.reserve(candidates.size() * 16);
            for (size_t i = 0; i < candidates.size(); ++i) {
                alt_flat.insert(alt_flat.end(), alterations[i].begin(), alterations[i].end());
            }

            comm.send_q_blocks(candidates.size(), alt_flat);
            responses = comm.receive_responses(candidates.size());
            for (size_t i = 0; i < responses.size(); ++i) {
                if (responses[i] == 0x01) {
                    candidates[0] = candidates[i];
                    break;
                }
            }
        }

        uint8_t xi = candidates[0] ^ padding_value;
        x_values.insert(x_values.begin(), xi);
        plaintext_block[current_byte] = xi ^ previous_block[current_byte];
    }

    comm.send_q_blocks(0, {0, 0});
    return std::vector<uint8_t>(plaintext_block.begin(), plaintext_block.end());
}

std::vector<uint8_t> padding_attack(const std::string& hostname, int port,
                                  const std::array<uint8_t, 16>& iv,
                                  const std::vector<uint8_t>& ciphertext) {
    std::vector<uint8_t> text;
    text.reserve(iv.size() + ciphertext.size());
    text.insert(text.end(), iv.begin(), iv.end());
    text.insert(text.end(), ciphertext.begin(), ciphertext.end());
    size_t num_blocks = (text.size() / 16) - 1;

    ThreadSafeQueue work_queue;
    std::vector<std::pair<size_t, std::vector<uint8_t>>> results;
    std::mutex results_mutex;

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker_thread,
                           std::ref(work_queue),
                           std::ref(results),
                           std::ref(results_mutex),
                           std::ref(hostname),
                           port,
                           std::ref(text));
    }

    for (size_t i = 0; i < num_blocks; ++i) {
        std::vector<uint8_t> block_to_decrypt(
            text.begin() + (i + 1) * 16,
            text.begin() + (i + 2) * 16
        );
        work_queue.push(i, block_to_decrypt);
    }

    work_queue.finish();
    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<uint8_t> plaintext;
    plaintext.reserve(results.size() * 16);
    std::ranges::sort(results);
    for (const auto& result : results) {
        plaintext.insert(plaintext.end(), result.second.begin(), result.second.end());
    }

    return plaintext;
}