include(FetchContent)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(iubpatch_bench
  bench_main.cc
  bench_ips.cc
  bench_ups.cc
  bench_bps.cc
  bench_io.cc
)

target_link_libraries(iubpatch_bench 
  PRIVATE 
    iubpatch
    benchmark::benchmark
)
