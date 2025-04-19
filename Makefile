CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter -Wno-reorder -Wno-sign-compare
LDFLAGS = -lboost_system -lboost_serialization -lboost_filesystem -lboost_regex -lreadline

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Find all .cc files recursively under src/
SRCS = $(shell find $(SRC_DIR) -name '*.cc')

# Map src/foo/bar.cc -> obj/foo/bar.o
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = $(BIN_DIR)/HackyDb

all: directories $(TARGET)

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(sort $(dir $(OBJS)))

# Link all object files into binary
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .cc to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories
