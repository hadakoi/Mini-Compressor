# LZ77 + Huffman Mini Compressor

A lossless compression tool combining:

- LZ77 → removes repeated patterns
- Huffman Coding → compresses symbols based on frequency

**Why LZ77 before Huffman?**

Huffman alone cannot detect patterns,
only frequency leading to a worse compression


### Complexity

``LZ77 → O(n × window)`` <br>
``Huffman → O(n log n)``

---

## Build

```bash
g++ -std=c++17 -O2 -Iinclude -o lztool src/main.cpp
````

---

## Run

```bash
./lztool
```

---

## Pipeline

```
Input
  ↓
LZ77
  ↓
Packed byte stream
  ↓
Huffman
  ↓
Compressed bitstream
  ↓
.lzhf file
```

---

## LZ77 Design

* Window size: 4096 bytes
* Max match length: 18
* Min match length: 3

### Encoding

```
Literal:
0 + 8 bits

Match:
1 + 12 bits (offset) + 4 bits (length)
```

### Notes

* 12-bit offset → supports window up to 4096
* Length stored as (len - 3)
* Actual length range: 3–18

---

## Huffman Design

* Built from LZ77 output (not original data)
* Min-heap used to construct tree
* Left = 0, Right = 1

### Properties

* Prefix-free codes
* No separators needed
* Frequent symbols → shorter codes

---

## File Format (.lzhf)

```
[4 bytes]  "LZHF"
[4 bytes]  Original size
[4 bytes]  LZ77 size
[4 bytes]  Checksum

[1024 bytes] Frequency table (256 × uint32)

[variable] Bitstream
```

---

## Compression

```
Read input file
  ↓
LZ77 compress → packed stream
  ↓
Build frequency table
  ↓
Huffman encode → bitstream
  ↓
Write file (header + freq + bits)
```

### Notes

* Huffman operates on LZ77 output
* Bit-level packing used to store variable-length codes
* Frequency table is stored to rebuild tree during decoding

---

## Decompression

```
Read file
  ↓
Parse header (sizes, checksum, frequency table)
  ↓
Rebuild Huffman tree
  ↓
Decode bitstream → LZ77 packed stream
  ↓
LZ77 decode → original data
```

### Notes

* Huffman decoding reads bits one at a time:

  * 0 → left
  * 1 → right

* Symbol produced at leaf node

* Prefix-free property removes need for delimiters

* Decoding stops using:

  * packedSize (Huffman output size)
  * origSize (final output size)

* Bitstream may contain padding bits

* LZ77 decoding:

  * literals → direct copy
  * matches → copy from previous output

**Why prefix-free codes matter?**

Huffman codes are prefix free, meaning no code is a prefix of another.  
This allows decoding without explicit delimiters the tree structure determines symbol boundaries.

---

## Integrity Check

* Uses FNV-1a hash
* Stored during compression
* Verified after decompression
* Ensures exact reconstruction

---

## Limitations

* 1024-byte frequency table overhead
* Brute-force LZ77 matching (O(n × window))
* Entire file loaded into memory
