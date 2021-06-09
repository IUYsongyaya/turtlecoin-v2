file(GLOB_RECURSE Core *)

source_group("" FILES ${Core})

add_library(Core STATIC ${Core})

target_link_libraries(Core crypto-static Database Errors)
target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
