file(GLOB_RECURSE Seed_Node *)

source_group("" FILES ${Seed_Node})

add_executable(TurtleCoinSeed ${Seed_Node} ${WIN32_ICON_FILE})

target_link_libraries(TurtleCoinSeed P2P Utilities)

set_property(TARGET TurtleCoinSeed PROPERTY OUTPUT_NAME "TurtleCoinSeed")

