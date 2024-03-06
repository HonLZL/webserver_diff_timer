/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef Lst_TIMER_H
#define Lst_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include <unordered_map>

#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;


// 定时器节点
struct LstTimerNode {
    int id;
    TimeStamp expires; // 过期时间点
    TimeoutCallBack cb; // 回调函数

    // TimerNode* prev; // 下一个节点
    // TimerNode* next; // 上一个节点

    std::shared_ptr<LstTimerNode> prev = nullptr; // 下一个节点
    std::shared_ptr<LstTimerNode> next = nullptr; // 上一个节点

    LstTimerNode(int _id, TimeStamp _expires, TimeoutCallBack _cb)
        : id(_id), expires(_expires), cb(_cb), next(nullptr) {}
};

class LstTimer {
public:
    typedef std::shared_ptr<LstTimerNode> timer_ptr;

    LstTimer() { }

    ~LstTimer() { clear(); }
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void insert_timer(timer_ptr timer);

    void del_timer(timer_ptr timer);


    // TimerNode* head = nullptr;
    // TimerNode* tail = nullptr;

    timer_ptr head = nullptr;
    timer_ptr tail = nullptr;

    std::unordered_map<int, timer_ptr> ref_;

};

#endif //Lst_TIMER_H