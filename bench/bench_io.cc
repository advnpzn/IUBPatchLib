#include <benchmark/benchmark.h>
#include "iubpatch/io.h"
#include <fstream>
#include <filesystem>

using namespace iubpatch;

namespace {
    const char* test_file = "/tmp/iubpatch_bench_test.bin";
    
    void create_test_file(std::size_t size) {
        std::ofstream out(test_file, std::ios::binary);
        std::vector<Byte> data(size, 0xAA);
        out.write(reinterpret_cast<const char*>(data.data()), size);
    }
    
    void cleanup_test_file() {
        std::filesystem::remove(test_file);
    }
}

// buffered file reading (small file)
static void BM_BufferedRead_Small(benchmark::State& state) {
    create_test_file(1024);  // 1KB
    
    for (auto _ : state) {
        auto result = read_file(test_file);
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_BufferedRead_Small);

// buffered file reading (medium file)
static void BM_BufferedRead_Medium(benchmark::State& state) {
    create_test_file(1024 * 1024);  // 1MB
    
    for (auto _ : state) {
        auto result = read_file(test_file);
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_BufferedRead_Medium);

// buffered file reading (large file)
static void BM_BufferedRead_Large(benchmark::State& state) {
    create_test_file(10 * 1024 * 1024);  // 10MB
    
    for (auto _ : state) {
        auto result = read_file(test_file);
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 10 * 1024 * 1024);
}
BENCHMARK(BM_BufferedRead_Large);

// file writing (small)
static void BM_FileWrite_Small(benchmark::State& state) {
    std::vector<Byte> data(1024, 0xBB);  // 1KB
    
    for (auto _ : state) {
        auto result = write_file(test_file, std::span<const Byte>(data.data(), data.size()));
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 1024);
}
BENCHMARK(BM_FileWrite_Small);

// file writing (medium)
static void BM_FileWrite_Medium(benchmark::State& state) {
    std::vector<Byte> data(1024 * 1024, 0xBB);  // 1MB
    
    for (auto _ : state) {
        auto result = write_file(test_file, std::span<const Byte>(data.data(), data.size()));
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_FileWrite_Medium);

// file writing (large)
static void BM_FileWrite_Large(benchmark::State& state) {
    std::vector<Byte> data(10 * 1024 * 1024, 0xBB);  // 10MB
    
    for (auto _ : state) {
        auto result = write_file(test_file, std::span<const Byte>(data.data(), data.size()));
        benchmark::DoNotOptimize(result);
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 10 * 1024 * 1024);
}
BENCHMARK(BM_FileWrite_Large);

// BufferedFileReader creation
static void BM_BufferedReader_Open(benchmark::State& state) {
    create_test_file(1024);
    
    for (auto _ : state) {
        auto reader = open_file_reader(test_file);
        benchmark::DoNotOptimize(reader);
    }
    
    cleanup_test_file();
}
BENCHMARK(BM_BufferedReader_Open);

// reading entire file with FileReader
static void BM_FileReader_ReadAll(benchmark::State& state) {
    create_test_file(1024 * 1024);  // 1MB
    
    for (auto _ : state) {
        auto reader = open_file_reader(test_file);
        if (reader.is_ok()) {
            auto result = reader.value()->read_all();
            benchmark::DoNotOptimize(result);
        }
    }
    
    cleanup_test_file();
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
}
BENCHMARK(BM_FileReader_ReadAll);
