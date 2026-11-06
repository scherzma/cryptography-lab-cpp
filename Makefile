CXX = g++
# Profile directory
PROFILE_DIR = profile
# Profile flags
PROFILE_GEN_FLAGS = -fprofile-generate=$(PROFILE_DIR)
PROFILE_USE_FLAGS = -fprofile-use=$(PROFILE_DIR)

CXXFLAGS = -std=c++23 -O3 -march=native -flto -maes \
           -ffast-math -mavx2 -mfma -msse4.2 -mpclmul \
           -funroll-loops -fomit-frame-pointer \
           -finline-functions -finline-limit=1000 \
           -fno-stack-protector \
           -fno-stack-check \
           -fno-strict-aliasing \
           -fno-rtti \
           -fno-exceptions \
           -fno-asynchronous-unwind-tables \
           -fno-semantic-interposition \
           -fno-plt \
           -s

INCLUDES = -I. -I./src

# Output binary            -fno-exceptions
TARGET = building/KryptoCpp

# Source files
SRCS = src/algorithms/ddf.cpp \
       src/algorithms/edf.cpp \
       src/algorithms/gcm.cpp \
       src/algorithms/padding_oracle.cpp \
       src/algorithms/sea128.cpp \
       src/algorithms/ssf.cpp \
       src/algorithms/xex_fde.cpp \
       src/algorithms/gcm_crack.cpp \
       src/main.cpp \
       src/math/Element.cpp \
       src/math/Polynom.cpp \
       src/util/base64_utils.cpp \
       src/util/process_functions.cpp \
       src/util/Block.cpp \
       src/util/helper_functinos.cpp \
       src/util/Communicator.cpp \
       src/util/OutputPrinter.cpp \
       src/util/input_file_parser.cpp \
       src/util/action_parsers.cpp

# Object files
OBJS = $(SRCS:%.cpp=building/%.o)

# Make sure build directories exist
BUILD_DIRS = building/src/algorithms building/src/math building/src/util

# Default target
all: $(BUILD_DIRS) $(TARGET)

# Profile generation build
profile-gen: CXXFLAGS += $(PROFILE_GEN_FLAGS)
profile-gen: clean all
	@echo "Run your program with typical workload to generate profile data"

# Profile-guided optimization build
profile-use: CXXFLAGS += $(PROFILE_USE_FLAGS)
profile-use: clean all

# Profile-guided optimization workflow
pgo: profile-gen
	./$(TARGET)
	$(MAKE) profile-use

# Create build directories
$(BUILD_DIRS):
	mkdir -p $@

# Link the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LIBS) \
	-Wl,-O3 -Wl,--as-needed -Wl,--gc-sections -Wl,-z,norelro

# Compile source files
building/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf building $(PROFILE_DIR)

# Phony targets
.PHONY: all clean profile-gen profile-use pgo

# Generate dependencies automatically
DEPS = $(OBJS:.o=.d)
-include $(DEPS)