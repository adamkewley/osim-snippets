cmake_minimum_required(VERSION 3.13)

project(osimsnippets)

set(CMAKE_CXX_STANDARD 20)

set(OPENSIM_INSTALL /Users/akewley/Desktop/opensim-core/osc-debug-install)
set(SIMBODY_INSTALL /Users/akewley/Desktop/opensim-core/osc-deps-debug-install/simbody)
set(ALL_INCLUDES
  ${OPENSIM_INSTALL}/include
  ${SIMBODY_INSTALL}/include/simbody
  ${OPENSIM_INSTALL}/include/OpenSim)
set(LIB_DIR ${OPENSIM_INSTALL}/lib)
set(ALL_LIBS
  SimTKcommon_d
  SimTKmath_d
  SimTKsimbody_d
  osimCommon_d
  osimActuators_d
  osimAnalyses_d
  osimSimulation_d
  osimTools_d)

add_executable(size_of_objects src/size_of_objects.cpp)
target_include_directories(size_of_objects PUBLIC ${ALL_INCLUDES})
target_link_directories(size_of_objects PUBLIC ${LIB_DIR})
target_link_libraries(size_of_objects ${ALL_LIBS})

add_executable(study_simbody_4_pendulum src/study_simbody_4_pendulum.cpp)
target_include_directories(study_simbody_4_pendulum PUBLIC ${ALL_INCLUDES})
target_link_directories(study_simbody_4_pendulum PUBLIC ${LIB_DIR})
target_link_libraries(study_simbody_4_pendulum ${ALL_LIBS})

add_executable(experiment_wrapping_surfs src/experiment_wrapping_surfs.cpp)
target_include_directories(experiment_wrapping_surfs PUBLIC ${ALL_INCLUDES})
target_link_directories(experiment_wrapping_surfs PUBLIC ${LIB_DIR})
target_link_libraries(experiment_wrapping_surfs ${ALL_LIBS})

add_executable(show_model src/show_model.cpp)
target_include_directories(show_model PUBLIC ${ALL_INCLUDES})
target_link_directories(show_model PUBLIC ${LIB_DIR})
target_link_libraries(show_model ${ALL_LIBS})

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
#find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

add_executable(opensim_show src/opensim_show.cpp src/stb_image.h src/opensim_wrapper.hpp src/opensim_wrapper.cpp)
target_include_directories(opensim_show PUBLIC
    ${ALL_INCLUDES}
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
)
target_link_directories(opensim_show PUBLIC
    ${LIB_DIR}
)
target_link_libraries(opensim_show
    ${ALL_LIBS}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_LIBRARY}
    ${SDL2_TTF_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    GLEW::glew_s
    glm::glm
#    GLUT::GLUT
)
