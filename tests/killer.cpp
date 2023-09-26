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

int main() {
    Fiberz::Reactor::init();

    Fiberz::FiberHandle fh = Fiberz::reactor().createFiber( waiterFiber, 5 );
    Fiberz::reactor().createFiber( killerFiber, fh );

    return Fiberz::reactor().start();
}
