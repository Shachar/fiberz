#pragma once

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>

#include <stdint.h>

namespace Fiberz {

template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    
    char value[N];
};

template<StringLiteral Name, typename Type, Type EmptyValue = Type{},
        std::ios::fmtflags Format = std::ios::dec, size_t Width = 0, char Fill = '0'>

class Typed {
    Type _value;

public:
    using UnderlyingType = Type;
    static constexpr Type Empty = EmptyValue;

    explicit constexpr Typed(Type value = EmptyValue) : _value(value) {}

    constexpr Typed(const Typed &that) = default;
    constexpr Typed(Typed &&that) = default;
    Typed &operator=(const Typed &that) = default;
    Typed &operator=(Typed &&that) = default;

    constexpr Type get() const {
        return _value;
    }

    constexpr explicit operator bool() const {
        return get()!=EmptyValue;
    }

    constexpr explicit operator Type() const {
        return get();
    }

    constexpr bool operator==(Typed that) const {
        return this->get() == that.get();
    }
    
    constexpr bool operator!=(Typed that) const {
        return this->get() != that.get();
    }
    
    friend std::ostream &operator<<(std::ostream &out, Typed typed) {
        auto savedFormat = out.flags();
        out.flags(Format);
        char savedFill = out.fill();
        out<<Name.value<<"<"<<std::setw(Width)<<std::setfill(Fill)<<typed.get()<<">";
        out.fill(savedFill);
        out.flags(savedFormat);

        return out;
    }
};

} // namespace Fiberz

namespace std {

// Allow using Typed in a hash table
template<Fiberz::StringLiteral Name, typename Type, Type EmptyValue,
        std::ios::fmtflags Format, size_t Width, char Fill>
class hash< Fiberz::Typed<Name, Type, EmptyValue, Format, Width, Fill> > {
public:
    std::size_t operator()(
                Fiberz::Typed<Name, Type, EmptyValue, Format, Width, Fill> val
            ) const
    {
        return std::hash<Type>{}( val.get() );
    }
};

} // namespace std
