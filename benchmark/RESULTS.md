# Benchmark Results

## Canterbury Corpus

The [Canterbury Corpus](https://corpus.canterbury.ac.nz/descriptions/) is a
fixed, public collection for evaluating lossless compression. Its 11 files
include prose, technical writing, HTML, C and Lisp source, an Excel spreadsheet,
fax data, a SPARC executable, and a Unix manual page.

Test environment:

- Apple M5 (10-core), 16 GB memory
- Homebrew Clang 22.1.8
- C++17 release build with `-O3 -DNDEBUG`
- Five measured runs per file after one warmup
- In-memory codec timing; file I/O excluded
- Complete 1,040-byte `.lzhf` header included once per compressed file

Run:

```bash
make bench-canterbury
```

Results recorded on July 30, 2026:

| File | Type | Input bytes | Output bytes | Reduction |
|---|---|---:|---:|---:|
| alice29.txt | English prose | 152,089 | 73,085 | 51.946% |
| asyoulik.txt | Shakespeare play | 125,179 | 65,354 | 47.792% |
| cp.html | HTML source | 24,603 | 11,774 | 52.144% |
| fields.c | C source | 11,150 | 4,783 | 57.103% |
| grammar.lsp | Lisp source | 3,721 | 2,492 | 33.029% |
| kennedy.xls | Excel spreadsheet | 1,029,744 | 254,408 | 75.294% |
| lcet10.txt | Technical writing | 426,754 | 197,833 | 53.642% |
| plrabn12.txt | Poetry | 481,861 | 259,932 | 46.057% |
| ptt5 | Fax data | 513,216 | 87,598 | 82.932% |
| sum | SPARC executable | 38,240 | 18,077 | 52.728% |
| xargs.1 | Unix manual page | 4,227 | 3,071 | 27.348% |
| **Aggregate** | **11 mixed files** | **2,810,784** | **978,407** | **65.191%** |

Aggregate results:

- Compression factor: 2.873x
- Compression throughput: 1.323 MiB/s
- Decompression throughput: 142.402 MiB/s
- All 11 byte-for-byte round trips verified

Performance results vary with hardware and system load. The slow compression
throughput reflects the current brute-force LZ77 match search and identifies a
clear optimization target.
