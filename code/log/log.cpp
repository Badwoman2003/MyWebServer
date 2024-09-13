#include "log.h"

Log::Log() : fp_(nullptr), deque_(nullptr), writTread_(nullptr)
{
    Line_count_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

Log::~Log()
{
    while (!deque_->empty())
    {
        deque_->flush();
    }
    deque_->close();
    writTread_->join();
    if (fp_)
    {
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

void Log::flush()
{
    if (isAsync_)
    {
        deque_->flush();
    }
    fflush(fp_);
}

Log *Log::Instance()
{
    static Log log;
    return &log;
}

void Log::FlushLogThread()
{
    Log::Instance()->AsyncWrite_();
}

void Log::AsyncWrite_()
{
    std::string str = "";
    while (deque_->pop(str))
    {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::init(int level, const char *path, const char *suffix, int maxQueueCapacity)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if (maxQueueCapacity)
    {
        isAsync_ = true;
        if (!deque_)
        {
            std::unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
            deque_ = std::move(newQueue);

            std::unique_ptr<std::thread> newTread(new std::thread(FlushLogThread));
            writTread_ = std::move(newTread);
        }
    }
    else
    {
        isAsync_ = false;
    }
    Line_count_ = 0;
    std::time_t timer = std::time(nullptr);
    struct tm *systime = localtime(&timer);
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s%04d_%02d_%02d%s", path_, systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday, suffix_);
    toDay_ = systime->tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_)
        {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            mkdir(fileName, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}