objects := client server 
cxxflags := -std=c++11 -lpthread -w 
all: $(objects)
client: TestClient.cpp Reactor.h ReactorSingleton.h HeapTimer.h
	g++ ./TestClient.cpp $(cxxflags) -o client
server: TestServer.cpp Reactor.h ReactorSingleton.h HeapTimer.h
	g++ ./TestServer.cpp $(cxxflags) -o server


.PHONY: clean
clean:
	rm $(objects)