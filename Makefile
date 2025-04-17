CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -arch arm64 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lboost_filesystem -lboost_regex
TARGET = BufferPoolManagerTest

SRC = \
	src/BufferPoolManager/BufferPoolManager.cpp \
	src/TableHeap/TableHeap.cpp \
	src/CatalogManager/CatalogManager.cpp \
	src/RecordManager/RecordManager.cpp \
	src/test.cpp

OBJ = $(SRC:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	chmod +x ./$(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)

.PHONY: all clean run
