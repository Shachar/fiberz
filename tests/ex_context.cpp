#include <fiberz/utility/excontext.h>

#include <array>
#include <filesystem>
#include <iostream>

void open(unsigned index, const std::filesystem::path &path) {
    auto ex_context = ExContext("OS open", path);

    if( index==3 )
        throw std::exception();
}

void init() {
    auto ex_context = ExContext("Init start");

    const std::array paths{
        std::filesystem::path("/etc/passwd"),
        std::filesystem::path("/home/shachar/secrets"),
        std::filesystem::path("/etc/shadow"),
        std::filesystem::path("/home/something"),
    };

    for( unsigned i=0; i<paths.size(); ++i ) {
        auto ex_context = ExContext("Opening file", i, paths[i]);

        open(i, paths[i]);
    }
}


int main() {
    try {
        auto ex_context = ExContext("Start of program", 5, 25);

        init();
    } catch( const std::exception &ex ) {
        std::cerr<<"Caught exception "<<ex.what()<<"\n";
        ExContextBase::dumpTrace(std::cerr);
    }
}
