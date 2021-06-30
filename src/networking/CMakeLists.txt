file(GLOB_RECURSE Networking *)

source_group("" FILES ${Networking})

add_library(Networking STATIC ${Networking})

target_link_libraries(Networking External Errors Logger)

target_include_directories(Networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
