// the linter wants this to be above everything else
#include "fixed_size_attribute_vector.hpp"
#include <boost/numeric/conversion/cast.hpp>
#include <types.hpp>
#include <vector>

namespace opossum {

template <typename T>
FixedSizeAttributeVector<T>::FixedSizeAttributeVector(const size_t size) {
  _values = std::vector<T>(size);
}

// returns the value id at a given position
template <typename T>
ValueID FixedSizeAttributeVector<T>::get(const size_t i) const {
  return ValueID{_values.at(i)};
}

// sets the value id at a given position
template <typename T>
void FixedSizeAttributeVector<T>::set(const size_t i, const ValueID value_id) {
  _values[i] = value_id;
}

// returns the number of values
template <typename T>
size_t FixedSizeAttributeVector<T>::size() const {
  return _values.size();
}

// returns the width of biggest value id in bytes
template <typename T>
AttributeVectorWidth FixedSizeAttributeVector<T>::width() const {
  AttributeVectorWidth max_width = 0;
  for (uint32_t value_index = 0; value_index < _values.size(); value_index++) {
    if (sizeof(_values[value_index]) > max_width) {
      max_width = AttributeVectorWidth{sizeof(_values[value_index])};
    }
  }
  return max_width;
}

template class FixedSizeAttributeVector<uint8_t>;
template class FixedSizeAttributeVector<uint16_t>;
template class FixedSizeAttributeVector<uint32_t>;

}  // namespace opossum
