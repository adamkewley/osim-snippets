cmake_minimum_required(VERSION 3.13)

project(osimsnippets VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# libraries: apart from OpenSim, only quickly-compiled C libraries are used, so
# they are just included in the source tree, rather than fucking around
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/glew-2.1.0/build/cmake/)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/freeglut-3.2.1)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/glm-0.9.9.8)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/SDL2-2.0.12)
find_package(OpenGL REQUIRED)
find_package(OpenSim REQUIRED)

# generate top-level configured file that contains version etc.
configure_file("${PROJECT_SOURCE_DIR}/src/OsimsnippetsConfig.h.in" "OsimsnippetsConfig.h")

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

add_executable(osim-snippets
    ${PROJECT_SOURCE_DIR}/third_party/stb_image.h

    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/imgui.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/examples/imgui_impl_opengl3.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/examples/imgui_impl_sdl.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/imgui_demo.cpp

    src/osim-snippets.cpp
    src/expt_wrapp.cpp
    src/opensim_show.cpp
    src/opensim_wrapper.hpp
    src/opensim_wrapper.cpp
    src/size_of_objects.cpp
    src/study_simbody_4_pendulum.cpp
)
target_include_directories(osim-snippets PUBLIC
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}

    ${PROJECT_SOURCE_DIR}/third_party/imgui-1.78/
    ${PROJECT_SOURCE_DIR}/third_party/
)
target_link_libraries(osim-snippets
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    glew_s
    ${GLUT_LIBRARIES}
    snippets_common_options
)

add_subdirectory(src/simbody_cable)


# BUILD LIB PATHS:
#
# in Windows, copy libraries to the exe dir, because Windows specifically
# searches in the exe's dir for libraries. see:
#
# https://stackoverflow.com/questions/44909846/cmake-exe-cant-find-dll
if (WIN32)
    set(STATIC_LIB_PATHS ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${GLEW_LIBRARIES})
    foreach(STATIC_LIB ${STATIC_LIB_PATHS})
        string(REGEX REPLACE ".lib$" ".dll" DLL_PATH ${STATIC_LIB})
        if(EXISTS ${DLL_PATH})
            add_custom_command(
                TARGET opensim_show
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_PATH}" "$<TARGET_FILE_DIR:opensim_show>")
        endif()
    endforeach()

    file(GLOB OPENSIM_LIBS LIST_DIRECTORIES FALSE "${OpenSim_ROOT_DIR}/bin/*.dll")
    foreach(OPENSIM_LIB ${OPENSIM_LIBS})
        add_custom_command(
            TARGET opensim_show
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENSIM_LIB}" "$<TARGET_FILE_DIR:opensim_show>")
    endforeach()

    add_custom_command(
        TARGET opensim_show
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_LIBRARY} "${CMAKE_BINARY_DIR}"
    )
endif()
