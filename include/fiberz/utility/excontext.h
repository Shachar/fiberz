#pragma once

#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

class ExContextBase {
protected:
    class TraceBase {
    protected:
        const char *_message;

        explicit TraceBase( const char *message ) : _message(message) {}
    public:
        virtual ~TraceBase() = default;
        virtual void printParams( std::ostream &out ) const = 0;
    };
    static thread_local std::vector< std::unique_ptr<TraceBase> > trace;

    int                 _active_exceptions;
    const char *        _message;

public:
    static void dumpTrace( std::ostream &out ) {
        for( const auto &trace_line : trace ) {
            trace_line->printParams(out);
        }
    }

    virtual ~ExContextBase() = default;

protected:
    explicit ExContextBase(const char *message) : _active_exceptions( std::uncaught_exceptions() ), _message(message) {}

    ExContextBase(const ExContextBase &) = delete;
    ExContextBase &operator=(const ExContextBase &) = delete;

    ExContextBase(const ExContextBase &&) = delete;
    ExContextBase &operator=(const ExContextBase &&) = delete;
};

template<typename... T>
class ExContext final : private ExContextBase {
    std::tuple<T...>    _params;

    class Trace final : public TraceBase {
        std::tuple<T...> _params;

    public:
        explicit Trace( const char *message, std::tuple<T...> &&params ) :
            TraceBase(message), _params( std::move(params) )
        {}
        ~Trace() = default;

        void printParams( std::ostream &out ) const override {
            out<<"Context: "<<_message<<" ";

            if constexpr ( sizeof...(T)!=0 ) {
                printParamsTail< 0 >( out );
            }

            out<<"\n";
        }

        template<std::size_t I>
        void printParamsTail( std::ostream &out ) const {
            out<<std::get<I>(_params);

            if constexpr( I+1<sizeof...(T) ) {
                out<<" ";
                printParamsTail<I+1>( out );
            }
        }
    };

public:
    ExContext(const char *message, T... params) : ExContextBase(message), _params( std::forward<T>(params)... ) {}
    ~ExContext() {
        if( std::uncaught_exceptions()>_active_exceptions ) {
            trace.push_back( std::make_unique<Trace>( _message, std::move(_params)) ); 
        }
    }
};
