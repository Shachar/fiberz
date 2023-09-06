#include <fiberz/fiberz.h>

#include "reactor.h"

#include <optional>

namespace Fiberz {

using namespace Internal;

static thread_local std::optional<Reactor> the_reactor;

void init(Params params) {
    ASSERT( !the_reactor, "Tried to initialize an already initialized Fiberz reactor" );

    the_reactor.emplace(params);
}

} // namespace Fiberz
