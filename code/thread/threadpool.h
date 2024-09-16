#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include<assert.h>

class ThreadPool
{
private:
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool;
    
public:
    ThreadPool() = default;

    ~ThreadPool();
};



#endif