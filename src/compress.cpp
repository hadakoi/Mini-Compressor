#include <bits/stdc++.h>
#include "../include/bitstream.hpp"
#include "../include/lz77.hpp"
#include "../include/huffman.hpp"
using namespace std;

static void writeU32(ofstream &out,uint32_t v){
    out.write((char*)&v,4);
}

static uint32_t fnv1a(const vector<uint8_t> &data){
    uint32_t h = 2166136261u;

    for(uint8_t b : data){
        h ^= b;
        h *= 16777619u;
    }

    return h;
}

int main(){

    string inName,outName;

    cout << "Input file name: ";
    cin >> inName;

    outName = inName + ".lzhf";

    ifstream in(inName,ios::binary);
    if(!in){
        cerr << "Failed to open input.\n";
        return 1;
    }

    vector<uint8_t> data((istreambuf_iterator<char>(in)),istreambuf_iterator<char>());

    auto packed = lz77_compress_packed(data);
    auto freq = huff_build_freq(packed);
    auto bits = huff_encode(packed,freq);
    uint32_t checksum = fnv1a(data);

    ofstream out(outName,ios::binary);
    if(!out){
        cerr << "Failed to open output.\n";
        return 1;
    }

    out.write("LZHF",4);
    writeU32(out,(uint32_t)data.size());
    writeU32(out,(uint32_t)packed.size());
    writeU32(out,checksum);

    for(int i=0;i<256;i++){
        writeU32(out,freq[i]);
    }

    out.write((char*)bits.data(),bits.size());
    out.close();

    ifstream in2(inName,ios::binary | ios::ate);
    ifstream out2(outName,ios::binary | ios::ate);

    if(!out2){
        cout << "Error opening output file for size check\n";
        return 1;
    }

    size_t orig = (size_t)in2.tellg();
    size_t comp = (size_t)out2.tellg();

    cout << "Original size: " << orig << " bytes\n";
    cout << "Compressed size: " << comp << " bytes\n";

    if(orig){
        cout << "Compression ratio: " << fixed << setprecision(3) << (double)comp / orig << "\n";
        if (comp < orig) {
            cout << "Saved: " << (orig - comp) << " bytes\n";
        } else {
            cout << "Extra: " << (comp - orig) << " bytes\n";
        }
        cout << "Reduction: " << (1.0 - (double)comp / orig) * 100.0 << "%\n";
    }

    return 0;
}
