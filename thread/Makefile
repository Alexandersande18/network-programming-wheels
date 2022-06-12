objects := thread threadpool 
cxxflags := -std=c++11 -lpthread -w 
# -Wint-to-pointer-cast -Wdelete-incomplete -Wint-to-pointer-cast -Wint-to-pointer-cast -Wdelete-incomplete
all: $(objects)
thread: TestThread.cpp Task.h Thread.h 
	g++ ./TestThread.cpp $(cxxflags) -o thread
threadpool: TestThreadPool.cpp Task.h Thread.h ThreadPool.h
	g++ ./TestThreadPool.cpp $(cxxflags) -o threadpool


.PHONY: clean
clean:
	rm $(objects)