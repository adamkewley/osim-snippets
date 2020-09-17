add_executable(experiment_wrapping_surfs
    ${CMAKE_CURRENT_LIST_DIR}/experiment_wrapping_surfs.cpp
)
target_link_libraries(experiment_wrapping_surfs
    snippets_common_options
)
