# SimpleDataStructure
C++17 interface (header only) library packing some of succinct data structures.
This library supporting cmake build environment.

**We welcome to your contacts and contributions.**

## Usage
### As CMake library examples
1. Add your git repository as submodule like follows:
```bash
git submodule add https://github.com/MatsuTaku/SimpleDataStructure.git
```
2. In your `CMakeLists.txt` file, type like follows:
```CMake
...
add_subdirectory(SimpleDataStructure)
target_link_libraries(your_target [PUBLIC|PRIVATE|INTERFACE] sim_ds)
...
```

## Documents

### Succinct data structures
- [BitVector](documents/BitVector.md)
- [SuccinctBitVector](documents/SuccinctBitVector.md)
  - Extended binary array supporting rank/select operation.
- WaveletTree
- DacVector
  - Compressed array representation that stores each value almost as fit-bits size.
- Heap
- SuffixArray
- FactorOracle
- Samc

### Custom data structure
- MultiBitVector
- EmptyLinkedVector

### Utilities
- bit_util
- graph_util
- PatternMatching
- sort
- graph_util
