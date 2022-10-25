#include "stk.h"
#include "hist.h"

#include <iostream>
#include <cxxopts.hpp>
#include <filesystem>
#include <execution>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    cxxopts::Options options("minifitstopng");
    options.add_options()
            ("e,evt3", "Event3 Files", cxxopts::value<std::string>())
            ("o,output", "Output directory", cxxopts::value<fs::path>())
            ("s,scale", "Scaling (linear or log)", cxxopts::value<std::string>()->default_value("linear"))
            ("h,help", "Print usage");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::string scale = result["scale"].as<std::string>();
    fs::path output = result["output"].as<fs::path>();
    if (!is_directory(output)) {
        std::cerr << "Output must be a directory!\n";
        return EXIT_FAILURE;
    }

    auto evt3_paths = stack_build(result["evt3"].as<std::string>());

    std::for_each(evt3_paths.begin(), evt3_paths.end(),
                  [&](const std::string &path) {
                      fs::path p_path{path};
                      fs::path png_path{output.string() + p_path.stem().string() + ".png"};
                      calc_histogram(path, png_path.string(), scale);
                  });

    return EXIT_SUCCESS;
}
