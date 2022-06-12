#include "Epoll.h"
#include "../socket/Socket.h"
#include <signal.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>
#include <map>
using namespace std;

void err_exit(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

int main()
{
    map<int, TCPClient *> client_map;
    signal(SIGPIPE, SIG_IGN);
    char buf[BUFSIZ];
    int clientCount = 0;
    TCPServer server(8001);
    int listenfd = server.getfd();
    Epoll epoll;
    epoll.addfd(server.getfd(), EPOLLIN, true);
    while (true)
    {
        int nReady = epoll.wait();
        for (int i = 0; i < nReady; ++i)
            if (epoll.getEventOccurfd(i) == listenfd)
            {
                int connectfd = accept(listenfd, NULL, NULL);
                if (connectfd == -1)
                    err_exit("accept error");
                cout << "accept success..." << endl;
                cout << "clientCount = " << ++ clientCount << endl;
                setUnBlock(connectfd, true);
                epoll.addfd(connectfd, EPOLLIN, true);
            }
            else if (epoll.getEvents(i) & EPOLLIN)
            {
                int in_fd = epoll.getEventOccurfd(i);
                auto iter = client_map.find(in_fd);
                TCPClient *client;
                if (iter == client_map.end()){
                    client = new TCPClient(epoll.getEventOccurfd(i));
                    client_map.insert(make_pair(in_fd, client));
                }
                else {
                    client = iter->second;
                }
                memset(buf, 0, sizeof(buf));
                if (client->read(buf, sizeof(buf)) == 0)
                {
                    cerr << "client " << client->getfd() <<" connect closed" << endl;
                    epoll.delfd(client->getfd());
                    delete client;
                    continue;
                }
                cout << "msg from client "<< client->getfd() << ":" << buf << endl;
                client->write(buf);
            }
    }
    
}