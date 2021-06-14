set(UPNPC_BUILD_STATIC ON CACHE BOOL "Build static library")
set(UPNPC_BUILD_SHARED OFF CACHE BOOL "Build shared library")
set(UPNPC_BUILD_TESTS OFF CACHE BOOL "Build test executables")

add_subdirectory(cppfs)
add_subdirectory(crypto)
add_subdirectory(cxxfolders)
add_subdirectory(lmdb)
add_subdirectory(libzmq)
add_subdirectory(miniupnp/miniupnpc)

target_include_directories(libminiupnpc-static PUBLIC miniupnp/miniupnpc)

set_property(TARGET cppfs PROPERTY FOLDER "external")
set_property(TARGET crypto-static PROPERTY FOLDER "external")
set_property(TARGET platform_folders PROPERTY FOLDER "external")
set_property(TARGET lmdb PROPERTY FOLDER "external")
set_property(TARGET libzmq-static PROPERTY FOLDER "external")
set_property(TARGET libminiupnpc-static PROPERTY FOLDER "external")
