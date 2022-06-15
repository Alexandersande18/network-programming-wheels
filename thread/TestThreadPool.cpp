#include "ThreadPool.h"

#include <iostream>
#include <stack>

using namespace std;

const int MAX_TASKS = 4;
const int MAX_ITERARIONS = 5;

static stack<int> resource;
static Mutex mutex;
static Cond cond;

void producer(void *arg)
{
    int i = 0;
    while( i++ < MAX_ITERARIONS)
    {
        int data = rand() % 1234;
        sleep(2);
        mutex.lock();
        resource.push(data);
        cout << "Producing data = " << data << endl;
        mutex.unlock();
        cond.signal();
    }
}

void consumer(void *arg)
{
    int data;
    int i = 0;
    while( i++ < MAX_ITERARIONS)
    {
        mutex.lock();
        while(resource.empty())
        {
            cout << "Producer is not ready" << endl << endl;
            cond.wait(mutex.get_mutex_ptr());
            break;
        }
        
        cout << "Producer is ready" << endl;
        data = resource.top();
        resource.pop();
        cout << "Consuming data = " << data << endl;
        
        mutex.unlock();
    }
    sleep(1);
}

int main(int argc, char* argv[])
{
    ThreadPool tp(2);
    int ret = tp.initialize_threadpool();
    if (ret == -1) {
        cerr << "Failed to init tp" << endl;
        return 0;
    }

    Task *t1 = new Task(&producer, (void *)0);
    Task *t2 = new Task(&consumer, (void *)0);
    tp.add_task(t1);
    tp.add_task(t2);

    sleep(2);

    tp.destroy_threadpool();

    return 0;
}