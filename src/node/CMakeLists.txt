file(GLOB_RECURSE Node *)

source_group("" FILES ${Node})

add_library(Node STATIC ${Node})

target_link_libraries(Node External)

target_include_directories(Node PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
