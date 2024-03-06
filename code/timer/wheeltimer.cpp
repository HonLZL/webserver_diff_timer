#include "wheeltimer.h"


WheelTimer::WheelTimer() { 
    slots.assign(slot_num, nullptr);
}

WheelTimer::~WheelTimer() { 
     clear(); 
}

void WheelTimer::adjust(int id, int newExpires) {
    // printf("ref_ size = %d\n", (int)ref_.size());
    if (ref_.count(id) > 0) {
        timer_ptr tmp = ref_[id];
        del_timer(ref_[id]);
        tmp->expires = Clock::now() + MS(newExpires);
        insert_timer(tmp);
    }
}

void WheelTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    if (timeout < 0) {
        return;
    }
    if (ref_.count(id) == 0) {
        timer_ptr timer = std::make_shared<WheelTimerNode>(id, Clock::now() + MS(timeout), cb);
        ref_[id] = timer;
        insert_timer(timer);
    } else {
        ref_[id]->cb = cb;
        adjust(id, timeout);
    }
}

void WheelTimer::doWork(int id) {
    if(ref_.count(id) == 0) {
        return;
    }
    // 执行定时器回调函数
    ref_[id]->cb();
    // 从时间轮中删除定时器节点
    del_timer(ref_[id]);
    // 从引用表中删除定时器
    ref_.erase(id);
}

void WheelTimer::clear() {
    // 清空时间轮和引用表
    for (auto& slot : slots) {
        slot.reset();
    }
    cur_slot = 0;
    ref_.clear();
}

void WheelTimer::tick() {
    // auto& slot = slots[cur_slot];
    timer_ptr tmp = slots[cur_slot];
    while (tmp) {
        timer_ptr next = tmp->next;
        if(std::chrono::duration_cast<MS>(tmp->expires - Clock::now()).count() <= 0) { 
            tmp->cb();
            del_timer(tmp); 
        }
        tmp = next;
    }
    // 时间轮指针向前移动一个槽位
    cur_slot = (cur_slot + 1) % slot_num;
}


int WheelTimer::GetNextTick() {
    tick();
    int res = -1;
    timer_ptr& slot = slots[cur_slot];
    if (slot) {
        res = std::chrono::duration_cast<MS>(slot->expires - Clock::now()).count();
        if (res < 0) {
            res = 0;
        }
    }
    return res;
}

void WheelTimer::insert_timer(timer_ptr timer) {
    TimeStamp expires = timer->expires;
    MS timeout = std::chrono::duration_cast<MS>(expires - Clock::now());
    long long ticks = timeout.count();
    if (ticks < 0) {
        ticks = 0;
    }
    // SI * 1000, SI是毫秒
    size_t idx = (cur_slot + ticks / (SI * 1000)) % slot_num;
    timer->time_slot = idx;
    timer->next = slots[idx];
    if (slots[idx]) {
        slots[idx]->prev = timer;
    }
    slots[idx] = timer;
}

void WheelTimer::del_timer(timer_ptr timer) {
    int id = timer->id;

    if (ref_.count(id) ==0 )
        return;

    size_t idx = timer->time_slot;
    if (timer == slots[idx]) {
        slots[idx] = timer->next;
        if (slots[idx]) {
            slots[idx]->prev.reset();
        }
        return;
    }
    timer->prev->next = timer->next;
    if (timer->next) {
        timer->next->prev = timer->prev;
    }
    ref_.erase(id);
}
