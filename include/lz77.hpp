#pragma once
#include <bits/stdc++.h>
#include "bitstream.hpp"
using namespace std;

static vector<uint8_t> lz77_compress_packed(const vector<uint8_t> &in){

    const int window = 4096;
    const int maxLen = 18;
    const int minLen = 3;

    BitWriter bw;
    size_t i = 0;

    while(i < in.size()){

        int bestLen = 0;
        int bestOff = 0;

        int start = max(0,(int)i - window);

        for(int j = (int)i - 1;j >= start;j--){

            int len = 0;

            while(len < maxLen && i + len < in.size() && in[j + len] == in[i + len]){
                len++;
            }

            if(len > bestLen){
                bestLen = len;
                bestOff = (int)(i - j);
            }

            if(bestLen == maxLen){
                break;
            }
        }

        if(bestLen >= minLen){

            bw_write_bit(bw,1);

            bw_write_bits(bw,(uint32_t)(bestOff - 1),12);
            bw_write_bits(bw,(uint32_t)(bestLen - minLen),4);

            i += bestLen;
        }
        else{

            bw_write_bit(bw,0);

            bw_write_bits(bw,(uint32_t)in[i],8);

            i++;
        }
    }

    return bw_get_data(bw);
}

static vector<uint8_t> lz77_decompress_packed(const vector<uint8_t> &packed,size_t expectedSize){

    const int minLen = 3;

    vector<uint8_t> out;
    out.reserve(expectedSize);

    BitReader br;
    br_load(br,packed);

    while(out.size() < expectedSize){

        int flag = br_read_bit(br);

        if(flag == 0){

            uint8_t b = (uint8_t)br_read_bits(br,8);

            out.push_back(b);
        }
        else{

            uint32_t off = br_read_bits(br,12) + 1;
            uint32_t len = br_read_bits(br,4) + (uint32_t)minLen;

            if(off > out.size()){
                break;
            }

            size_t start = out.size() - off;

            for(uint32_t k = 0;k < len;k++){
                out.push_back(out[start + k]);
            }
        }
    }

    return out;
}