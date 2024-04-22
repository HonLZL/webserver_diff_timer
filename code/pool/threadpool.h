/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    // 不能通过隐式类型转换来调用该构造函数
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            for(size_t i = 0; i < threadCount; i++) {
                // lambda 表达式, 无参数 () 可以省略
                std::thread([pool = pool_] {
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);   // 进入阻塞状态
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();   // 调用 cond.notify_all() 通知所有等待线程池关闭的线程
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();   // 调用 cond.notify_one() 通知一个等待的工作线程有新任务可执行
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;  // 用于线程间同步，工作线程在此等待新任务或关闭信号, 对应 pool->cond.wait, 
        bool isClosed;   // 标志线程池是否已关闭
        std::queue<std::function<void()>> tasks;   // 队列, 存储待执行的任务（类型为 std::function<void()> 的容器
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H