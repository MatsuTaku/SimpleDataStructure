# SuccinctBitVector

Extended binary array structure supporting *rank/select* operations.
Supporting basic operation like `std::vector<>`.

- $B$: $n$th bits array. $(B = \{0,1\}^n)$
- $\texttt{rank}(i)$: counts $1$th in $B[0, \dots, i-1)$
- $\texttt{select}(i)$: index of $i$th 1

*rank* and *select* can calculate in $O(1)$ times.

**Note**
This class is constant (immutable) structure.
You must set bits using [BitVector](BitVector.md) or etc before construction.

## Template parameters
```c++
template <bool UseSelect>
class SuccinctBitVector;
```

- `UseSelect`
  - Set `true` if you want to use *select* operations.

## Constructions
- `SuccinctBitVector(sim_ds::BitVector&& bv)`
  - Same as `std::vector<bool>(size)`

## Central Operations
- `bool operator[](size_t i)`
  - return $i$th bit $\in \{0,1\}$.
- `size_t rank(size_t i)`
  - return counts $1$th in $B[0, \dots, i-1)$
- `size_t select(size_t i)`
  - return index of $i$th 1

## Examples
```c++
#include <iostream>
#include <vector>
#include "sim_ds/SuccinctBitVector.hpp"

int main() {
  sim_ds::BitVector bv(std::vector<bool>{0,1,1,0,1,0,1,1});
  sim_ds::SuccinctBitVector<true> sbv(std::move(bv));

  // rank operation
  for (int i = 0; i < 8; i++)
    if (sbv[i])
      std::cout << i << ':' << sbv.rank(i) << std::endl;

  // 1:0
  // 2:1
  // 4:2
  // 6:3
  // 7:4

  // select operation
  for (int i = 0; i < 5; i++)
    std::cout << i << ':' << sbv.select(i) << std::endl;

  // 0:1
  // 1:2
  // 2:4
  // 3:6
  // 4:7

  return 0;
}
```
