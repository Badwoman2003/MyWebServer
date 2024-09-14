#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include "blockqueue.h"
#include "../buffer/Buffer.h"

class Log
{
private:
    Log();
    virtual ~Log();
    void AppeendLogLevelTitle_(int level);
    void AsyncWrite_(); // async write log
public:
    void init(int level, const char *path = "./log", const char *suffix = ".log", int maxQueueCapacity = 1024);
    static Log *Instance();
    static void FlushLogThread();

    void write(int level, const char *format, ...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; };

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 5000;

    const char *path_;
    const char *suffix_;

    int Max_Lines_;

    int Line_count_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;
    std::unique_ptr<BlockQueue<std::string>> deque_;
    std::unique_ptr<std::thread> writTread_;
    std::mutex mtx_;
};


