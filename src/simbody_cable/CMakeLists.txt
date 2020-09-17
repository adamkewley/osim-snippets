add_executable(simbody_cable
    ${CMAKE_CURRENT_LIST_DIR}/OpenSimPartyDemoCable.cpp
)
target_link_libraries(simbody_cable
    snippets_common_options
)
