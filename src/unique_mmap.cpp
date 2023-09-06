#include <fiberz/unique_mmap.h>

#include <system_error>

namespace Fiberz {

UniqueMmap::UniqueMmap(void *start_addr, size_t length, int prot, int flags) {
    void *mapping = mmap(start_addr, length, prot, flags|MAP_ANONYMOUS, -1, 0);
    if( mapping==MAP_FAILED )
        throw std::system_error( errno, std::system_category(), "Failed to allocate memory" );

    _data = std::span( reinterpret_cast<std::byte *>(mapping), length );
}

// UniqueMmap::UniqueMmap(void *start_addr, size_t length, int prot, int flags, const FD &fd, off_t offset)

UniqueMmap::~UniqueMmap() {
    if( ! _data.empty() ) {
        munmap( _data.data(), _data.size_bytes() );
    }
}

} // namespace Fiberz
