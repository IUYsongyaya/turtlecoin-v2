file(GLOB_RECURSE Logger *)

source_group("" FILES ${Logger})

add_library(Logger STATIC ${Logger})

target_link_libraries(Logger crypto-static)
target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
