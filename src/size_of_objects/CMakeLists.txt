add_executable(size_of_objects
    ${CMAKE_CURRENT_LIST_DIR}/size_of_objects.cpp
)
target_link_libraries(size_of_objects
    snippets_common_options
)
