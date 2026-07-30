#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../include/huffman.hpp"
#include "../include/lz77.hpp"

namespace {

bool round_trips(const std::vector<std::uint8_t>& input) {
    const auto packed = lz77_compress_packed(input);
    const auto frequencies = huff_build_freq(packed);
    const auto bits = huff_encode(packed, frequencies);
    const auto decoded_packed =
        huff_decode(bits, frequencies, packed.size());
    const auto output = lz77_decompress_packed(decoded_packed, input.size());
    return output == input;
}

}  // namespace

int main() {
    std::vector<std::vector<std::uint8_t>> cases = {
        {},
        {'A'},
        {'A', 'A'},
        {'A', 'A', 'A'},
        std::vector<std::uint8_t>(10'000, 'A'),
    };

    const std::string sentence =
        "the quick brown fox jumps over the lazy dog\n";
    cases.emplace_back(sentence.begin(), sentence.end());

    std::vector<std::uint8_t> every_byte;
    for (int repeat = 0; repeat < 8; ++repeat) {
        for (int value = 0; value < 256; ++value) {
            every_byte.push_back(static_cast<std::uint8_t>(value));
        }
    }
    cases.push_back(every_byte);

    std::mt19937 generator(0xC0FFEE);
    std::uniform_int_distribution<int> byte_distribution(0, 255);
    for (std::size_t size : {7u, 31u, 257u, 4096u}) {
        std::vector<std::uint8_t> random_bytes(size);
        for (auto& byte : random_bytes) {
            byte = static_cast<std::uint8_t>(byte_distribution(generator));
        }
        cases.push_back(std::move(random_bytes));
    }

    for (std::size_t index = 0; index < cases.size(); ++index) {
        if (!round_trips(cases[index])) {
            std::cerr << "round-trip test " << index << " failed\n";
            return 1;
        }
    }

    std::cout << "All " << cases.size()
              << " deterministic round-trip tests passed.\n";
}
