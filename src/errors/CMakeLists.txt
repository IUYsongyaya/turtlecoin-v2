file(GLOB_RECURSE Errors *)

source_group("" FILES ${Errors})

add_library(Errors STATIC ${Errors})

target_link_libraries(Errors crypto-static)

target_include_directories(Errors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
