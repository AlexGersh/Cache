
CXX = g++

CXXFLAGS= -std=c++17 -g

TARGET = cacheSim

SRCS = cacheSim.cpp cache_engine.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) cacheSim.cpp -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET) 
