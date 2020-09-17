add_executable(show_model
    ${CMAKE_CURRENT_LIST_DIR}/show_model.cpp
)
target_link_libraries(show_model
    snippets_common_options
)
