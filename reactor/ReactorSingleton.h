#ifndef SINGLETON_H
#define SINGLETON_H
#include "Reactor.h"
#include <cstring>
template <class T>
class Singleton
{
public:
    static inline T* instance();
    void release();
protected:
    Singleton(void){}
    ~Singleton(void){}
    static T* _instance;
};

template <class T>
inline T* Singleton<T>::instance()
{
    if(!_instance)
        _instance = new T;
    return _instance;
}

template <class T>
void Singleton<T>::release()
{
    if (!_instance)
        return;
    delete _instance;
    _instance = 0;
}

#define DECLARE_SINGLETON_MEMBER(_Ty)   \
    template <> _Ty* Singleton<_Ty>::_instance = NULL;

class reactor::Reactor;

class Global : public Singleton<Global>
{
public:
    Global(void);
    ~Global(void);

    reactor::Reactor* g_reactor_ptr;
};

DECLARE_SINGLETON_MEMBER(Global);

Global::Global(void)
{
    g_reactor_ptr = new reactor::Reactor();
}

Global::~Global(void)
{
    delete g_reactor_ptr;
    g_reactor_ptr = NULL;
}


#define sGlobal Global::instance()

extern bool IsValidHandle(reactor::handle_t handle)
{
    return handle >= 0;
}

extern void ReportSocketError(const char * msg)
{
    fprintf(stderr, "%s error: %s\n", msg, strerror(errno));
}


#endif