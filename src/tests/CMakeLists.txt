file(GLOB test_files "*.cpp")

# Doing it in this way probably isn't the best way, but it does work
foreach(test_file ${test_files})
    get_filename_component(TEST_NAME ${test_file} NAME_WE)

    get_filename_component(TEST_FILE ${test_file} NAME)

    message(STATUS "Activating test source: ${TEST_FILE}")

    add_executable("${TEST_NAME}_test" ${TEST_FILE} ${WIN32_ICON_FILE})

    target_link_libraries("${TEST_NAME}_test" crypto-static ipv6-parser Core Utilities P2P Networking)

    if(WIN32)
        target_link_libraries("${TEST_NAME}_test" ws2_32 crypt32)
    endif()

    set_property(TARGET "${TEST_NAME}_test" PROPERTY OUTPUT_NAME "test_${TEST_NAME}")
endforeach()
