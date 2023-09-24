#include <fiberz/reactor.h>

int fiberBody(int a) {
    while( a>0 ) {
        std::cout<<"Fiber "<<Fiberz::reactor().currentFiberId()<<" running "<<a<<"\n";
        Fiberz::reactor().yield();
        --a;
    }

    return 3;
}

int main() {
    Fiberz::Reactor::init();

    Fiberz::reactor().createFiber( fiberBody, 5 );
    Fiberz::reactor().createFiber( fiberBody, 2 );
    Fiberz::reactor().createFiber( fiberBody, 8 );
    Fiberz::reactor().createFiber( fiberBody, 3 );
    Fiberz::reactor().createFiber( fiberBody, 9 );
    Fiberz::reactor().createFiber( fiberBody, 1 );
    Fiberz::reactor().createFiber( fiberBody, 1 );
    Fiberz::reactor().createFiber( fiberBody, 5 );
    Fiberz::reactor().createFiber( fiberBody, 7 );

    return Fiberz::reactor().start();
}
