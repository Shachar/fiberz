#include "fiberz/utility/fd.h"

#include <system_error>
#include <utility>      // swap

#include <unistd.h>

FD::FD(int fd) : fd_(fd) {
    if( fd==-1 )
        throw std::system_error( errno, std::system_category() );
}

FD &FD::operator=(FD that) {
    std::swap(fd_, that.fd_);

    return *this;
}

void FD::close() noexcept {
    if( fd_!=-1 ) {
        // No error checking as there's nothing really to do with an error here
        ::close(fd_);

        fd_ = -1;
    }
}
