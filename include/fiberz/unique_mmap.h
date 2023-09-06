#pragma once

#include <span>

#include <sys/mman.h>

namespace Fiberz {

class FD;

class UniqueMmap {
    std::span<std::byte>        _data;

public:
    explicit UniqueMmap() = default;
    explicit UniqueMmap(void *start_addr, size_t length, int prot, int flags);
    explicit UniqueMmap(void *start_addr, size_t length, int prot, int flags, const FD &fd, off_t offset);

    ~UniqueMmap();

    UniqueMmap(const UniqueMmap &that) = delete;
    UniqueMmap &operator=(const UniqueMmap &that) = delete;

    constexpr UniqueMmap(UniqueMmap &&that) = default;
    constexpr UniqueMmap &operator=(UniqueMmap &&that) = default;

    std::byte *get() {
        return _data.data();
    }

    const std::byte *get() const {
        return _data.data();
    }
};

} // namespace Fiberz
