#pragma once

#include "deque"
#include <condition_variable>
#include <mutex>
#include <sys/time.h>

template <typename T>
class BlockQueue
{
private:
    /* data */
public:
    explicit BlockQueue(size_t max_size = 1000);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T& item);
    void push_front(const T& item);
    bool pop(T& item);
    bool pop(T& item,int timeout);
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();

    void flush();
    void close();

};
