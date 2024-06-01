/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef TIME_WHEEL_TIMER_H
#define TIME_WHEEL_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include <vector>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

// 定时器节点
struct WheelTimerNode {
    int id;
    int time_slot;

    TimeStamp expires; // 过期时间点
    TimeoutCallBack cb; // 回调函数

    // TimerNode* prev; // 下一个节点
    // TimerNode* next; // 上一个节点

    std::shared_ptr<WheelTimerNode> prev = nullptr; // 下一个节点
    std::shared_ptr<WheelTimerNode> next = nullptr; // 上一个节点

    WheelTimerNode(int _id, TimeStamp _expires, TimeoutCallBack _cb)
        : id(_id), expires(_expires), cb(_cb), next(nullptr) {}
};

class WheelTimer {
public:
    typedef std::shared_ptr<WheelTimerNode> timer_ptr;

    WheelTimer();

    ~WheelTimer();
    
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

    static const int slot_num = 60;  // 时间轮上槽的数量

    static const int SI = 1;  // 每 SI 秒,转动一次

    // 时间轮
    std::vector<timer_ptr> slots;

    size_t cur_slot = 0;

    // 方便根据 id 查询结点
    std::unordered_map<int, timer_ptr> ref_;
};

#endif // TIME_WHEEL_TIMER_H
