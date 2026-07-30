#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "../include/bitstream.hpp"
#include "../include/lz77.hpp"
#include "../include/huffman.hpp"
using namespace std;

// write/read 32-bit values (used in file header)
static void writeU32(ofstream &out,uint32_t v){
    out.write((char*)&v,4);
}

static uint32_t readU32(ifstream &in){
    uint32_t v;
    in.read((char*)&v,4);
    return v;
}

// checksum to verify data integrity
static uint32_t fnv1a(const vector<uint8_t> &data){
    uint32_t h = 2166136261u;
    for(uint8_t b : data){
        h ^= b;
        h *= 16777619u;
    }
    return h;
}

// simple extension check
static bool endsWith(const string &s,const string &suf){
    if(s.size() >= suf.size()){
        if(s.compare(s.size() - suf.size(),suf.size(),suf) == 0){
            return true;
        }
    }
    return false;
}

int main(){

    cout << "1. Compress\n2. Decompress\n";
    int choice = 0;
    cin >> choice;

    string inName,outName;
    cout << "Input file name: ";
    cin >> inName;

    if(choice == 1){

        outName = inName + ".lzhf";

        ifstream in(inName,ios::binary);
        if(!in){
            cerr << "Failed to open input.\n";
            return 1;
        }

        // read full file
        vector<uint8_t> data((istreambuf_iterator<char>(in)),istreambuf_iterator<char>());

        // LZ77 → Huffman pipeline
        auto packed = lz77_compress_packed(data);
        auto freq = huff_build_freq(packed); 
        auto bits = huff_encode(packed,freq);
        uint32_t checksum = fnv1a(data);

        ofstream out(outName,ios::binary);
        if(!out){
            cerr << "Failed to open output.\n";
            return 1;
        }

        // file format: header + metadata + freq + compressed bits
        out.write("LZHF",4);
        writeU32(out,(uint32_t)data.size());
        writeU32(out,(uint32_t)packed.size());
        writeU32(out,checksum);

        for(int i=0;i<256;i++){
            writeU32(out,freq[i]);
        }

        out.write((char*)bits.data(),bits.size());
        out.close();

        // size comparison
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
    }

    else if(choice == 2){

        if(!endsWith(inName,".lzhf")){
            cerr << "Input must have .lzhf extension.\n";
            return 1;
        }

        cout << "Output file name: ";
        cin >> outName;

        ifstream in(inName,ios::binary);
        if(!in){
            cerr << "Failed to open input.\n";
            return 1;
        }

        char magic[4];
        in.read(magic,4);

        // validate file format
        if(strncmp(magic,"LZHF",4) != 0){
            cerr << "Invalid file.\n";
            return 1;
        }

        // read metadata
        uint32_t origSize = readU32(in);
        uint32_t packedSize = readU32(in);
        uint32_t checksum = readU32(in);

        vector<uint32_t> freq(256);
        for(int i=0;i<256;i++){
            freq[i] = readU32(in);
        }

        // remaining data = compressed bitstream
        vector<uint8_t> bits((istreambuf_iterator<char>(in)),istreambuf_iterator<char>());

        // Huffman → LZ77 decode pipeline
        auto packed = huff_decode(bits,freq,packedSize);
        auto outData = lz77_decompress_packed(packed,origSize);

        ofstream out(outName,ios::binary);
        if(!out){
            cerr << "Failed to open output.\n";
            return 1;
        }

        out.write((char*)outData.data(),outData.size());
        out.close();

        // verification
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
    }

    else{
        cout << "Invalid choice.\n";
    }

    return 0;
}
