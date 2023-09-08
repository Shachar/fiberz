#pragma once

namespace Fiberz {

    class NoFreeFibers : public std::exception  {
    public:
        const char *what() const noexcept override {
            return "Out of fibers";
        }
    };

} // namespace
