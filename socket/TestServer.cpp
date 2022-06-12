#include <iostream>

#include "Socket.h"

using namespace std;

int main()
{
    char buf[BUFSIZ];
    int listenfd;
    TCPClient client;
    TCPServer server(8000);
    listenfd = server.getfd();
    while(true)
    {
        TCPClient *client = new TCPClient(server.accept());
        cout << client << endl;
        while (1) {
            memset(buf, 0, sizeof(buf));
            client->read(buf, sizeof(buf));
            cout << "client: " <<buf << endl;
            client->write(buf, sizeof(buf));
        }
    }
}