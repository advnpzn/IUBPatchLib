#include <benchmark/benchmark.h>
#include "iubpatch/formats/ups.h"

using namespace iubpatch;

// UPS parsing with minimal patch
static void BM_UPS_Parse_Minimal(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'U', 'P', 'S', '1',  // Magic
        0x80,                 // src_size = 0
        0x80,                 // target_size = 0
        0x00, 0x00, 0x00, 0x00,  // src_crc
        0x00, 0x00, 0x00, 0x00,  // target_crc
        0x00, 0x00, 0x00, 0x00   // patch_crc
    };
    
    for (auto _ : state) {
        auto result = UPSPatch::load(patch_data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UPS_Parse_Minimal);

// UPS apply with empty patch
static void BM_UPS_Apply_Empty(benchmark::State& state) {
    std::vector<Byte> patch_data = {
        'U', 'P', 'S', '1',
        0x80,  // src_size = 0
        0x80,  // target_size = 0
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    
    auto patch = UPSPatch::load(patch_data).value();
    std::vector<Byte> source;
    
    for (auto _ : state) {
        auto result = patch->apply(source);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UPS_Apply_Empty);
