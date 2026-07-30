#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/huffman.hpp"
#include "../include/lz77.hpp"

namespace {

using Clock = std::chrono::steady_clock;

constexpr std::size_t kFileOverhead = 4 + 3 * sizeof(std::uint32_t) +
                                      256 * sizeof(std::uint32_t);

struct CompressedData {
    std::vector<std::uint8_t> packed;
    std::vector<std::uint32_t> frequencies;
    std::vector<std::uint8_t> bits;
};

struct BenchmarkResult {
    std::size_t input_size;
    std::size_t output_size;
    double compression_ms;
    double decompression_ms;
};

CompressedData compress(const std::vector<std::uint8_t>& input) {
    CompressedData result;
    result.packed = lz77_compress_packed(input);
    result.frequencies = huff_build_freq(result.packed);
    result.bits = huff_encode(result.packed, result.frequencies);
    return result;
}

std::vector<std::uint8_t> decompress(const CompressedData& input,
                                     std::size_t original_size) {
    auto packed =
        huff_decode(input.bits, input.frequencies, input.packed.size());
    return lz77_decompress_packed(packed, original_size);
}

double median_ms(std::vector<double> samples) {
    std::sort(samples.begin(), samples.end());
    const std::size_t middle = samples.size() / 2;
    if (samples.size() % 2 != 0) {
        return samples[middle];
    }
    return (samples[middle - 1] + samples[middle]) / 2.0;
}

double throughput_mib_s(std::size_t bytes, double milliseconds) {
    if (milliseconds == 0.0) {
        return 0.0;
    }
    return (static_cast<double>(bytes) / (1024.0 * 1024.0)) /
           (milliseconds / 1000.0);
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("could not open " + path);
    }
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

BenchmarkResult benchmark_file(const std::string& path, int runs) {
    const auto input = read_file(path);

    // Warm up code and allocator state before collecting samples.
    auto compressed = compress(input);
    auto restored = decompress(compressed, input.size());
    if (restored != input) {
        throw std::runtime_error("round-trip verification failed for " + path);
    }

    std::vector<double> compression_times;
    std::vector<double> decompression_times;
    compression_times.reserve(runs);
    decompression_times.reserve(runs);

    for (int run = 0; run < runs; ++run) {
        const auto start = Clock::now();
        compressed = compress(input);
        const auto end = Clock::now();
        compression_times.push_back(
            std::chrono::duration<double, std::milli>(end - start).count());
    }

    for (int run = 0; run < runs; ++run) {
        const auto start = Clock::now();
        restored = decompress(compressed, input.size());
        const auto end = Clock::now();
        decompression_times.push_back(
            std::chrono::duration<double, std::milli>(end - start).count());

        if (restored != input) {
            throw std::runtime_error("round-trip verification failed for " +
                                     path);
        }
    }

    const double compression_ms = median_ms(compression_times);
    const double decompression_ms = median_ms(decompression_times);
    const std::size_t output_size = kFileOverhead + compressed.bits.size();
    const double ratio =
        input.empty()
            ? 0.0
            : static_cast<double>(output_size) / static_cast<double>(input.size());
    const double reduction = input.empty() ? 0.0 : (1.0 - ratio) * 100.0;
    const double factor = output_size == 0
                              ? 0.0
                              : static_cast<double>(input.size()) /
                                    static_cast<double>(output_size);

    std::cout << "\n"
              << path << "\n"
              << "  Runs:                 " << runs << " (+ 1 warmup)\n"
              << "  Input size:           " << input.size() << " bytes\n"
              << "  Compressed file size: " << output_size << " bytes\n"
              << std::fixed << std::setprecision(3)
              << "  Compression ratio:    " << ratio << "\n"
              << "  Compressed to:         " << ratio * 100.0 << "%\n"
              << "  Compression factor:   " << factor << "x\n"
              << "  Space reduction:      " << reduction << "%\n"
              << "  Compression median:   " << compression_ms << " ms ("
              << throughput_mib_s(input.size(), compression_ms) << " MiB/s)\n"
              << "  Decompression median: " << decompression_ms << " ms ("
              << throughput_mib_s(input.size(), decompression_ms)
              << " MiB/s)\n"
              << "  Round-trip verified:  yes\n";

    return {input.size(), output_size, compression_ms, decompression_ms};
}

void print_aggregate(const std::vector<BenchmarkResult>& results) {
    std::size_t input_size = 0;
    std::size_t output_size = 0;
    double compression_ms = 0.0;
    double decompression_ms = 0.0;

    for (const auto& result : results) {
        input_size += result.input_size;
        output_size += result.output_size;
        compression_ms += result.compression_ms;
        decompression_ms += result.decompression_ms;
    }

    const double ratio =
        input_size == 0
            ? 0.0
            : static_cast<double>(output_size) / static_cast<double>(input_size);
    const double factor = output_size == 0
                              ? 0.0
                              : static_cast<double>(input_size) /
                                    static_cast<double>(output_size);

    std::cout << "\nAggregate (" << results.size() << " files)\n"
              << "  Total input size:      " << input_size << " bytes\n"
              << "  Total output size:     " << output_size << " bytes\n"
              << std::fixed << std::setprecision(3)
              << "  Compression ratio:    " << ratio << "\n"
              << "  Compressed to:         " << ratio * 100.0 << "%\n"
              << "  Compression factor:   " << factor << "x\n"
              << "  Space reduction:      " << (1.0 - ratio) * 100.0 << "%\n"
              << "  Compression throughput:   "
              << throughput_mib_s(input_size, compression_ms) << " MiB/s\n"
              << "  Decompression throughput: "
              << throughput_mib_s(input_size, decompression_ms) << " MiB/s\n"
              << "  All round trips verified: yes\n";
}

}  // namespace

int main(int argc, char** argv) {
    int runs = 5;
    std::vector<std::string> paths;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--runs") {
            if (++i >= argc) {
                std::cerr << "--runs requires a positive integer\n";
                return 2;
            }
            try {
                runs = std::stoi(argv[i]);
            } catch (const std::exception&) {
                std::cerr << "--runs requires a positive integer\n";
                return 2;
            }
            if (runs <= 0) {
                std::cerr << "--runs requires a positive integer\n";
                return 2;
            }
        } else {
            paths.push_back(argument);
        }
    }

    if (paths.empty()) {
        paths.push_back("big_demo.txt");
    }

    std::cout << "Mini Compressor benchmark (in-memory codec timing)\n";
    try {
        std::vector<BenchmarkResult> results;
        results.reserve(paths.size());
        for (const auto& path : paths) {
            results.push_back(benchmark_file(path, runs));
        }
        if (results.size() > 1) {
            print_aggregate(results);
        }
    } catch (const std::exception& error) {
        std::cerr << "Benchmark error: " << error.what() << "\n";
        return 1;
    }
}
