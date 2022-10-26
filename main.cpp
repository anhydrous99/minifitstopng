#include "stk.h"
#include "hist.h"

#include <iostream>
#include <cxxopts.hpp>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    cxxopts::Options options("minifitstopng");
    options.add_options()
            ("e,evt3", "Event3 Files", cxxopts::value<std::string>())
            ("o,output", "Output directory", cxxopts::value<fs::path>())
            ("s,scale", "Scaling (linear or log)", cxxopts::value<std::string>())
            ("h,help", "Print usage");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if ((result.count("evt3") + result.count("output") + result.count("scale")) != 3) {
        std::cerr << "Missing parameters\n";
        return EXIT_FAILURE;
    }

    std::string scale = result["scale"].as<std::string>();
    fs::path output = result["output"].as<fs::path>();
    if (!is_directory(output)) {
        std::cerr << "Output must be a directory!\n";
        return EXIT_FAILURE;
    }

    auto evt3_paths = stack_build(result["evt3"].as<std::string>());

    for (const auto& path: evt3_paths) {
	    fs::path p_path{path};
	    std::string png_path{output.string() + "/" + p_path.stem().string() + ".png"};
        std::cout << "Processing " << png_path << std::endl;
	    calc_histogram(path, png_path, scale);
    }

    return EXIT_SUCCESS;
}
