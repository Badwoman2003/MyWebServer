#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>
class Buffer
{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;
    size_t WritableBytes() const; // count of writable Bytes
    size_t ReadableBytes() const; // count of readable Bytes
    size_t PrependBytes() const;  // count of prepend Bytes

    const char *Peek() const;        // return readPos_ ptr
    void EnsureWritable(size_t len); // make sure write space enough
    void HasWritten(size_t len);     // change writePos_

    void Retrieve(size_t len);
    void RetrieveUtil(const char *end);
    void RetrieveAll();
    std::string RetrieveAlltoStr();

    const char *BeginWriteConst();
    char *BeginWrite();

    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buff);

    ssize_t ReadFd(int fd, int *Errno);
    ssize_t WriteFd(int fd, int *Errno);

private:
    char *BeginPtr_();
    const char *BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;  // read index
    std::atomic<std::size_t> writePos_; // write index
};
#endif