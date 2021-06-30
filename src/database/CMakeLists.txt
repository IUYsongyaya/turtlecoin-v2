file(GLOB_RECURSE Database *)

source_group("" FILES ${Database})

add_library(Database STATIC ${Database})

target_link_libraries(Database External Errors)

target_include_directories(Database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
