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

static vector<uint8_t> bw_get_data(BitWriter &bw){ // mainly when one wrote partial bits for a malformed byte 

    if(bw.bitpos){
        bw.cur <<= (8 - bw.bitpos); // padd remaining bits with 0.
        bw.data.push_back(bw.cur); // storing it
        bw.cur = 0; // resetting values
        bw.bitpos = 0; // resetting values
    }

    return bw.data;
}

static void br_load(BitReader &br,const vector<uint8_t> &d){
    br.data = &d;
    br.rbyte = 0;
    br.rbit = 0;
} // Initalizing Reading at the 0th byte and 0th bit.

static int br_read_bit(BitReader &br){

    if(!br.data){
        return 0;
    } // check if data is present

    if(br.rbyte >= br.data->size()){
        return 0;
    } // check if the rbyte has not exceeded the size of the actual data

    int b = ((*br.data)[br.rbyte] >> (7 - br.rbit)) & 1; // this extracts 1 bit. (bits read left -> right)

    br.rbit++;

    if(br.rbit == 8){
        br.rbit = 0;
        br.rbyte++;
    } // when we have checked 8 bits (1 byte), reset counter and check next byte.

    return b;
}

static uint32_t br_read_bits(BitReader &br,int n){
    uint32_t v = 0;

    for(int i = 0;i < n;i++){
        v = (v << 1) | (uint32_t)br_read_bit(br); // reconstructs entire numbers from bits. eg, read(1) - 1, read(0) - 10, read(1) - 101
    }

    return v;
}