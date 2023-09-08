#include <fiberz/fiberz.h>

#include "reactor.h"

namespace Fiberz {

using namespace Internal;

void init(Params params) {
    ASSERT( !the_reactor, "Tried to initialize an already initialized Fiberz reactor" );

    the_reactor.emplace(params);
}

int start() {
    ASSERT( !!the_reactor, "Fiberz::init was not called");

    return the_reactor->start();
}

void yield() {
    ASSERT( !!the_reactor, "Fiberz::init was not called");

    the_reactor->yield();
}

FiberHandle currentFiberHandle() {
    ASSERT( !!the_reactor, "Fiberz::init was not called");

    return the_reactor->currentFiberHandle();
}

namespace Internal {

FiberHandle createFiber( std::unique_ptr<ParametersBase> parameters ) {
    ASSERT( !!the_reactor, "Fiberz::init was not called");

    return the_reactor->createFiber( std::move(parameters) );
}

}

} // namespace Fiberz
