#include "fiberz/fiberz.h"

int fiberBody(int a) {
    std::cout<<"Fiber "<<a<<" running\n";

    return 3;
}

int main() {
    Fiberz::init();

    Fiberz::createFiber( fiberBody, 2 );

    return Fiberz::start();
}
