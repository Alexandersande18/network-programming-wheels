#ifndef EPOLL_H
#define EPOLL_H

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <strings.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <cstring>

class EpollException
{
public:
    typedef std::string string;
    EpollException(const string &_msg = string())
        : msg(_msg) {}
    string what() const
    {
        if (errno == 0)
            return msg;
        return msg + ": " + strerror(errno);
    }

private:
    string msg;
};

class Epoll
{
public:
    Epoll(int flags = EPOLL_CLOEXEC, int noFile = 1024);
    ~Epoll();

    void addfd(int fd, uint32_t events = EPOLLIN, bool ETorNot = false);
    void modfd(int fd, uint32_t events = EPOLLIN, bool ETorNot = false);
    void delfd(int fd);
    int wait(int timeout = -1);
    int getEventOccurfd(int eventIndex) const;
    uint32_t getEvents(int eventIndex) const;

public:
    bool isValid()
    {
        if (m_epollfd == -1)
            return false;
        return true;
    }
    void close()
    {
        if (isValid())
        {
            :: close(m_epollfd);
            m_epollfd = -1;
        }
    }

private:
    std::vector<struct epoll_event> events;
    int m_epollfd;
    int fdNumber;
    int nReady;
private:
    struct epoll_event event;
};

Epoll::Epoll(int flags, int noFile) : fdNumber(0), nReady(0)
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = noFile;
    if ( ::setrlimit(RLIMIT_NOFILE, &rlim) == -1 )
        throw EpollException("setrlimit error");

    m_epollfd = epoll_create1(flags);
    if (m_epollfd == -1)
        throw EpollException("epoll_create1 error");
}

Epoll::~Epoll()
{
    this -> close();
}

/** epoll_ctl **/
void Epoll::addfd(int fd, uint32_t events, bool ETorNot)
{
    bzero(&event, sizeof(event));
    event.events = events;
    if (ETorNot)
        event.events |= EPOLLET;
    event.data.fd = fd;
    if( ::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event) == -1 )
        throw EpollException("epoll_ctl_add error");
    ++ fdNumber;
}

void Epoll::modfd(int fd, uint32_t events, bool ETorNot)
{
    bzero(&event, sizeof(event));
    event.events = events;
    if (ETorNot)
        event.events |= EPOLLET;
    event.data.fd = fd;
    if( ::epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event) == -1 )
        throw EpollException("epoll_ctl_mod error");
}

void Epoll::delfd(int fd)
{
    bzero(&event, sizeof(event));
    event.data.fd = fd;
    if( ::epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &event) == -1 )
        throw EpollException("epoll_ctl_del error");
    -- fdNumber;
}

int Epoll::wait(int timeout)
{
    events.resize(fdNumber);
    while (true)
    {
        nReady = epoll_wait(m_epollfd, &*events.begin(), fdNumber, timeout);
        if (nReady == 0)
            throw EpollException("epoll_wait timeout");
        else if (nReady == -1)
        {
            if (errno == EINTR)
                continue;
            else  throw EpollException("epoll_wait error");
        }
        else
            return nReady;
    }
    return -1;
}

int Epoll::getEventOccurfd(int eventIndex) const
{
    if (eventIndex > nReady)
        throw EpollException("parameter(s) error");
    return events[eventIndex].data.fd;
}

uint32_t Epoll::getEvents(int eventIndex) const
{
    if (eventIndex > nReady)
        throw EpollException("parameter(s) error");
    return events[eventIndex].events;
}

bool setUnBlock(int fd, bool unBlock)
{
    int flags = fcntl(fd,F_GETFL);
    if (flags == -1)
        return false;

    if (unBlock)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd,F_SETFL,flags) == -1)
        return false;
    return true;
}

#endif