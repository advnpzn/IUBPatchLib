#include <benchmark/benchmark.h>
#include "iubpatch/formats/bps.h"

using namespace iubpatch;

// Benchmark BPS parsing with minimal patch
static void BM_BPS_Parse_Minimal(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'B', 'P', 'S', '1',  // Magic
        0x80,                 // src_size = 0
        0x80,                 // target_size = 0
        0x80,                 // metadata_size = 0
        0x00, 0x00, 0x00, 0x00,  // src_crc
        0x00, 0x00, 0x00, 0x00,  // target_crc
        0x00, 0x00, 0x00, 0x00   // patch_crc
    };
    
    for (auto _ : state) {
        auto result = BPSPatch::load(patch_data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BPS_Parse_Minimal);

// Benchmark BPS apply with empty patch
static void BM_BPS_Apply_Empty(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'B', 'P', 'S', '1', // Magic
        0x80,  // src_size = 0
        0x80,  // target_size = 0
        0x80,  // metadata_size = 0
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    
    auto patch = BPSPatch::load(patch_data).value();
    std::vector<Byte> source;
    
    for (auto _ : state) {
        auto result = patch->apply(source);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BPS_Apply_Empty);
