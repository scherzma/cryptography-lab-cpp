#ifndef PADDING_ORACLE_H
#define PADDING_ORACLE_H

#include <vector>
#include <string>
#include <cstdint>

std::vector<uint8_t> padding_attack(const std::string& hostname, int port, const std::array<uint8_t, 16>& iv, const std::vector<uint8_t>& ciphertext);

std::vector<uint8_t> de_pad(const std::vector<uint8_t>& plaintext);


std::vector<uint8_t> attack_block(const std::string& hostname, int port,
                                  const std::vector<uint8_t>& block,
                                  const std::vector<uint8_t>& previous_block);








#endif
