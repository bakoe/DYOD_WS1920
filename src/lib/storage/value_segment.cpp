#include "value_segment.hpp"

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "type_cast.hpp"
#include "utils/assert.hpp"
#include "utils/performance_warning.hpp"

namespace opossum {

// TODO Check if this implementation is actually necessary
template <typename T>
ValueSegment<T>::ValueSegment() {}

template <typename T>
AllTypeVariant ValueSegment<T>::operator[](const ChunkOffset chunk_offset) const {
  PerformanceWarning("operator[] used");
  return _values[chunk_offset];
}

template <typename T>
void ValueSegment<T>::append(const AllTypeVariant& val) {
  _values.push_back(type_cast<T>(val));
}

template <typename T>
size_t ValueSegment<T>::size() const {
  return _values.size();
}

template <typename T>
const std::vector<T> &ValueSegment<T>::values() const {
  return _values;
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(ValueSegment);

}  // namespace opossum
