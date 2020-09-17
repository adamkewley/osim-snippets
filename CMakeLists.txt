cmake_minimum_required(VERSION 3.13)

project(osimsnippets VERSION 0.0.1)

configure_file("${PROJECT_SOURCE_DIR}/src/OsimsnippetsConfig.h.in" "OsimsnippetsConfig.h")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(OpenSim REQUIRED)
find_package(Simbody REQUIRED)

# INTERFACE target that transitively sets common compiler options on any
# targets that link to it
add_library(snippets_common_options INTERFACE)
target_compile_options(snippets_common_options INTERFACE
    # disable MSVC permissiveness. Forces MSVC to obey C++ standard
    # - specifically, this enables the use of 'and', 'not', etc.
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)
target_link_libraries(snippets_common_options INTERFACE
    osimLepton
    osimCommon
    osimSimulation
    osimActuators
    osimAnalyses
    osimTools
    SimTKcommon
    SimTKmath
    SimTKsimbody
)
target_include_directories(snippets_common_options INTERFACE
  ${PROJECT_BINARY_DIR}
)

add_subdirectory(src/size_of_objects)
add_subdirectory(src/study_simbody_4_pendulum)
add_subdirectory(src/experiment_wrapping_surfs)
add_subdirectory(src/show_model)
add_subdirectory(src/simbody_cable)
add_subdirectory(src/opensim_show)
