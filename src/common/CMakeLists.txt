file(GLOB_RECURSE Common *)

source_group("" FILES ${Common})

add_library(Common STATIC ${Common})

target_link_libraries(Common External Errors)

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
