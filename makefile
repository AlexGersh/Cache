CXX = g++

CXXFLAGS= -std=c++17 -g

TARGET = cacheSim
DEBUG = $(TARGET)Debug 
SRCS = cacheSim.cpp cache_engine.cpp
OBJS = $(SRCS:.cpp=.o)

all : $(TARGET) $(DEBUG)  

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) cacheSim.cpp -o $(TARGET)

$(DEBUG): $(SRCS)

	$(CXX) $(CXXFLAGS) -DDEBUG cacheSim.cpp -o $(DEBUG)


.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET) 
