#include <fiberz/reactor.h>

#include <fiberz/containers/compact_intrusive_list.h>

void somefunc();

class SomeData : public boost::intrusive_ref_counter<SomeData, boost::thread_unsafe_counter> {
    friend void somefunc();

    Fiberz::Containers::CompactIntrusiveList_Node node;
    uint64_t datum;

public:
    explicit SomeData( uint64_t d = 0 ) : datum(d) {
        std::cout<<"Constructed SomeData "<<datum<<"\n";
    }

    ~SomeData() {
        std::cout<<"Destructed SomeData "<<datum<<"\n";
    }
};

void somefunc() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    Fiberz::Containers::CompactIntrusiveList<SomeData, offsetof(SomeData, node)> list;
#pragma GCC diagnostic pop

    list.push_front( new SomeData(2) );
    list.push_back( new SomeData(3) );
    list.push_front( new SomeData(1) );

    list.front();
    list.back();
}

void testFiber() {
}

int main() {
    /*
    Fiberz::Reactor::init();

    Fiberz::reactor().createFiber( testFiber );

    return Fiberz::reactor().start();
    */

    somefunc();
}
