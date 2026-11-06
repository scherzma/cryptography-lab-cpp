#include "OutputPrinter.h"
#include <iostream>
#include <cstring>
#include <algorithm>

OutputPrinter::OutputPrinter(size_t estimatedSize) : capacity(estimatedSize), length(0) {
    buffer = new uint8_t[capacity];
    buffer[0] = '\0';
}

OutputPrinter::~OutputPrinter() {
    delete[] buffer;
}

void OutputPrinter::reserve(size_t size) {
    if (size > capacity) {
        uint8_t* newBuffer = new uint8_t[size];
        std::memcpy(newBuffer, buffer, length);
        delete[] buffer;
        buffer = newBuffer;
        capacity = size;
    }
}

void OutputPrinter::clear() {
    length = 0;
}

size_t OutputPrinter::get_length() const {
    return length;
}

OutputPrinter& OutputPrinter::operator+=(const std::string& str) {
    return *this += str.c_str();
}

OutputPrinter& OutputPrinter::operator+=(const char* str) {
    size_t strLength = std::strlen(str);
    if (length + strLength + 1 > capacity) {
        reserve(std::max(capacity * 2, length + strLength + 1));
    }
    std::memcpy(buffer + length, str, strLength);
    length += strLength;
    buffer[length] = '\0';
    return *this;
}

OutputPrinter& OutputPrinter::operator+=(uint8_t c) {
    if (length + 2 > capacity) {
        reserve(capacity * 2);
    }
    buffer[length++] = c;
    buffer[length] = '\0';
    return *this;
}

OutputPrinter& OutputPrinter::operator+=(int val) {
    return *this += std::to_string(val);
}

OutputPrinter& OutputPrinter::operator+=(bool val) {
    return *this += (val ? "true" : "false");
}

OutputPrinter& OutputPrinter::operator+=(const std::vector<std::string>& vec) {
    *this += "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        *this += "\"";
        *this += vec[i];
        *this += "\"";
        if (i < vec.size() - 1) {
            *this += ",";
        }
    }
    *this += "]";
    return *this;
}

OutputPrinter& OutputPrinter::operator+=(const std::vector<uint8_t>& vec) {
    *this += "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        *this += std::to_string(vec[i]);
        if (i < vec.size() - 1) {
            *this += ",";
        }
    }
    *this += "]";
    return *this;
}

void OutputPrinter::add(const std::array<uint8_t, 64>& arr, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        *this += static_cast<uint8_t>(arr[i]);
    }
}

void OutputPrinter::set(uint8_t val) {
    buffer[length - 1] = val;
}

void OutputPrinter::print() const {
    write(STDOUT_FILENO, buffer, length);
    write(STDOUT_FILENO, "\n", 1);
}