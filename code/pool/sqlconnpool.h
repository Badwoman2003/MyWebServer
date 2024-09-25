#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool
{
private:
    SqlConnPool(/* args */);
    ~SqlConnPool();

    int MaxConn_;
    int UseConn_;
    int FreeConn_;

    std::queue<MYSQL *> conn_queue;
    std::mutex mtx_;
    sem_t semId_;
public:
    static SqlConnPool* Instance();
    MYSQL* GetConn();
    void FreeConn(MYSQL* Conn);
    int GetFreeConnCount();
};



#endif