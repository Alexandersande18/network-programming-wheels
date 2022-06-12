objects := client server 
all: $(objects)
client: TestClient.cpp Epoll.h
	g++ ./TestClient.cpp -std=c++11 -o client
server: TestServer.cpp Epoll.h
	g++ ./TestServer.cpp -std=c++11 -o server


.PHONY: clean
clean:
	rm $(objects)