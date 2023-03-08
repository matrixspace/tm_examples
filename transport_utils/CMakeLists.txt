add_executable(tm_listener listener.cpp)
target_link_libraries(tm_listener ${TM_EXAMPLES_LINKER_ITEMS})
set_target_properties(tm_listener PROPERTIES FOLDER "transport_utils")

add_executable(tm_capturer capturer.cpp)
target_link_libraries(tm_capturer ${TM_EXAMPLES_LINKER_ITEMS})
set_target_properties(tm_capturer PROPERTIES FOLDER "transport_utils")

add_executable(tm_delay_measurer delay_measurer.cpp)
target_link_libraries(tm_delay_measurer ${TM_EXAMPLES_LINKER_ITEMS})
set_target_properties(tm_delay_measurer PROPERTIES FOLDER "transport_utils")