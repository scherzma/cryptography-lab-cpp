# Cryptography Lab (C++)

Experimental cryptography implementations and attack exercises in C++23, including AES-based encryption, XEX, GCM, finite-field polynomial arithmetic, a padding-oracle attack, and GCM nonce-reuse analysis.

## Build and run

Requires CMake 3.10+, a C++23 compiler, pkg-config, and an x86 CPU with AES-NI and carry-less multiplication support. The build uses native CPU optimizations.

```sh
cmake -S . -B building
cmake --build building -j4
./building/KryptoCpp sample.json
```

The executable reads JSON test cases and writes JSON results. `testcases/` contains additional fixtures, including fixtures adapted from an external contribution. Some exercises require the local padding-oracle server in `src/server.py` (Python with the `cryptography` package).

This is educational, experimental code; it is not a production cryptography library. Keys in the example JSON files and local oracle server are exercise data.

Some GCM analysis code was adapted from an external contribution; the source retains this acknowledgment.

## Known limitations

The project builds with GCC 15, but running `testcases/gfpoly_add_input.json` currently aborts on a standard-library vector bounds assertion. The included fixtures are historical exercise data and are not a passing regression suite.
