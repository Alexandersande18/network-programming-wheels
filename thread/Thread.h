#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <unistd.h>
#include <iostream>
class Thread
{
public:
    Thread();
    virtual ~Thread();
    int run(void *pContext = 0);
    int wait();

private:
    static void* startFunctionOfThread(void *context);

protected:
    virtual int ThreadFunction() = 0;
    void *context;
    pthread_t threadID;
    unsigned long mID;
};


class Cond {
public:

    Cond();
    ~Cond();
    void wait(pthread_mutex_t* mutex);
    void signal();
    void broadcast();

private:
    pthread_cond_t m_cond_var;
};


class Mutex
{
public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
    pthread_mutex_t* get_mutex_ptr();

private:
    pthread_mutex_t m_lock;
    volatile bool is_locked;
};


Thread::Thread()
{
    context = 0;
}

Thread::~Thread()
{
    
}

int Thread::run(void *pContext)
{
    context = pContext;

    int r = pthread_create(&threadID, 0, startFunctionOfThread, this);
    if(r != 0)
    {
        printf("In Thread::run(), pthread_create error.\n");
        return -1;
    }
    return 0;
}

int Thread::wait()
{
    int r = pthread_join(threadID, 0);
    if(r != 0)
    {
        printf("In Thread::wait(), pthread_join error.\n");
        return -1;
    }
    return 0;
}

void* Thread::startFunctionOfThread(void *pThis)
{
    Thread *threadThis = (Thread *)pThis;
    int s = threadThis->ThreadFunction();
    return (void *)s;
}

Cond::Cond() {
    pthread_cond_init(&m_cond_var, NULL);
}
Cond::~Cond() {
    pthread_cond_destroy(&m_cond_var);
}
void Cond::wait(pthread_mutex_t* mutex) {
    pthread_cond_wait(&m_cond_var, mutex);
}
void Cond::signal() {
    pthread_cond_signal(&m_cond_var);
}
void Cond::broadcast() {
    pthread_cond_broadcast(&m_cond_var);
}


Mutex::Mutex() {
    pthread_mutex_init(&m_lock, NULL);
    is_locked = false;
}

Mutex::~Mutex() {
    while(is_locked);
    unlock(); // Unlock Mutex after shared resource is safe
    pthread_mutex_destroy(&m_lock);
}

void Mutex::lock() {
    pthread_mutex_lock(&m_lock);
    is_locked = true;
}

void Mutex::unlock() {
    is_locked = false; // do it BEFORE unlocking to avoid race condition
    pthread_mutex_unlock(&m_lock);
}

pthread_mutex_t* Mutex::get_mutex_ptr(){
    return &m_lock;
}



#endif