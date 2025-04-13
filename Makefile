CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = BufferPoolManager

SRC = src/BufferPoolManager/BufferPoolManager.cpp
OBJ = build/BufferPoolManager.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

$(OBJ): $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)
	
clean:
	rm -rf build $(TARGET)

.PHONY: all clean
