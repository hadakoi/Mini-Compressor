#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "bitstream.hpp"
using namespace std;

static vector<uint8_t> lz77_compress_packed(const vector<uint8_t> &in){

    const int window = 4096; // 12 bits
    const int maxLen = 18; // 4 bits 
    const int minLen = 3; // avoid useless matches

    BitWriter bw;
    size_t i = 0; // current position in input.
    

    // parameters for searching longest match within a limited window

    while(i < in.size()){  // try every previous position in the window as a match start
        int bestLen = 0;
        int bestOff = 0;

        int start = max(0,(int)i - window); // start of search window (up to 4096 bytes back)

        for(int j = (int)i - 1;j >= start;j--){ // try matching from every prior position

            int len = 0;

            while(len < maxLen && i + len < in.size() && in[j + len] == in[i + len]){
                len++;
            } // count how long they match. 

            if(len > bestLen){ // keep the longest match found so far
                bestLen = len;
                bestOff = (int)(i - j);
            }

            if(bestLen == maxLen){ // stop early if maximum possible match is found
                break;
            }
        }

        if(bestLen >= minLen){ // Matched 

            bw_write_bit(bw,1); // write flag (Match)

            bw_write_bits(bw,(uint32_t)(bestOff - 1),12); // writing the offset
            bw_write_bits(bw,(uint32_t)(bestLen - minLen),4); // write length

            // 1 + 12 + 4 = 17 bits total
            // 1 bit for flag (literal or match)
            // 12 bits for offset (supports window up to 4096)
            // 4 bits for length (stores values 0–15, representing lengths 3–18 after adding minLen)
            i += bestLen; // we then skip the matched region
        }
        else{

            bw_write_bit(bw,0); // write flag (literal)

            bw_write_bits(bw,(uint32_t)in[i],8); // we write the entire byte

            i++; // move forward in the input
        }
    }

    return bw_get_data(bw); // finalize and return packed byte stream
}

static vector<uint8_t> lz77_decompress_packed(const vector<uint8_t> &packed,size_t expectedSize){ 

    const int minLen = 3; // must match compressor

    vector<uint8_t> out; // output buffer (reconstructed data)
    out.reserve(expectedSize); // avoid reallocations

    BitReader br;
    br_load(br,packed); // initialize bit reader

    while(out.size() < expectedSize){ // stop when original size is reached

        int flag = br_read_bit(br); // 0 = literal, 1 = match

        if(flag == 0){

            // read literal byte (8 bits)
            uint8_t b = (uint8_t)br_read_bits(br,8);

            out.push_back(b);
        }
        else{

            uint32_t off = br_read_bits(br,12) + 1; // read offset (stored as off-1 during compression)

            uint32_t len = br_read_bits(br,4) + (uint32_t)minLen; // read length (stored as len-minLen during compression)

            if(off > out.size()){
                break;
            } // invalid reference 

            size_t start = out.size() - off; // starting position to copy from

            for(uint32_t k = 0;k < len;k++){
                out.push_back(out[start + k]);
            } // copy from already decompressed output (back-reference)
        }
    }

    return out;
}
