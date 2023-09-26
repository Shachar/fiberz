#pragma once

#include <fiberz/typed.h>

namespace Fiberz {

class Reactor;
namespace Internal { class Fiber; }

using FiberId = Typed<"FiberId", uint16_t, 0xffff, std::ios::hex, 4>;

class FiberHandle {
    uint16_t param1;
    uint16_t param2;

private:
    friend Reactor;
    friend Internal::Fiber;

    explicit FiberHandle(uint16_t p1, uint16_t p2) : param1(p1), param2(p2) {}
public:
    FiberHandle() : param1(0), param2(0) {}

    FiberId getId() const;

    bool isSet() const {
        return param1!=0 || param2!=0;
    }

    explicit operator bool() const { return isSet(); }

    friend std::ostream &operator<<(std::ostream &out, const FiberHandle &handle) {
        return out<<handle.getId();
    }
};

} // namespace Fiberz
