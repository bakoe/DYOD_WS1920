#include <limits>
#include <string>
#include <vector>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "../lib/storage/fixed_size_attribute_vector.hpp"

namespace opossum {

class FixedSizeAttributeVectorTest : public BaseTest {};

TEST_F(FixedSizeAttributeVectorTest, InstantiateWithUint8Size) {
  auto vector = FixedSizeAttributeVector<uint8_t>(5);
  EXPECT_EQ(vector.width(), size_t{1});
}

TEST_F(FixedSizeAttributeVectorTest, InstantiateWithUint16Size) {
  auto vector = FixedSizeAttributeVector<uint16_t>(5);
  EXPECT_EQ(vector.width(), size_t{2});
}

TEST_F(FixedSizeAttributeVectorTest, InstantiateWithUint32Size) {
  auto vector = FixedSizeAttributeVector<uint32_t>(5);
  EXPECT_EQ(vector.width(), size_t{4});
}

}  // namespace opossum
