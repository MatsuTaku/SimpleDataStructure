# SimpleDataStructure
C++17 interface (header only) library packing some of succinct data structures.
This library supporting cmake build environment.

## Usage
### As CMake library examples
1. Add your git repository as submodule like follows:
```bash
git submodule add https://github.com/MatsuTaku/SimpleDataStructure.git
```
2. In your `CMakeLists.txt` file, type like follows:
```CMake
...
add_submodule(SimpleDataStructure)
target_link_libraries(your_target [PUBLIC|PRIVATE|INTERFACE] sim_ds)
...
```

## Documents

### Succinct data structures
- [BitVector](documents/BitVector.md)
- [SuccinctBitVector](documents/SuccinctBitVector.md)
  - Extended binary array supporting rank/select operation.
- [WaveletTree](documents/WaveletTree.md)
- [DacVector](documents/DacVector.md)
  - Compressed array representation that stores each value almost as fit-bits size.
- [Heap](documents/Heap.md)
- String handler
  - [SuffixArray](documents/SuffixArray.md)
  - [FactorOracle](documents/FactorOracle.md)
  - [Samc](documents/Samc.md)

### Custom data structure
- [MultiBitVector](documents/MultiBitVector.md)
- [EmptyLinkedVector](documents/EmptyLinkedVector.md)

### Utilities
- [bit_util](documents/bit_util.md)
- [graph_util](documents/graph_util.md)
- [PatternMatching](documents/PatternMatching.md)
- [sort](documents/sort.md)
- [graph_util](documents/graph_util.md)
