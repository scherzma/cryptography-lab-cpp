//
// Created by user on 26/12/24.
//

#ifndef INPUTFUNCTIONPARSER_H
#define INPUTFUNCTIONPARSER_H

#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "OutputPrinter.h"

static constexpr size_t BUFFER_SIZE = 2048;
extern uint8_t file_read_buffer[BUFFER_SIZE];

constexpr uint64_t hash_arr(std::array<char, 17> str, size_t n);
constexpr uint64_t hash(const char* str, size_t n);
constexpr uint64_t operator""_h(const char* str, size_t n);

bool fill_buffer(FILE& file, size_t& pos, size_t& bytesRead);
void process_block(std::vector<uint8_t>& content, OutputPrinter& output);
void parse_input_file(const char* filename, OutputPrinter& output);



#endif //INPUTFUNCTIONPARSER_H
