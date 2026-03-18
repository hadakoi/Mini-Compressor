#pragma once
#include <bits/stdc++.h>
#include "bitstream.hpp"
using namespace std;

struct HuffNode{
    int sym;
    uint32_t freq;
    HuffNode *l,*r;

    HuffNode(int s,uint32_t f){
        sym = s;
        freq = f;
        l = nullptr;
        r = nullptr;
    }

    HuffNode(HuffNode *a,HuffNode *b){
        sym = -1;
        freq = a->freq + b->freq;
        l = a;
        r = b;
    }
};

static bool huff_cmp(HuffNode* a,HuffNode* b){
    return a->freq > b->freq;
}

static void huff_build_codes(HuffNode* n,uint32_t code,int len, vector<uint32_t> &codes, vector<uint8_t> &lens){

    if(!n){
        return;
    }

    if(!n->l && !n->r){

        if(len == 0){
            len = 1;
            code = 0;
        }

        codes[n->sym] = code;
        lens[n->sym] = (uint8_t)len;
        return;
    }

    huff_build_codes(n->l,code << 1,len + 1,codes,lens);
    huff_build_codes(n->r,(code << 1) | 1,len + 1,codes,lens);
}

static void huff_free(HuffNode* n){

    if(!n){
        return;
    }

    huff_free(n->l);
    huff_free(n->r);

    delete n;
}

static vector<uint32_t> huff_build_freq(const vector<uint8_t> &data){

    vector<uint32_t> f(256,0);

    for(uint8_t b : data){
        f[b]++;
    }

    return f;
}

static vector<uint8_t> huff_encode(const vector<uint8_t> &data, const vector<uint32_t> &freq){

    priority_queue<HuffNode*,vector<HuffNode*>,decltype(&huff_cmp)> pq(huff_cmp);

    for(int i = 0;i < 256;i++){
        if(freq[i]){
            pq.push(new HuffNode(i,freq[i]));
        }
    }

    if(pq.empty()){
        return {};
    }

    while(pq.size() > 1){

        HuffNode *a = pq.top();
        pq.pop();

        HuffNode *b = pq.top();
        pq.pop();

        pq.push(new HuffNode(a,b));
    }

    HuffNode *root = pq.top();

    vector<uint32_t> codes(256,0);
    vector<uint8_t> lens(256,0);

    huff_build_codes(root,0,0,codes,lens);

    BitWriter bw;

    for(uint8_t b : data){
        bw_write_bits(bw,codes[b],lens[b]);
    }

    vector<uint8_t> out = bw_get_data(bw);

    huff_free(root);

    return out;
}

static vector<uint8_t> huff_decode(const vector<uint8_t> &bits, const vector<uint32_t> &freq, size_t outSize){

    priority_queue<HuffNode*,vector<HuffNode*>,decltype(&huff_cmp)> pq(huff_cmp);

    for(int i = 0;i < 256;i++){
        if(freq[i]){
            pq.push(new HuffNode(i,freq[i]));
        }
    }

    if(pq.empty()){
        return {};
    }

    while(pq.size() > 1){

        HuffNode *a = pq.top();
        pq.pop();

        HuffNode *b = pq.top();
        pq.pop();

        pq.push(new HuffNode(a,b));
    }

    HuffNode *root = pq.top();

    vector<uint8_t> out;
    out.reserve(outSize);

    if(!root->l && !root->r){

        out.assign(outSize,(uint8_t)root->sym);

        huff_free(root);

        return out;
    }

    BitReader br;
    br_load(br,bits);

    HuffNode *cur = root;

    while(out.size() < outSize){

        int bit = br_read_bit(br);

        if(bit){
            cur = cur->r;
        } else {
            cur = cur->l;
        }

        if(!cur->l && !cur->r){
            out.push_back((uint8_t)cur->sym);
            cur = root;
        }
    }

    huff_free(root);

    return out;
}