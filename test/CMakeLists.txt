
cmake_minimum_required(VERSION 3.14)

include(CTest)

enable_testing()

function(add_test_list)
    set(TEST_LIST)

    foreach(test IN LISTS ARGN)
        add_executable(${test}_test ${test}.cpp)

        target_link_libraries(${test}_test modern_bloom::modern_bloom)
        target_compile_features(${test}_test PRIVATE cxx_std_20)

        add_test(NAME ${test}_test COMMAND ${test}_test)

        set_property(
            TEST ${test}_test
            PROPERTY FAIL_REGULAR_EXPRESSION ".*FAILED.*"
        )

        add_custom_target(
            run_${test}
            COMMAND ${CMAKE_CTEST_COMMAND} -C Debug -R ${test}_test
            DEPENDS ${test}_test
        )

        set(TEST_LIST ${TEST_LIST} ${test}_test)
    endforeach()

    add_custom_target(
        run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -C Debug
        DEPENDS ${TEST_LIST}
    )
endfunction(add_test_list)

add_test_list(
    bloom_filter
    dynamic_bloom
    static_bloom
)
