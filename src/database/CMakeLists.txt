file(GLOB_RECURSE Database *)

source_group("" FILES ${Database})

add_library(Database STATIC ${Database})

target_link_libraries(Database lmdb cppfs crypto-static)
target_include_directories(Database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
