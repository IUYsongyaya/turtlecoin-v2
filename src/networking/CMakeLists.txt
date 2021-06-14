file(GLOB_RECURSE Networking *)

source_group("" FILES ${Networking})

add_library(Networking STATIC ${Networking})

target_link_libraries(Networking crypto-static libzmq-static libminiupnpc-static Errors)

if(WIN32)
    target_link_libraries(Networking ws2_32 advapi32 crypt32 gdi32 user32)
endif()

target_include_directories(Networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
