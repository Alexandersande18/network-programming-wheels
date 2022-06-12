#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include <stdint.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include <vector>
#include <map>

#include "HeapTimer.h"

#define INITSIZE 100


namespace reactor
{
    
typedef unsigned int event_t;
enum
{
    kReadEvent    = 0x01,
    kWriteEvent   = 0x02,
    kErrorEvent   = 0x04,
    kEventMask    = 0xff
};

typedef int handle_t;

class EventHandler
{
public:

    virtual handle_t GetHandle() const = 0;

    virtual void HandleRead() {}

    virtual void HandleWrite() {}

    virtual void HandleError() {}

protected:

    EventHandler() {}

    virtual ~EventHandler() {}
};


class EventDemultiplexer
{
public:

    virtual ~EventDemultiplexer() {}

    virtual int WaitEvents(std::map<handle_t, EventHandler *> * handlers,
                            int timeout = 0, time_heap* event_timer = NULL) = 0;
                            
    virtual int RequestEvent(handle_t handle, event_t evt) = 0;

    virtual int UnrequestEvent(handle_t handle) = 0;
};

///////////////////////////////////////////////////////////////////////////////

class EpollDemultiplexer : public EventDemultiplexer
{
public:
    EpollDemultiplexer();

    ~EpollDemultiplexer();

    virtual int WaitEvents(std::map<handle_t, EventHandler *> * handlers,
                            int timeout = 0, time_heap* event_timer = NULL);

    virtual int RequestEvent(handle_t handle, event_t evt);

    virtual int UnrequestEvent(handle_t handle);

private:

    int  m_epoll_fd;
    int  m_fd_num;
};


class ReactorImplementation;

class Reactor
{
public:

    Reactor();

    ~Reactor();

    int RegisterHandler(EventHandler * handler, event_t evt);

    int RemoveHandler(EventHandler * handler);

    void HandleEvents();

    int RegisterTimerTask(heap_timer* timerevent);
private:

    Reactor(const Reactor &);
    Reactor & operator=(const Reactor &);

private:

    ReactorImplementation * m_reactor_impl;
};


class ReactorImplementation
    {
public:

    ReactorImplementation();

    ~ReactorImplementation();

    int RegisterHandler(EventHandler * handler, event_t evt);

    int RemoveHandler(EventHandler * handler);

    void HandleEvents();

    int RegisterTimerTask(heap_timer* timerevent);
private:

    EventDemultiplexer *                m_demultiplexer;
    std::map<handle_t, EventHandler *>  m_handlers;
    time_heap* m_eventtimer;
};

///////////////////////////////////////////////////////////////////////////////
EpollDemultiplexer::EpollDemultiplexer()
{
    m_epoll_fd = ::epoll_create(FD_SETSIZE);
    assert(m_epoll_fd != -1);
    m_fd_num = 0;
}

EpollDemultiplexer::~EpollDemultiplexer()
{
    ::close(m_epoll_fd);
}

int EpollDemultiplexer::WaitEvents(
        std::map<handle_t, EventHandler *> * handlers,
        int timeout, time_heap* event_timer)
        
{
    std::vector<epoll_event> ep_evts(m_fd_num);
    int num = epoll_wait(m_epoll_fd, &ep_evts[0], ep_evts.size(), timeout);
    if (num > 0)
    {
        for (int idx = 0; idx < num; ++idx)
        {
            handle_t handle = ep_evts[idx].data.fd;
            assert(handlers->find(handle) != handlers->end());
            if ((ep_evts[idx].events & EPOLLERR) ||
                    (ep_evts[idx].events & EPOLLHUP))
            {
                (*handlers)[handle]->HandleError();
            }
            else
            {
                if (ep_evts[idx].events & EPOLLIN)
                {
                    (*handlers)[handle]->HandleRead();
                }
                if (ep_evts[idx].events & EPOLLOUT)
                {
                    (*handlers)[handle]->HandleWrite();
                }
            }
        }
    }
    if (event_timer != NULL)
    {
        event_timer->tick();
    }

    return num;
}

int EpollDemultiplexer::RequestEvent(handle_t handle, event_t evt)
{
    epoll_event ep_evt;
    ep_evt.data.fd = handle;
    ep_evt.events = 0;

    if (evt & kReadEvent)
    {
        ep_evt.events |= EPOLLIN;
    }
    if (evt & kWriteEvent)
    {
        ep_evt.events |= EPOLLOUT;
    }
    ep_evt.events |= EPOLLONESHOT;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, handle, &ep_evt) != 0)
    {
        if (errno == ENOENT)
        {
            if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, handle, &ep_evt) != 0)
            {
                return -errno;
            }
            ++m_fd_num;
        }
    }
    return 0;
}

int EpollDemultiplexer::UnrequestEvent(handle_t handle)
{
    epoll_event ep_evt;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, handle, &ep_evt) != 0)
    {
        return -errno;
    }
    --m_fd_num;
    return 0;
}


Reactor::Reactor()
{
    m_reactor_impl = new ReactorImplementation();
}

Reactor::~Reactor()
{
    delete m_reactor_impl;
}

int Reactor::RegisterHandler(EventHandler * handler, event_t evt)
{
    return m_reactor_impl->RegisterHandler(handler, evt);
}

int Reactor::RemoveHandler(EventHandler * handler)
{
    return m_reactor_impl->RemoveHandler(handler);
}

void Reactor::HandleEvents()
{
    m_reactor_impl->HandleEvents();
}

int Reactor::RegisterTimerTask(heap_timer* timerevent)
{
    return m_reactor_impl->RegisterTimerTask(timerevent);
}

///////////////////////////////////////////////////////////////////////////////

ReactorImplementation::ReactorImplementation()
{
    m_demultiplexer = new EpollDemultiplexer();
    m_eventtimer = new time_heap(INITSIZE);
}

ReactorImplementation::~ReactorImplementation()
{
    delete m_demultiplexer;
}

int ReactorImplementation::RegisterHandler(EventHandler * handler, event_t evt)
{
    handle_t handle = handler->GetHandle();
    std::map<handle_t, EventHandler *>::iterator it = m_handlers.find(handle);
    if (it == m_handlers.end())
    {
        m_handlers[handle] = handler;
    }
    return m_demultiplexer->RequestEvent(handle, evt);
}

int ReactorImplementation::RemoveHandler(EventHandler * handler)
{
    handle_t handle = handler->GetHandle();
    m_handlers.erase(handle);
    return m_demultiplexer->UnrequestEvent(handle);
}

//parm timeout is useless.
void ReactorImplementation::HandleEvents()
{
    int timeout = 0;
    if (m_eventtimer->top() == NULL)
    {
        timeout = 0;
    }
    else
    {
        timeout = ((m_eventtimer->top())->expire - time(NULL)) * 1000;
    }
    m_demultiplexer->WaitEvents(&m_handlers, timeout, m_eventtimer);
}

int ReactorImplementation::RegisterTimerTask(heap_timer* timerevent)
{
    if (timerevent == NULL)
        return -1;
    m_eventtimer->add_timer(timerevent);
    return 0;
}

}


#endif