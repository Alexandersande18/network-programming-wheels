#include "WheelTimer.h"
#include "../thread/Thread.h"
using namespace std;
static Cond cond;
static Mutex mutex;
int main(int argc,char **argv)
{
    TimerWheel tw(100);       //tick=100ms
    struct TimerTask task1{repeat: true, timeval: 500, task: std::bind([]{printf("trigger! my interval is 0.5s\n"); })}; //定时每0.5s执行一次
    struct TimerTask task2{repeat: true, timeval: 800, task: std::bind([]{printf("trigger! my interval is 0.8s\n"); })}; //定时每0.8s执行一次
    tw.AddTask(task1);
    tw.AddTask(task2);
    tw.Start();

    mutex.lock();
    cond.wait(mutex.get_mutex_ptr()); //主线程挂起
    return 0;
}