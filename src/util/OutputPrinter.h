#ifndef OUTPUTPRINTER_H
#define OUTPUTPRINTER_H

#include <cstdint>
#include <string>
#include <vector>
#include <unistd.h>
#include <array>

class OutputPrinter {
public:
    OutputPrinter(size_t estimatedSize);
    ~OutputPrinter();

    OutputPrinter& operator+=(const std::string& str);
    OutputPrinter& operator+=(const char* str);
    OutputPrinter& operator+=(uint8_t c);
    OutputPrinter& operator+=(int val);
    OutputPrinter& operator+=(bool val);
    OutputPrinter& operator+=(const std::vector<std::string>& vec);
    OutputPrinter& operator+=(const std::vector<uint8_t>& vec);


    void add(const std::array<uint8_t, 64>& arr, size_t len);
    void set(uint8_t val);
    void print() const;
    void reserve(size_t size);
    void clear();
    size_t get_length() const;

private:
    uint8_t* buffer;
    size_t capacity;
    size_t length;
};

#endif // OUTPUTPRINTER_H