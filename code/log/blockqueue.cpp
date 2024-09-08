#include "blockqueue.h"

template <typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacity(maxsize)
{
    assert(maxsize > 0);
    isClose_ = false;
}

template <typename T>
BlockQueue<T>::~BlockQueue()
{
    close();
}

template <typename T>
void BlockQueue<T>::close()
{
    clear();
    isClose_ = true;
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

template <typename T>
void BlockQueue<T>::clear()
{
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template <typename T>
bool BlockQueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template <typename T>
bool BlockQueue<T>::full()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template <typename T>
void BlockQueue<T>::push_back(const T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.size() >= capacity_)
    {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template <typename T>
void BlockQueue<T>::push_front(const T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.size() >= capacity_)
    {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template <typename T>
bool BlockQueue<T>::pop(T &item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.empty())
    {
        condConsumer_.wait(locker);
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template <typename T>
bool BlockQueue<T>::pop(T &item, int timeout)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.empty())
    {
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
        if (isClose_)
        {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template <typename T>
T BlockQueue<T>::front()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template <typename T>
T BlockQueue<T>::back()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template <typename T>
size_t BlockQueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template <typename T>
size_t BlockQueue<T>::size()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template <typename T>
void BlockQueue<T>::flush()
{
    condConsumer_.notify_one();
}
