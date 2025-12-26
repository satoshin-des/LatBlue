CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -O3 -flto -fopenmp -march=native -funroll-loops -ffast-math -Wall -Iinclude
LDFLAGS = -lntl -mwindows

MAIN = Main.cpp

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o) Main.o

TARGET = test.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -s -g -o $@ $^ $(LDFLAGS)

Main.o: Main.cpp
	$(CXX) $(CXXFLAGS) -c $< -g -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -g -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
