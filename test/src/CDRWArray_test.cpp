#include "gtest/gtest.h"
#include "sim_ds/CDRWArray.hpp"

using namespace sim_ds;

TEST(FitVectorTest, ConvertVector) {
    const auto size = 0x10000;

    std::vector<uint64_t> source(size);
    for (auto i = 0; i < size; i++)
        source[i] = (1U << (rand() % 18)) - 1;
    CDRWArray vector(source.begin(), source.end());
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(source[i], vector.get(i));
}

