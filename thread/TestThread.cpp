#include "Thread.h"
#include <iostream>
#include <vector>
#include <stack>
using namespace std;

const int MAX_TASKS = 4;
const int MAX_ITERARIONS = 10;

static Cond cond;
static Mutex mutex;

static stack<int> resourse;

class ProducerThread : public Thread
{
    public:
    virtual int ThreadFunction() 
    {
        int i = 0;
        while( i++ < MAX_ITERARIONS)
        {
            int data = rand() % 1234;
            sleep(2);
            mutex.lock();
            resourse.push(data);
            cout << "Producing data = " << data << endl;
            mutex.unlock();
            cond.signal();
        }
    }
};

class ConsumerThread : public Thread
{
    public: 
    virtual int ThreadFunction()
    {
        int data;
        int i = 0;
        while( i++ < MAX_ITERARIONS)
        {
            mutex.lock();
            while(resourse.empty())
            {
                cout << "Producer is not ready" << endl << endl;
                cond.wait(mutex.get_mutex_ptr());
                break;
            }
            cout << "Producer is ready" << endl;
            data = resourse.top();
            resourse.pop();
            cout << "Consuming data = " << data << endl;
            
            mutex.unlock();
        }
        sleep(1);
    }
};

int main(int argc, char* argv[])
{
    vector<Thread*> vt(2);
    vt[0] = new ProducerThread();
    vt[1] = new ConsumerThread();
    for(auto v : vt){
        v->run();
    }
    for(auto v: vt){
        v->wait();
    }
    return 0;
}