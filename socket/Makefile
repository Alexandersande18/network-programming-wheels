objects := client server 
all: $(objects)
client: TestClient.cpp Socket.h
	g++ ./TestClient.cpp -std=c++11 -o client
server: TestServer.cpp Socket.h
	g++ ./TestServer.cpp -std=c++11 -o server

.PHONY: clean
clean:
	rm $(objects)