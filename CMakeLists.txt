cmake_minimum_required(VERSION 3.10)
project(KryptoCpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BOTAN_LIBRARIES "botan-2")

# Find Botan
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -g -pg")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -pg -pthread")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")
set(TARGET "KryptoCpp" PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#set("-O3")

# Find OpenSSL

find_package(PkgConfig REQUIRED)
# pkg_check_modules(BOTAN REQUIRED botan-2)

# Recursively find all .cpp files in src/ and its subdirectories
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable
add_executable(KryptoCpp ${SOURCES}
        src/util/base64_utils.h
        src/util/Block.h
        src/util/semantic.h
        src/util/process_functions.h
        src/math/Element.h
        src/util/helper_functions.h
        src/algorithms/sea128.cpp
        src/algorithms/sea128.h
        src/algorithms/xex_fde.cpp
        src/algorithms/xex_fde.h
        src/algorithms/padding_oracle.cpp
        src/algorithms/padding_oracle.h
        src/util/Communicator.cpp
        src/util/Communicator.h
        src/algorithms/gcm.cpp
        src/algorithms/gcm.h
        src/util/helper_functinos.cpp
        src/math/Polynom.cpp
        src/math/Polynom.h
        src/algorithms/ssf.cpp
        src/algorithms/ssf.h
        src/algorithms/ddf.cpp
        src/algorithms/ddf.h
        src/algorithms/edf.cpp
        src/algorithms/edf.h
        src/algorithms/gcm_crack.cpp
        src/algorithms/gcm_crack.h
        src/util/OutputPrinter.cpp
        src/util/OutputPrinter.h
        src/util/input_file_parser.cpp
        src/util/input_file_parser.h
        src/util/action_parsers.cpp
        src/util/action_parsers.h)

# Specify include directories for the target
target_include_directories(KryptoCpp PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external/json/include  # Adjust if you have external dependencies
)



# Link libraries
target_link_libraries(KryptoCpp PRIVATE)
target_compile_options(KryptoCpp PRIVATE -msse4.1 -mpclmul -maes -msse4 -march=native -flto -maes)