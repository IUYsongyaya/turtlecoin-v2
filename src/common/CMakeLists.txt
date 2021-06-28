file(GLOB_RECURSE Common *)

source_group("" FILES ${Common})

add_library(Common STATIC ${Common})

target_link_libraries(Common crypto-static)

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
