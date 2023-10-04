#include <fiberz/utility/excontext.h>

thread_local std::vector< std::unique_ptr<ExContextBase::TraceBase> > ExContextBase::trace;
