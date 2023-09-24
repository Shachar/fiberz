#pragma once

static inline void ASSERT_HELPER( bool cond, const char *message, const char *file, unsigned line ) {
    if( !cond ) {
        std::cerr<<"ASSERTION FAILED "<<file<<":"<<line<<": "<<message<<"\n";

        abort();
    }
}

#define ASSERT(cond, message) ASSERT_HELPER((cond), (message), __FILE__, __LINE__) 


