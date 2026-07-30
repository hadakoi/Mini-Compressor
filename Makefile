CXX := /opt/homebrew/opt/llvm/bin/clang++
CXXFLAGS := -std=c++17 -O3 -DNDEBUG -Wall -Wextra -Wpedantic -Iinclude

CANTERBURY_ARCHIVE := /tmp/mini-compressor-cantrbry.tar.gz
CANTERBURY_DIR := /tmp/mini-compressor-canterbury

.PHONY: all benchmark check bench bench-canterbury clean

all: lztool benchmark codec_test

lztool: src/main.cpp include/bitstream.hpp include/lz77.hpp include/huffman.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

benchmark: benchmark/benchmark

benchmark/benchmark: benchmark/benchmark.cpp include/bitstream.hpp include/lz77.hpp include/huffman.hpp
	$(CXX) $(CXXFLAGS) -o $@ benchmark/benchmark.cpp

codec_test: tests/codec_test.cpp include/bitstream.hpp include/lz77.hpp include/huffman.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests/codec_test.cpp

check: codec_test
	./codec_test

bench: benchmark/benchmark
	./benchmark/benchmark --runs 5 big_demo.txt

bench-canterbury: benchmark/benchmark
	mkdir -p $(CANTERBURY_DIR)
	curl -L --fail https://corpus.canterbury.ac.nz/resources/cantrbry.tar.gz -o $(CANTERBURY_ARCHIVE)
	tar -xzf $(CANTERBURY_ARCHIVE) -C $(CANTERBURY_DIR)
	./benchmark/benchmark --runs 5 $(CANTERBURY_DIR)/*

clean:
	rm -f lztool benchmark/benchmark codec_test
