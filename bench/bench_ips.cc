#include <benchmark/benchmark.h>
#include "iubpatch/formats/ips.h"

using namespace iubpatch;

// IPS parsing with minimal patch
static void BM_IPS_Parse_Minimal(benchmark::State& state) {
    std::vector<Byte> patch_data = {'P', 'A', 'T', 'C', 'H', 'E', 'O', 'F'};
    
    for (auto _ : state) {
        auto result = IPSPatch::load(patch_data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_IPS_Parse_Minimal);

// IPS parsing with single record
static void BM_IPS_Parse_SingleRecord(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'P', 'A', 'T', 'C', 'H',  // Magic
        0x00, 0x00, 0x10,          // Offset 0x000010
        0x00, 0x08,                // Size 8 bytes
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,  // Data
        'E', 'O', 'F'              // EOF
    };
    
    for (auto _ : state) {
        auto result = IPSPatch::load(patch_data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_IPS_Parse_SingleRecord);

// Benchmark IPS parsing with RLE record
static void BM_IPS_Parse_RLE(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'P', 'A', 'T', 'C', 'H',  // Magic
        0x00, 0x00, 0x10,          // Offset 0x000010
        0x00, 0x00,                // Size 0 (RLE)
        0x01, 0x00,                // RLE size 256
        0xAA,                      // RLE value
        'E', 'O', 'F'              // EOF
    };
    
    for (auto _ : state) {
        auto result = IPSPatch::load(patch_data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_IPS_Parse_RLE);

// IPS apply with small source, a simple patch that modifies 8 bytes
static void BM_IPS_Apply_SmallPatch(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'P', 'A', 'T', 'C', 'H',
        0x00, 0x00, 0x10,
        0x00, 0x08,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        'E', 'O', 'F'
    };
    
    auto patch = IPSPatch::load(patch_data).value();
    std::vector<Byte> source(1024, 0x00);  // 1KB source
    
    for (auto _ : state) {
        auto result = patch->apply(source);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * source.size());
}
BENCHMARK(BM_IPS_Apply_SmallPatch);

// IPS apply with RLE = 1024 bytes
static void BM_IPS_Apply_RLE(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'P', 'A', 'T', 'C', 'H',
        0x00, 0x00, 0x00,
        0x00, 0x00,
        0x04, 0x00,
        0xAA,
        'E', 'O', 'F'
    };
    
    auto patch = IPSPatch::load(patch_data).value();
    std::vector<Byte> source(2048, 0x00);  // 2KB source
    
    for (auto _ : state) {
        auto result = patch->apply(source);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * source.size());
}
BENCHMARK(BM_IPS_Apply_RLE);

// IPS apply with multiple records, let's porbably add 10 records
static void BM_IPS_Apply_MultipleRecords(benchmark::State& state) {
    std::vector<Byte> patch_data = {'P', 'A', 'T', 'C', 'H'};
    
    for (int i = 0; i < 10; ++i) {
        patch_data.push_back(0x00);
        patch_data.push_back(static_cast<Byte>(i >> 8));
        patch_data.push_back(static_cast<Byte>(i & 0xFF));
        patch_data.push_back(0x00);
        patch_data.push_back(0x10);  // 16 bytes
        for (int j = 0; j < 16; ++j) {
            patch_data.push_back(static_cast<Byte>(i * 16 + j));
        }
    }
    patch_data.push_back('E');
    patch_data.push_back('O');
    patch_data.push_back('F');
    
    auto patch = IPSPatch::load(patch_data).value();
    std::vector<Byte> source(4096, 0x00);  // 4KB source
    
    for (auto _ : state) {
        auto result = patch->apply(source);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * source.size());
}
BENCHMARK(BM_IPS_Apply_MultipleRecords);
