#include "lsttimer.h"

void LstTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    if (ref_.count(id) == 0) {
        timer_ptr timer = std::make_shared<LstTimerNode>(id, Clock::now() + MS(timeout), cb);
        ref_[id] = timer;
        if (!head) {
            // TimerNode* timer = new TimerNode(id, Clock::now() + MS(timeout), cb);
            head = tail = timer;
            return;
        }
        if (timer->expires < head->expires) {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        // timer 需要插入链表中
        insert_timer(timer);
    } else {
        ref_[id]->cb = cb;
        adjust(id, timeout);
    }
}

void LstTimer::doWork(int id) {
    if (head || ref_.count(id)) {
        return;
    }
    ref_[id]->cb();
    del_timer(ref_[id]);
}

void LstTimer::adjust(int id, int newExpires) {
    if (!head) {
        return;
    }
    assert(head && ref_.count(id) > 0);
    ref_[id]->expires = Clock::now() + MS(newExpires);

    if (ref_[id]->prev && ref_[id]->next) {  // 中间
        ref_[id]->prev->next = ref_[id]->next;
        ref_[id]->next->prev = ref_[id]->prev;
        ref_[id]->prev.reset();
        ref_[id]->next.reset();
        insert_timer(ref_[id]);
    } else if (ref_[id]->prev) {  //  is tail
        ref_[id]->prev->next.reset();
        tail = ref_[id]->prev;
        ref_[id]->prev.reset();
        insert_timer(ref_[id]);
    } else if (ref_[id]->next) {  // is head
        head = head->next;
        head->prev.reset();
        ref_[id]->next.reset();
        insert_timer(ref_[id]);
    } else {
        // 只有这一个 timer, 所以不用变
    }
}

void LstTimer::clear() {
    head.reset();
    ref_.clear();
}

void LstTimer::tick() {
    if (!head) {
        return;
    }
    // printf( "timer tick\n" );
    // LOG_INFO("%s", "timer tick");
    // Log::get_instance()->flush();
    timer_ptr tmp = head;
    while (tmp) {
        if (std::chrono::duration_cast<MS>(tmp->expires - Clock::now()).count() > 0) {
            break;
        }
        tmp->cb();
        tmp = tmp->next;
        pop();
    }
}

void LstTimer::pop() {
    if(!head) {
        return;
    }
    del_timer(head);
}

int LstTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if (head) {
        res = std::chrono::duration_cast<MS>(head->expires - Clock::now()).count();
        if (res < 0)
            res = 0;
    }
    return res;
}

void LstTimer::insert_timer(timer_ptr timer) {
    if (!head) {
        head = timer;
        tail = timer;
        head->prev.reset();
        head->next.reset();
        tail->prev.reset();
        tail->next.reset();
        return;
    }
    timer_ptr pre = head;
    timer_ptr tmp = pre->next;

    while (tmp) {
        if (timer->expires < tmp->expires) {
            pre->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = pre;
            break;
        }
        pre = tmp;
        tmp = tmp->next;
    }
    if (!tmp) {  // 链表内所有定时器的超时时间均小于 timer
        pre->next = timer;
        timer->prev = pre;
        timer->next.reset();
        tail = timer;
    }
}

void LstTimer::del_timer(timer_ptr timer) {
    if (!timer) {
        return;
    }
    if ((timer == head) && (timer == tail)) {
        head.reset();
        tail.reset();
        return;
    }
    if (timer == head) {
        head = head->next;
        head->prev.reset();
        return;
    }
    if (timer == tail) {
        tail = tail->prev;
        tail->next.reset();
        return;
    }
    int id_ = timer->id;
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    ref_.erase(id_);
}
