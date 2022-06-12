#include "../socket/Socket.h"
#include <iostream>
using namespace std;

int main()
{
    TCPClient client(8001, "127.0.0.1");
    char buf[BUFSIZ];
    while (scanf("%s", buf) != EOF)
    {
        write(client.getfd(), buf, strlen(buf));
        memset(buf, 0, sizeof(buf));
        read(client.getfd(), buf, sizeof(buf));
        cout << "echo from server: "<<buf<< endl;
        memset(buf, 0, sizeof(buf));
    }
}