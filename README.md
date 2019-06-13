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

See [documents](https://MatsuTaku.github.io/SimpleDataStructure/)
