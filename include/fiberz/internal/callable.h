#pragma once

#include <memory>
#include <type_traits>

namespace Fiberz::Internal {

template<typename RetType, typename... Args>
class Callable {
    class ActualCallableBase {
    public:
        ActualCallableBase() = default;
        virtual ~ActualCallableBase() = default;

        virtual RetType call(Args... args) = 0;
        virtual RetType call(Args... args) const = 0;
    };

    std::unique_ptr<ActualCallableBase> actualCallable;

    template<typename C> class ActualCallable : public ActualCallableBase {
        C callable;

    public:
        explicit ActualCallable(const C &c) : callable(c) {}
        explicit ActualCallable(C &&c) : callable(std::move(c)) {}

        RetType call(Args... args) override {
            return callable(std::forward<Args>(args)...);
        }
        RetType call(Args... args) const override {
            return callable(std::forward<Args>(args)...);
        }
    };

public:
    Callable() = default;

    template<typename C>
    Callable(const C &c) : actualCallable( new ActualCallable<C>(c) ) {} 
    template<typename C>
    Callable(C &&c) : actualCallable( new ActualCallable<C>(std::move(c)) ) {} 


    RetType operator()(Args... args) {
        assert( actualCallable );
        return actualCallable->call( args... );
    }
    RetType operator()(Args... args) const {
        assert( actualCallable );
        return actualCallable->call( args... );
    }
};

} // namespace Fiberz::Internal
