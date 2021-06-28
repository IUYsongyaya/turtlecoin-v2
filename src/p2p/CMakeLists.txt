file(GLOB_RECURSE P2P *)

source_group("" FILES ${P2P})

add_library(P2P STATIC ${P2P})

target_link_libraries(P2P crypto-static ipv6-parser Database Networking Errors)

target_include_directories(P2P PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
