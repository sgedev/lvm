
add_executable(lvm-test main.c)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES main.c)
target_link_libraries(lvm-test lvm)
