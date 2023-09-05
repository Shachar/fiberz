#include "fiberz/reactor.h"

int main() {
    Fiberz::Reactor::init();

    Fiberz::FiberId id;

    std::cout<<Fiberz::FiberId(12)<<"\n";
    std::cout<<"\""<<std::setw(10)<<12<<"\"\n";

    std::unordered_map<Fiberz::FiberId, int> hash;
}
