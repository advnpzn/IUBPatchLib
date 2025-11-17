include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(iubpatch_tests
  test_main.cc
  test_ips.cc
  test_ups.cc
  test_bps.cc
  test_io.cc
  test_errors.cc
  test_apply.cc
)

target_link_libraries(iubpatch_tests 
  PRIVATE 
    iubpatch
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(iubpatch_tests)
