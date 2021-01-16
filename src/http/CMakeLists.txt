file(GLOB_RECURSE HTTP *)

source_group("" FILES ${HTTP})

add_library(HTTP STATIC ${HTTP})

target_link_libraries(HTTP crypto-static)
target_include_directories(HTTP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
