#include <fiberz/reactor.h>

#include <fiberz/containers/compact_intrusive_list.h>

Fiberz::TimePoint base;

void recurring(uint32_t &counter) {
    counter++;
}

void testFiber() {
    uint32_t recurring_count = 0, one_time_count = 0;
    base = Fiberz::now();

    auto recurringTimer = Fiberz::reactor().registerRecurringTimer( 3ms, [&recurring_count]() { recurring(recurring_count); } );

    Fiberz::reactor().sleep( 10ms );

    assert( recurring_count==3 );
}

int main() {
    Fiberz::Reactor::init();

    Fiberz::reactor().createFiber( testFiber );

    return Fiberz::reactor().start();
}
