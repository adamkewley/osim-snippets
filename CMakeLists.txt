cmake_minimum_required(VERSION 3.13)

project(osimsnippets)

set(OPENSIM_INSTALL ~/Desktop/opensim-core/opensim-core-install)
set(SIMBODY_INSTALL ~/Desktop/opensim-core/opensim-dependencies-install/simbody)
set(ALL_INCLUDES
  ${OPENSIM_INSTALL}/include
  ${SIMBODY_INSTALL}/include/simbody
  ${OPENSIM_INSTALL}/include/OpenSim)
set(LIB_DIR ${OPENSIM_INSTALL}/lib)
set(ALL_LIBS
  SimTKcommon
  SimTKmath
  SimTKsimbody
  osimCommon
  osimActuators
  osimAnalyses
  osimSimulation
  osimTools)

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
