#ifndef TASK_H
#define TASK_H

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>

using namespace std;

class Task
{
public:
    Task(void (*fn_ptr)(void*), void* arg); // pass a free function pointer
    ~Task();
    void operator()();
    void run();
private:
    void (*fn_ptr_)(void*);
    void* m_arg;
};

Task::Task(void (*fn_ptr)(void*), void* arg) : fn_ptr_(fn_ptr), m_arg(arg) { }

Task::~Task() { }

void Task::operator()() {
    (*fn_ptr_)(m_arg);
    if (m_arg != NULL) {
        delete m_arg;
    }
}

void Task::run() {
    (*fn_ptr_)(m_arg);
}

#endif