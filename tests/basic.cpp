#include "fiberz/fiberz.h"

int fiberBody(int a) {
    while( a>0 ) {
        std::cout<<"Fiber "<<Fiberz::currentFiberId()<<" running "<<a<<"\n";
        Fiberz::yield();
        --a;
    }

    return 3;
}

int main() {
    Fiberz::init();

    Fiberz::createFiber( fiberBody, 5 );
    Fiberz::createFiber( fiberBody, 2 );
    Fiberz::createFiber( fiberBody, 8 );
    Fiberz::createFiber( fiberBody, 3 );
    Fiberz::createFiber( fiberBody, 9 );
    Fiberz::createFiber( fiberBody, 1 );
    Fiberz::createFiber( fiberBody, 1 );
    Fiberz::createFiber( fiberBody, 5 );
    Fiberz::createFiber( fiberBody, 7 );

    return Fiberz::start();
}
