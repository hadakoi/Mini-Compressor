#pragma once
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>
#include "bitstream.hpp"
using namespace std;

/*
1. Each symbol → leaf node (with frequency)
2. Take 2 smallest nodes
3. Merge them into a parent
4. Repeat until one root remains


Frequent symbols → closer to root → shorter paths
Rare symbols → deeper → longer paths

Left -> 0
Right -> 1
^ for traversing said tree.


so that means we, Traverse tree then recursively build binary number along the path

data → frequency table → heap → build tree → root node
*/

struct HuffNode{
    int sym; // byte value
    uint32_t freq; // frequency of occurence
    HuffNode *l,*r; // left and right children

    HuffNode(int s,uint32_t f){ // Initalizing a node with a real symbol
        sym = s;
        freq = f;
        l = nullptr;
        r = nullptr;
    }

    HuffNode(HuffNode *a,HuffNode *b){ // Combining 2 nodes into 1 parent node
        sym = -1;
        freq = a->freq + b->freq;
        l = a;
        r = b;
    }
};

static bool huff_cmp(HuffNode* a,HuffNode* b){
    return a->freq > b->freq;
} // compares two nodes based on frequency as smaller frequencies in this case have a higher priority

static void huff_build_codes(HuffNode* n,uint32_t code,int len, vector<uint32_t> &codes, vector<uint8_t> &lens){
    // code = bits built so far (stored in an integer)
    // len  = number of valid bits in 'code'

    if(!n){
        return;
    } // base case for stopping

    if(!n->l && !n->r){ // leaf node (actual symbol)

        if(len == 0){
            // special case: only one symbol in the tree
            // assign a single-bit code (0)
            len = 1;
            code = 0;
        }

        // store the generated code and its length for this symbol
        codes[n->sym] = code;
        lens[n->sym] = (uint8_t)len;
        return;
    }

    // go left → append 0 to the code (shift left, add 0)
    huff_build_codes(n->l,code << 1,len + 1,codes,lens);

    // go right → append 1 to the code (shift left, add 1)
    huff_build_codes(n->r,(code << 1) | 1,len + 1,codes,lens);
}

static void huff_free(HuffNode* n){

    if(!n){
        return;
    }

    huff_free(n->l);
    huff_free(n->r);

    delete n;
} // free the memory used by the huffman tree.

static vector<uint32_t> huff_build_freq(const vector<uint8_t> &data){

    vector<uint32_t> f(256,0);

    for(uint8_t b : data){ // for each byte in the data
        f[b]++; // increase said byte's frequency
    }

    return f;
} // return the frequency list

static vector<uint8_t> huff_encode(const vector<uint8_t> &data, const vector<uint32_t> &freq){

    priority_queue<HuffNode*,vector<HuffNode*>,decltype(&huff_cmp)> pq(huff_cmp); // Priorty Queue (Min Heap) meaning huff_cmp(a,b) will make a->freq > b->freq
    // so that smallest frequency nodes come out first

    for(int i = 0;i < 256;i++){
        if(freq[i]){
            pq.push(new HuffNode(i,freq[i]));
        }
    } // This creates all the leaf nodes

    if(pq.empty()){
        return {};
    } 

    while(pq.size() > 1){

        HuffNode *a = pq.top();
        pq.pop();

        HuffNode *b = pq.top();
        pq.pop();

        pq.push(new HuffNode(a,b));
    } // This builds the huffman tree by combining the two least frequent nodes repeatedly

    HuffNode *root = pq.top(); // Gets the root

    vector<uint32_t> codes(256,0); // Binary code for Byte x
    vector<uint8_t> lens(256,0); // Number of Bits in that code

    huff_build_codes(root,0,0,codes,lens); // generate said codes.

    BitWriter bw;

    for(uint8_t b : data){
        bw_write_bits(bw,codes[b],lens[b]);
    } // written into a byte stream by looking up the code then writing said number of bits for it based on length

    vector<uint8_t> out = bw_get_data(bw); 

    huff_free(root);

    return out;
}


// Huffman codes are PREFIX-FREE, hence we can reverse a bitsequence 
static vector<uint8_t> huff_decode(const vector<uint8_t> &bits, const vector<uint32_t> &freq, size_t outSize){

    priority_queue<HuffNode*,vector<HuffNode*>,decltype(&huff_cmp)> pq(huff_cmp);

    for(int i = 0;i < 256;i++){
        if(freq[i]){
            pq.push(new HuffNode(i,freq[i]));
        }
    } // This creates all the leaf nodes

    if(pq.empty()){
        return {};
    }

    while(pq.size() > 1){

        HuffNode *a = pq.top();
        pq.pop();

        HuffNode *b = pq.top();
        pq.pop();

        pq.push(new HuffNode(a,b));
    } // constructs huffman tree where we have 1 node.

    HuffNode *root = pq.top();

    vector<uint8_t> out;
    out.reserve(outSize);

    if(!root->l && !root->r){

        out.assign(outSize,(uint8_t)root->sym);

        huff_free(root);

        return out;
    } // see if tree was empty 

    BitReader br;
    br_load(br,bits);

    HuffNode *cur = root;

    // now reading the compressed bitstream one bit at a time
    while(out.size() < outSize){

        int bit = br_read_bit(br);

        if(bit){ // traversing the tree as necessary 
            cur = cur->r;
        } else {
            cur = cur->l;
        }

        if(!cur->l && !cur->r){ // if reached a symbol node then enter said symbol and restart at root for next symbol
            out.push_back((uint8_t)cur->sym);
            cur = root;
        }
    }

    huff_free(root);

    return out;
}
