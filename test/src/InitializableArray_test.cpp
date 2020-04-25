#include "gtest/gtest.h"
#include "sim_ds/InitializableArray.hpp"

namespace {
constexpr int SIZE = 0x1000;
constexpr int SAMPLE_SIZE = 0x100;
constexpr int LOOP = 0x1000;
}

TEST(InitializableArrayTest, basic) {
    sim_ds::InitializableArray<int> arr(SIZE);
    for (int i = 0; i < LOOP; i++) {
        int init_v = random()%SIZE;
        arr.fill(init_v); // O(1)
        std::map<int, int> queries;
        for (int j = 0; j < SAMPLE_SIZE; j++) {
            int id = random()%SIZE;
            int v = random()%SIZE;
            queries[id] = v;
            arr.set(id, v);
        }
        for (int j = 0; j < SIZE; j++) {
            EXPECT_EQ(arr.get(j), queries.count(j) ? queries[j] : init_v);
        }
    }
}
