#pragma once

// the linter wants this to be above everything else
#include <types.hpp>
#include <vector>
#include "base_attribute_vector.hpp"

namespace opossum {

// Attribute vector with a fixed size that stores values of the given template type
template <typename T>
class FixedSizeAttributeVector : public BaseAttributeVector {
 public:
  /**
   * Creates a Fixed size attribute vector with the given fixed size.
   *
   * @param size is a size_t indicating the fixed size that the attribute vector should reserve/use
   */
  explicit FixedSizeAttributeVector(const size_t size);

  // returns the value id at a given position
  ValueID get(const size_t i) const;

  // sets the value id at a given position
  void set(const size_t i, const ValueID value_id);

  // returns the number of values
  size_t size() const;

  // returns the width of biggest value id in bytes
  AttributeVectorWidth width() const;

 protected:
  std::vector<T> _values;
};

}  // namespace opossum
