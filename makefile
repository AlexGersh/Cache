

cacheSim: cacheSim.cpp cache_engine.cpp
	g++ -o cache_engine cache_engine.cpp cache_engine.cpp
.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim
