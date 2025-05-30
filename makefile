
CXX = g++

CXXFLAGS= -std=c++17 -g

TARGET = cacheSim

SRCS = cacheSim.cpp cache_engine.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS) 
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(TARGET) 
