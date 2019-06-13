# BitVector

Basic binary array structure.
Supporting basic operations like `std::vector<>`.

## Constructions
- `BitVector(void)`
  - Construct empty bit-vector.
- `BitVector(size_t size)`
  - Like `std::vector<bool>(size)`
- `BitVector(size_t size, bool initial_bit)`

## Central Operations
- `bool operator[](size_t i)`
  - return $i$th bit $\in \{0,1\}$.

## Examples
```c++
#include <iostream>
#include "sim_ds/BitVector.hpp"

int main() {
  sim_ds::BitVector bv;
  bv.resize(4);
  bv[0] = false;
  bv[1] = true;
  bv[2] = true;
  bv[3] = false;

  for (bool bit : bv)
    std::cout << bit << std::endl;

  // 0
  // 1
  // 1
  // 0

  return 0;
}
```
