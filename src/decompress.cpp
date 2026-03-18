#include <bits/stdc++.h>
#include "../include/bitstream.hpp"
#include "../include/lz77.hpp"
#include "../include/huffman.hpp"
using namespace std;

static uint32_t readU32(ifstream &in){
    uint32_t v;
    in.read((char*)&v,4);
    return v;
}

static uint32_t fnv1a(const vector<uint8_t> &data){
    uint32_t h = 2166136261u;

    for(uint8_t b : data){
        h ^= b;
        h *= 16777619u;
    }

    return h;
}

static bool endsWith(const string &s,const string &suf){
    if(s.size() >= suf.size()){
        if(s.compare(s.size() - suf.size(),suf.size(),suf) == 0){
            return true;
        }
    }
    return false;
}

int main(){

    string inName,outName;

    cout << "Input file name: ";
    cin >> inName;

    cout << "Output file name: ";
    cin >> outName;

    if(!endsWith(inName,".lzhf")){
        cerr << "Input must have .lzhf extension.\n";
        return 1;
    }

    ifstream in(inName,ios::binary);
    if(!in){
        cerr << "Failed to open input.\n";
        return 1;
    }

    char magic[4];
    in.read(magic,4);

    if(strncmp(magic,"LZHF",4) != 0){
        cerr << "Invalid file.\n";
        return 1;
    }

    uint32_t origSize = readU32(in);
    uint32_t packedSize = readU32(in);
    uint32_t checksum = readU32(in);

    vector<uint32_t> freq(256);
    for(int i=0;i<256;i++){
        freq[i] = readU32(in);
    }

    vector<uint8_t> bits((istreambuf_iterator<char>(in)),istreambuf_iterator<char>());

    auto packed = huff_decode(bits,freq,packedSize);
    auto outData = lz77_decompress_packed(packed,origSize);

    ofstream out(outName,ios::binary);
    if(!out){
        cerr << "Failed to open output.\n";
        return 1;
    }

    out.write((char*)outData.data(),outData.size());

    ifstream in2(inName,ios::binary | ios::ate);
    ifstream out2(outName,ios::binary | ios::ate);

    size_t comp = (size_t)in2.tellg();
    size_t dec = (size_t)out2.tellg();

    cout << "Compressed size: " << comp << " bytes\n";
    cout << "Decompressed size: " << dec << " bytes\n";

    if(dec == origSize){
        cout << "Matches original size: yes\n";
    } else {
        cout << "Matches original size: no\n";
    }

    if(fnv1a(outData) == checksum){
        cout << "Hash match: yes\n";
    } else {
        cout << "Hash match: no\n";
    }

    return 0;
}