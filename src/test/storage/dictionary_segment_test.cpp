#include <limits>
#include <memory>
#include <string>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "resolve_type.hpp"
#include "storage/base_segment.hpp"
#include "storage/dictionary_segment.hpp"
#include "storage/value_segment.hpp"

namespace opossum {

class StorageDictionarySegmentTest : public BaseTest {
 protected:
  std::shared_ptr<ValueSegment<int>> vc_int = std::make_shared<ValueSegment<int>>();
  std::shared_ptr<ValueSegment<std::string>> vc_str = std::make_shared<ValueSegment<std::string>>();
};

TEST_F(StorageDictionarySegmentTest, CompressSegmentString) {
  vc_str->append("Bill");
  vc_str->append("Steve");
  vc_str->append("Alexander");
  vc_str->append("Steve");
  vc_str->append("Hasso");
  vc_str->append("Bill");

  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("string", vc_str);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<std::string>>(col);

  // Test attribute_vector size
  EXPECT_EQ(dict_col->size(), 6u);

  // Test dictionary size (uniqueness)
  EXPECT_EQ(dict_col->unique_values_count(), 4u);

  // Test sorting
  auto dict = dict_col->dictionary();
  EXPECT_EQ((*dict)[0], "Alexander");
  EXPECT_EQ((*dict)[1], "Bill");
  EXPECT_EQ((*dict)[2], "Hasso");
  EXPECT_EQ((*dict)[3], "Steve");
}

TEST_F(StorageDictionarySegmentTest, CompressSegmentWithMoreThanSizeofUint8DistinctValues) {
  // Append one more than std::numeric_limits<uint8_t>::max() entries to the integer value segment
  for (int i = 0; i < std::numeric_limits<uint8_t>::max() + 1; i += 1) vc_int->append(i);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  auto attribute_vector = dict_col->attribute_vector();
  EXPECT_EQ((attribute_vector->width()), sizeof(uint16_t));
}

TEST_F(StorageDictionarySegmentTest, CompressSegmentWithMoreThanSizeofUint16DistinctValues) {
  // Append one more than std::numeric_limits<uint16_t>::max() entries to the integer value segment
  for (int i = 0; i < std::numeric_limits<uint16_t>::max() + 1; i += 1) vc_int->append(i);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  auto attribute_vector = dict_col->attribute_vector();
  EXPECT_EQ((attribute_vector->width()), sizeof(uint32_t));
}

TEST_F(StorageDictionarySegmentTest, ArrayAccessOperator) {
  vc_int->append(3);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  EXPECT_EQ(type_cast<int>(dict_col->operator[](0)), 3);
}

TEST_F(StorageDictionarySegmentTest, IsImmutable) {
  vc_int->append(3);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  EXPECT_THROW(dict_col->append(5), std::exception);
}

TEST_F(StorageDictionarySegmentTest, LowerUpperBound) {
  for (int i = 0; i <= 10; i += 2) vc_int->append(i);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  EXPECT_EQ(dict_col->lower_bound(4), ValueID{2});
  EXPECT_EQ(dict_col->upper_bound(4), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(5), ValueID{3});
  EXPECT_EQ(dict_col->upper_bound(5), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(15), INVALID_VALUE_ID);
  EXPECT_EQ(dict_col->upper_bound(15), INVALID_VALUE_ID);
}

TEST_F(StorageDictionarySegmentTest, MemoryUsage) {
  for (int i = 1; i <= 5; i += 1) vc_int->append(i);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto int_dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  EXPECT_EQ(int_dictionary_segment->estimate_memory_usage(), 5 * sizeof(int) + 5 * sizeof(uint8_t));
}

TEST_F(StorageDictionarySegmentTest, GetValueByValueId) {
  for (int i = 1; i <= 5; i += 1) vc_int->append(i);
  auto col = make_shared_by_data_type<BaseSegment, DictionarySegment>("int", vc_int);
  auto int_dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

  EXPECT_EQ(int_dictionary_segment->value_by_value_id(ValueID{0}), 1);
}

}  // namespace opossum
