#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include "deque"
#include <condition_variable>
#include <mutex>
#include <sys/time.h>
#include <assert.h>

template <typename T>
class BlockQueue
{
private:
    std::deque<T> deq_;
    std::mutex mtx_;
    bool isClose_;
    size_t capacity_;
    std::condition_variable condConsumer_;
    std::condition_variable condProducer_ public : explicit BlockQueue(size_t max_size = 1000);

public:
    explicit BlockQueue(size_t maxsize);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T &item);
    void push_front(const T &item);
    bool pop(T &item);
    bool pop(T &item, int timeout);
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();

    void flush();
    void close();
};

#endif