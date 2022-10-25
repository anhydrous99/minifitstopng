cmake_minimum_required(VERSION 3.22)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
project(minifitstopng)

set(CMAKE_CXX_STANDARD 17)

find_package(ASCDM REQUIRED)
find_package(cxxopts REQUIRED)
find_package(PNG REQUIRED)
find_package(GSL REQUIRED)

add_executable(minifitstopng main.cpp stk.h stk.cpp hist.h hist.cpp)
target_link_libraries(minifitstopng cxxopts::cxxopts CIAO::ASCDM PNG::PNG GSL::gsl)
configure_file("${CMAKE_SOURCE_DIR}/par_run.py" "${CMAKE_CURRENT_BINARY_DIR}/par_run.py" COPYONLY)
