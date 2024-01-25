#pragma once

#include <assert.h>

class FD {
    int fd_;

public:
    FD() : fd_(-1) {}
    explicit FD(int fd);

    FD(const FD &that) = delete;
    FD &operator=(const FD &that) = delete;

    FD(FD &&that) : fd_(that.fd_) { that.fd_=-1; }
    FD &operator=(FD that);

    ~FD() { close(); }

    void close() noexcept;

    int get() const {
        assert(fd_ != -1);
        return fd_;
    }
};
