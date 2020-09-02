cmake_minimum_required(VERSION 3.13)

project(osimsnippets)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)
find_package(OpenSim REQUIRED)


# INTERFACE target that transitively sets common compiler options on any
# targets that link to it
add_library(snippets_common_options INTERFACE)
target_compile_options(snippets_common_options INTERFACE
    # disable MSVC permissiveness. Forces MSVC to obey C++ standard
    # - specifically, this enables the use of 'and', 'not', etc.
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)
target_include_directories(snippets_common_options INTERFACE
    ${OpenSim_INCLUDE_DIRS}
    ${Simbody_INCLUDE_DIR}
    ${OpenSim_INCLUDE_DIRS}/OpenSim
)
target_link_libraries(snippets_common_options INTERFACE
    ${OpenSim_LIBRARIES}
    ${Simbody_LIBRARIES}
)


# TARGET: size_of_objects
add_executable(size_of_objects
    src/size_of_objects.cpp
)
target_link_libraries(size_of_objects
    snippets_common_options
)


# TARGET: study_simbody_4_pendulum
add_executable(study_simbody_4_pendulum
    src/study_simbody_4_pendulum.cpp
)
target_link_libraries(study_simbody_4_pendulum
    snippets_common_options
)


# TARGET: experiment_wrapping_surfs
add_executable(experiment_wrapping_surfs
    src/experiment_wrapping_surfs.cpp
)
target_link_libraries(experiment_wrapping_surfs
    snippets_common_options
)


# TARGET: show_model
add_executable(show_model
    src/show_model.cpp
)
target_link_libraries(show_model
    snippets_common_options
)


# TARGET: opensim_show
add_executable(opensim_show
    src/opensim_show.cpp
    src/stb_image.h
    src/opensim_wrapper.hpp
    src/opensim_wrapper.cpp

    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/examples/imgui_impl_opengl3.cpp
    imgui/examples/imgui_impl_sdl.cpp
    imgui/imgui_demo.cpp
)
target_include_directories(opensim_show PUBLIC
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    imgui/
)
target_link_libraries(opensim_show
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_LIBRARY}
    ${SDL2_TTF_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    snippets_common_options
)
add_custom_command(
    TARGET
    opensim_show
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/FantasqueSansMono-Regular.ttf $<TARGET_FILE_DIR:opensim_show>)
add_custom_command(
    TARGET
    opensim_show
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/container.jpg $<TARGET_FILE_DIR:opensim_show>)
add_custom_command(
    TARGET
    opensim_show
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/awesomeface.png $<TARGET_FILE_DIR:opensim_show>)
add_custom_command(
    TARGET
    opensim_show
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/opensim-models/Models/Arm26 $<TARGET_FILE_DIR:opensim_show>/Arm26)
add_custom_command(
    TARGET
    opensim_show
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/opensim-models/Models/RajagopalModel $<TARGET_FILE_DIR:opensim_show>/RajagopalModel)
