#include <iostream>
#include "Socket.h"
using namespace std;
int main()
{
    TCPClient client(8000, "127.0.0.1");
    char buf[BUFSIZ];
    while(scanf("%s", buf) != EOF)
    {
        client.write(buf, sizeof(buf));
        memset(buf, 0, sizeof(buf));
        client.read(buf, sizeof(buf));
        cout << "echo from server:"<< buf << endl;
        memset(buf, 0, sizeof(buf));
    }
}