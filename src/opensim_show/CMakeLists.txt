set(GLEW_USE_STATIC_LIBS TRUE)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

add_executable(opensim_show
    ${PROJECT_SOURCE_DIR}/third_party/stb_image.h

    ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui/examples/imgui_impl_opengl3.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui/examples/imgui_impl_sdl.cpp
    ${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp

    opensim_show.cpp
    opensim_wrapper.hpp
    opensim_wrapper.cpp
)
target_include_directories(opensim_show PUBLIC
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}

    ${PROJECT_SOURCE_DIR}/third_party/imgui/
    ${PROJECT_SOURCE_DIR}/third_party/
)
target_compile_definitions(opensim_show
    PRIVATE
        GLEW_STATIC
)
target_link_libraries(opensim_show
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    GLEW::glew_s
    ${GLUT_LIBRARIES}
    snippets_common_options
)
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
endif()

add_custom_command(
    TARGET opensim_show
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_LIBRARY} "${CMAKE_BINARY_DIR}"
)

# INSTALL RPATH
#
# see: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#
# effectively, this build just dumps all the binaries into a single
# dir to keep Windows happy, and then fudges the `RPATH` on Linux +
# MacOS to search for libraries in the install dir
set_target_properties(opensim_show
    PROPERTIES
        INSTALL_RPATH "$<UNIX:\$ORIGIN>"
        INSTALL_RPATH "$<APPLE:@executable_path>"
)

# INSTALL STEPS
#
# just dump all binaries into a single bin/ dir, because Windows
# doesn't support rpaths (so we can't easily do something like
# $ORIGIN/../lib) and this install doesn't need to be particularly
# fancy (it's better that it is *simple*, rather than *correct*)
install(
    TARGETS
        opensim_show
    RUNTIME
        DESTINATION bin/
    LIBRARY
        DESTINATION bin/
    ARCHIVE
        DESTINATION bin/
)
file(GLOB osim_libs LIST_DIRECTORIES false "${OpenSim_LIB_DIR}/*")
install(FILES ${osim_libs} DESTINATION bin/)
