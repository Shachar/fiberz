#pragma once

#include <concepts>
#include <memory>
#include <tuple>

namespace Fiberz::Internal {

class ParametersBase {
public:
    virtual ~ParametersBase() = default;

    virtual void invoke() = 0;
};

template<typename Callable, typename... Arguments>
    requires std::invocable<Callable, Arguments...>
class Parameters : public Internal::ParametersBase {
    std::tuple<Arguments...>            _arguments;
    Callable                            _callable;

public:
    Parameters(Callable callable, Arguments&&... arguments) :
        _arguments( std::forward<Arguments...>(arguments)... ),
        _callable( std::forward<Callable>(callable) )
    {}

    void invoke() override {
        std::apply<Callable, decltype(_arguments)>(
                std::forward<Callable>(_callable),
                std::move( _arguments )
            );
    }
};

FiberHandle createFiber( std::unique_ptr<ParametersBase> parameters );

} // namespace Fiberz::Internal
