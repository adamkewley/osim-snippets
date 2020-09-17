add_executable(study_simbody_4_pendulum
    ${CMAKE_CURRENT_LIST_DIR}/study_simbody_4_pendulum.cpp
)
target_link_libraries(study_simbody_4_pendulum
    snippets_common_options
)
