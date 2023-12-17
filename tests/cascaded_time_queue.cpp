#include <fiberz/internal/cascaded_time_queue.h>

#include <iostream>
#include <map>
#include <random>

using namespace Fiberz;

std::map<TimePoint, Internal::CascadedTimeQueue::TimerHandle> events;

int main(int argc, char *argv[]) {
    std::random_device r;
    auto seed = r();
    size_t num_iterations = 700;

    if( argc>1 )
        num_iterations = strtoul(argv[1], nullptr, 0);
    if( argc>2 )
        seed = strtoul(argv[2], nullptr, 0);

    std::cout<<"Running cascaded time queue test with seed "<<seed<<"\n";
    std::default_random_engine e1(seed);
    std::uniform_int_distribution<long> uniform_dist(-100, 100000);

    TimePoint now = TimePoint( Duration(1500) );

    const Duration resolution = 3000ns;
    Internal::CascadedTimeQueue tq(now, 6, resolution);

    now += 5000ms;
    assert( ! tq.executeEvent( now ) );

    auto start_time = now;
    for( unsigned i=0; i<num_iterations; ++i ) {
        auto time = now + std::chrono::milliseconds( uniform_dist(e1) );

        if( events.find( time ) != events.end() )
            continue;

        events.emplace(
                time,
                tq.insertEventWithHandle(
                    time,
                    [&now, resolution, start_time](TimePoint tp){
                        auto iter = events.find( tp );
                        assert( iter != events.end() );
                        auto corrected_now = now-resolution;
                        auto handle_time = tp;
                        assert( handle_time<start_time || corrected_now < handle_time );
                        events.erase( iter );
                    } )
                );
    }

    TimePoint nextEvent = tq.nextEvent();
    do {
        now = nextEvent;

        while( tq.executeEvent(now) ) {
        }

        nextEvent = tq.nextEvent();
    } while( nextEvent != TimePoint::max() );

    assert( events.empty() );
    assert( !tq.executeEvent( now+5000s ) );

    std::cout<<"Test passed\n";
}
