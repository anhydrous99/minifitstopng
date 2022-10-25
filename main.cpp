#include "stk.h"
#include "hist.h"

#include <iostream>
#include <cxxopts.hpp>

int main(int argc, char** argv) {
    cxxopts::Options options("minifitstopng");
    options.add_options()
            ("e,evt3", "Event3 Files", cxxopts::value<std::string>())
            ("o,output", "Output directory", cxxopts::value<std::string>())
            ("h,help", "Print usage");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    auto evt3_paths = stack_build(result["evt3"].as<std::string>());

    calc_histogram(evt3_paths[0], result["output"].as<std::string>());

    return 0;
}
