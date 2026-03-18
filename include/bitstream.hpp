#pragma once
#include <bits/stdc++.h>
using namespace std;

struct BitWriter{
    vector<uint8_t> data;
    uint8_t cur = 0;
    int bitpos = 0;
};

struct BitReader{
    const vector<uint8_t> *data = nullptr;
    size_t rbyte = 0;
    int rbit = 0;
};

static void bw_write_bit(BitWriter &bw,int b){
    bw.cur = (uint8_t)((bw.cur << 1) | (b & 1));

    bw.bitpos++;

    if(bw.bitpos == 8){
        bw.data.push_back(bw.cur);
        bw.cur = 0;
        bw.bitpos = 0;
    }
}

static void bw_write_bits(BitWriter &bw,uint32_t v,int n){
    for(int i = n - 1;i >= 0;i--){
        int bit = (v >> i) & 1;
        bw_write_bit(bw,bit);
    }
}

static vector<uint8_t> bw_get_data(BitWriter &bw){

    if(bw.bitpos){
        bw.cur <<= (8 - bw.bitpos);
        bw.data.push_back(bw.cur);
        bw.cur = 0;
        bw.bitpos = 0;
    }

    return bw.data;
}

static void br_load(BitReader &br,const vector<uint8_t> &d){
    br.data = &d;
    br.rbyte = 0;
    br.rbit = 0;
}

static int br_read_bit(BitReader &br){

    if(!br.data){
        return 0;
    }

    if(br.rbyte >= br.data->size()){
        return 0;
    }

    int b = ((*br.data)[br.rbyte] >> (7 - br.rbit)) & 1;

    br.rbit++;

    if(br.rbit == 8){
        br.rbit = 0;
        br.rbyte++;
    }

    return b;
}

static uint32_t br_read_bits(BitReader &br,int n){
    uint32_t v = 0;

    for(int i = 0;i < n;i++){
        v = (v << 1) | (uint32_t)br_read_bit(br);
    }

    return v;
}