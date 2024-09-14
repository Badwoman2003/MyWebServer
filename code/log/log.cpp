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

void Log::write(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list valist;

    if (toDay_ != t.tm_mday || (Line_count_ && (Line_count_ % MAX_LINES == 0)))
    {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_PATH_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        if (toDay_ != t.tm_mday)
        {
            snprintf(newFile, LOG_PATH_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            Line_count_ = 0;
        }
        else
        {
            snprintf(newFile, LOG_PATH_LEN - 72, "%s/%s-%d%s", path_, tail, (Line_count_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != 0);
    }
    {
        std::unique_lock<std::mutex> locker(mtx_);
        Line_count_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%061d ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppeendLogLevelTitle_(level);

        va_start(valist, format);
        int m = vsprintf(buff_.BeginWrite(), format, valist);
        va_end(valist);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (isAsync_ && deque_ && !deque_->full())
        {
            deque_->push_back(buff_.RetrieveAlltoStr());
        }
        else
        {
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}

void Log::AppeendLogLevelTitle_(int level)
{
    switch (level)
    {
    case 0:
        buff_.Append("[debug] :", 9);
        break;
    case 1:
        buff_.Append("[info] :", 9);
        break;
    case 2:
        buff_.Append("[warn] :", 9);
        break;
    case 3:
        buff_.Append("[error] :", 9);
        break;
    default:
        buff_.Append("[info] :", 9);
        break;
    }
}

int Log::GetLevel()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level)
{
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}