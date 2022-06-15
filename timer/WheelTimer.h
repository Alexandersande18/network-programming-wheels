#ifndef WHEELTIMER_H
#define WHEELTIMER_H

#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <thread>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <time.h>
#include <error.h>
#include <string.h>

struct param
{
    /*
    ** Posix定时器开始值和间隔
    */
    struct itimerspec its;
    int ifd;
};

/* 定时器任务 */
struct TimerTask
{
    bool repeat;
    int timeval;
    int id;
    std::function<void()> task;
};

class TimerWheel
{
public:
    TimerWheel(int tick = 5,int slot_num = (1<<3),int layer_num = (1<<4));
    ~TimerWheel();
    void Start();
    void Stop();
    int AddTask(struct TimerTask &task);

private:
    int init();
    void shedule(int);
    void tick();
private:
    int m_tick;     
    int m_slot_num; 
    int m_layer_num;
    int m_cur_slot_num;
    int m_cur_layer_num;
    int m_status;       //状态：0 = stop , 1 = running
    std::vector<std::vector<std::list<struct TimerTask>>> wheel;

    std::thread *m_run_shedule;     // 运行调度
    int m_timer_fd;
};


TimerWheel::TimerWheel(int tick, int slot_num, int layer_num):
    m_tick(tick),m_slot_num(slot_num),m_layer_num(layer_num)
{
    m_cur_slot_num = 0;
    m_cur_layer_num = 0;
    m_status = 0;
    m_run_shedule = nullptr;
    wheel.resize(m_slot_num);
    for(int i = 0;i < m_slot_num;++i){
        wheel[i].resize(m_layer_num);
    }
    printf("tick=%d ms, %d layers, %d slots, max_interval=%ld\n", m_tick, m_layer_num, m_slot_num, m_tick*m_slot_num*m_layer_num);
}

TimerWheel::~TimerWheel()
{
    if(m_run_shedule != nullptr && m_run_shedule->joinable()){
        m_run_shedule->join();
    }
}

void TimerWheel::Start()
{
    if(m_status == 1){
        perror("TimerWheel start running");
        return;
    }
    int eplfd = init();
    m_run_shedule = new std::thread(std::bind(&TimerWheel::shedule, this, eplfd));
    if(m_run_shedule == nullptr){
        perror("apply for a new thread memory");
        return;
    }
    m_status = 1;
}

void TimerWheel::Stop()
{

}

int TimerWheel::AddTask(struct TimerTask& task){
    int interval = task.timeval;
    if(interval > m_tick * m_slot_num * m_layer_num){
        printf("error:interval beyond MAX_TIMER_LIMIT");
    }

    // 设置任务所处的时间轮层数
    int layer = ((interval / m_tick + m_cur_slot_num) / m_slot_num + m_cur_layer_num) % m_layer_num;
    // 设置任务所处的时间槽
    int slot = (interval / m_tick + m_cur_slot_num) % m_slot_num;
    if(slot >= m_slot_num){
        slot = slot % m_slot_num;
        layer = (layer+1) % layer;
    }
    printf("add task in wheel[%d][%d]\n",layer,slot);
    wheel[slot][layer].push_back(std::move(task));
}

int TimerWheel::init()
{
    m_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if(m_timer_fd < 0){
        perror("create timer_fd error\n");
        return -1;
    } 
    struct itimerspec new_its;
    new_its.it_value.tv_sec = 2;
    new_its.it_value.tv_nsec = 0;
    new_its.it_interval.tv_sec = m_tick / 1000;
    new_its.it_interval.tv_nsec = (m_tick % 1000) * 1000 * 1000;
    // printf("timerset:%ld %ld\n", new_its.it_interval.tv_nsec, new_its.it_interval.tv_sec);
    int ret = timerfd_settime(m_timer_fd, 1, &new_its, NULL);
    if(ret < 0){
        perror("timerfd_settime error\n");
        close(m_timer_fd);
        return -1;
    }
    
    int eplfd = epoll_create(1);
    struct epoll_event ev;
    struct param pm;
    pm.its = new_its;
    pm.ifd = m_timer_fd;
    ev.events = EPOLLIN;
    ev.data.ptr = &pm;
    ev.data.fd = m_timer_fd;

    if(epoll_ctl(eplfd,EPOLL_CTL_ADD,m_timer_fd,&ev) != 0){
        perror("epoll_ctl() error");
        return -1;
    }
    return eplfd;
}

void TimerWheel::shedule(int eplfd)
{
    struct epoll_event events[1];
    while(1){
        int n = epoll_wait(eplfd,events, 1, -1);
        if(n == -1){
            perror("epoll_wait() error");
            break;
        }
        for (int i = 0; i < n; ++i)
        {
            if(m_timer_fd != events[i].data.fd){
                continue;
            }
            uint64_t exp;
            ssize_t size = read(events[i].data.fd, &exp, sizeof(uint64_t));
            if (size != sizeof(uint64_t)) {
                char* msg = strerror(errno);
                printf("read error:%d , Msg:%s\n",size,msg);
            }
            tick();
        }
    }
}

void TimerWheel::tick()
{
    //do timerwheel work();
    static clock_t oldtime;
    clock_t newtime = clock();
    // printf("shedule is working newtime%ld %ld %f \n",newtime,oldtime,(float)(newtime-oldtime)/CLOCKS_PER_SEC*1000);
    oldtime = newtime;
    
    printf("[tick]layer:%d slot:%d  \n",m_cur_layer_num,m_cur_slot_num);
    if(wheel[m_cur_slot_num][m_cur_layer_num].empty()){
        if (++m_cur_slot_num == m_slot_num)
        {
            m_cur_slot_num = 0;
            if (++m_cur_layer_num == m_layer_num)
            {
                m_cur_layer_num = 0;
            }
        }
        return;
    }
    std::list<struct TimerTask> *tasks = &(wheel[m_cur_slot_num][m_cur_layer_num]);
    int size = tasks->size();
    for(int i = 0;i<size;++i){
        tasks->front().task();
        if(tasks->front().repeat){
            AddTask(tasks->front());
        }
        tasks->pop_front();
    }

    if(++m_cur_slot_num == m_slot_num){
        m_cur_slot_num = 0;
        if(++m_cur_layer_num == m_layer_num){
            m_cur_layer_num = 0;
        }
    }
}

#endif