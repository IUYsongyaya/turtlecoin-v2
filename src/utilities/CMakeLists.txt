file(GLOB_RECURSE Utilities *)

source_group("" FILES ${Utilities})

add_library(Utilities STATIC ${Utilities})

target_link_libraries(Utilities crypto-static)
target_include_directories(Utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
