#include <fiberz/reactor.h>

int waiterFiber(int a) {
    while(a>0) {
        std::cout<<a<<" iterations to go fiber "<<Fiberz::reactor().currentFiberId()<<"\n";
        Fiberz::reactor().yield();
        a--;
    }

    return 0;
}

int killerFiber(Fiberz::FiberHandle handle) {
    std::cout<<"Killing "<<handle<<"\n";
    Fiberz::reactor().killFiber( handle );
    Fiberz::reactor().yield();
    std::cout<<"Killed?\n";

    return 0;
}

void testFiber() {
    using Fiberz::reactor, Fiberz::FiberHandle;

    FiberHandle fh = reactor().createFiber( waiterFiber, 12 );
    std::cout<<"Created "<<fh<<" waiter\n";
    reactor().killFiber(fh);
    std::cout<<"Killed before it even started "<<fh<<"\n";

    assert( reactor().isValid(fh) );
    reactor().yield();

    assert( !reactor().isValid(fh) );

    fh = reactor().createFiber( waiterFiber, 12 );
    std::cout<<"Created "<<fh<<" waiter\n";

    assert( reactor().isValid(fh) );
    reactor().yield();
    assert( reactor().isValid(fh) );
    reactor().yield();
    assert( reactor().isValid(fh) );
    reactor().yield();
    assert( reactor().isValid(fh) );
    reactor().killFiber(fh);
    assert( reactor().isValid(fh) );
    reactor().yield();
    assert( !reactor().isValid(fh) );

    fh = reactor().createFiber( waiterFiber, 12 );
    std::cout<<"Created "<<fh<<" waiter\n";
    assert( reactor().isValid(fh) );
    reactor().killFiber(fh);
    std::cout<<"Killed before it even started "<<fh<<"\n";
    reactor().yield();
    assert( !reactor().isValid(fh) );

    fh = reactor().currentFiberHandle();
    std::cout<<"Committing suicide "<<fh<<"\n";

    reactor().killFiber(fh);
    std::cout<<"Kill command is pending\n";

    reactor().yield();
    std::cout<<"This line was supposed to be unreachable!\n";
    abort();
}

int main() {
    Fiberz::Reactor::init();

    Fiberz::reactor().createFiber( testFiber );

    return Fiberz::reactor().start();
}
