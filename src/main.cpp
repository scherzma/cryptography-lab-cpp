#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <chrono>

#include "util/input_file_parser.h"

namespace fs = std::filesystem;
using namespace std::chrono;

int main(int argc, char* argv[]) {
    std::srand(54321); // 54321 0.9 edf 0.1 123456 0.9 edf 0.2 98765 0.9 edf 0.1 5647383 0.9 edf 0.2 35702139 0.8 edf 0.1

    OutputPrinter output(2048);

    bool mode = argc < 2;
    if(mode) {
        const std::string directory = "../testcases/";

        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".json") {
                std::string file = entry.path().string();
                if (file.contains("output")) {
                    continue;
                }
                int results_size = 0;
                for(int i = 0; i < 3; ++i) {
                    output.clear();
                    parse_input_file(file.c_str(), output);
                    results_size += output.get_length();
                }

                //output.print();
            }
        }

    } else {
        std::string file = argv[1];
        parse_input_file(file.c_str(), output);
        output.print();
    }


    return 0;
}