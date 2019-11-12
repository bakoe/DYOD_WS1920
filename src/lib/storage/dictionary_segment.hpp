#pragma once

#include <type_cast.hpp>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "all_type_variant.hpp"
#include "base_attribute_vector.hpp"
#include "fixed_size_attribute_vector.hpp"
#include "resolve_type.hpp"
#include "types.hpp"

namespace opossum {

class BaseAttributeVector;
class BaseSegment;

// Even though ValueIDs do not have to use the full width of ValueID (uint32_t), this will also work for smaller ValueID
// types (uint8_t, uint16_t) since after a down-cast INVALID_VALUE_ID will look like their numeric_limit::max()
constexpr ValueID INVALID_VALUE_ID{std::numeric_limits<ValueID::base_type>::max()};

// Dictionary is a specific segment type that stores all its values in a vector
template <typename T>
class DictionarySegment : public BaseSegment {
 public:
  /**
   * Creates a Dictionary segment from a given value segment.
   */
  explicit DictionarySegment(const std::shared_ptr<BaseSegment>& base_segment) {
    // sorted_values acts as a copy of base_segment's values in a sorted way (used only for the dictionary creation)
    auto sorted_values = std::vector<T>(base_segment->size());

    // Since we haven't access to the underlying data structure of the BaseSegment base_segment, we use
    // the [] operator to manually create a copy of the base_segment's values
    for (size_t value_index = (ChunkOffset)0; value_index < base_segment->size(); value_index++) {
      sorted_values[value_index] = type_cast<T>(base_segment->operator[](value_index));
    }

    // The copied values are sorted for efficient generation of the dictionary
    std::sort(sorted_values.begin(), sorted_values.end());

    _dictionary = std::make_shared<std::vector<T>>();

    // Fill the dictionary by walking through base_segment's sorted values and adding a new entry to the dictionary
    // whenever the current value has changed (i.e. a new and thus unknown - because of the sorting - value has been
    // found).
    // As a bonus, the dictionary is then already sorted as well.
    bool found_first_value = false;
    T previous_value;
    for (auto value_index = (ChunkOffset)0; value_index < sorted_values.size(); value_index++) {
      auto value = sorted_values[value_index];
      if (value != previous_value || !found_first_value) {
        found_first_value = true;
        _dictionary->push_back(value);
        previous_value = value;
      }
    }

    // Determine the size of the to-be-created FixedSizeAttributeVector based on the amount of distinct values
    // (i.e. the length/size of the dictionary)
    if (_dictionary->size() < std::numeric_limits<uint8_t>::max()) {
      _attribute_vector = std::make_shared<FixedSizeAttributeVector<uint8_t>>(base_segment->size());
    } else if (_dictionary->size() < std::numeric_limits<uint16_t>::max()) {
      _attribute_vector = std::make_shared<FixedSizeAttributeVector<uint16_t>>(base_segment->size());
    } else {
      _attribute_vector = std::make_shared<FixedSizeAttributeVector<uint32_t>>(base_segment->size());
    }

    // Build up the attribute vector by looking up the index of the value in the dictionary for each value.
    // Because the dictionary is already sorted, we can use the efficient std::lower_bound method for performing a
    // binary search for the dictionary index.
    for (size_t value_index = (ChunkOffset)0; value_index < base_segment->size(); value_index++) {
      uint32_t dictionary_index =
          std::distance(_dictionary->begin(), std::lower_bound(_dictionary->begin(), _dictionary->end(),
                                                               type_cast<T>(base_segment->operator[](value_index))));
      _attribute_vector->set(value_index, ValueID{dictionary_index});
    }
  }

  // SEMINAR INFORMATION: Since most of these methods depend on the template parameter, you will have to implement
  // the DictionarySegment in this file. Replace the method signatures with actual implementations.

  // return the value at a certain position. If you want to write efficient operators, back off!
  AllTypeVariant operator[](const ChunkOffset chunk_offset) const override {
    return _dictionary->at(_attribute_vector->get(chunk_offset));
  }

  // return the value at a certain position.
  T get(const size_t chunk_offset) const { return _dictionary->at(_attribute_vector->get(chunk_offset)); }

  // dictionary segments are immutable
  void append(const AllTypeVariant&) override { throw std::exception(); }

  // returns an underlying dictionary
  std::shared_ptr<const std::vector<T>> dictionary() { return _dictionary; }

  // returns an underlying data structure
  std::shared_ptr<const BaseAttributeVector> attribute_vector() { return _attribute_vector; }

  // return the value represented by a given ValueID
  const T& value_by_value_id(ValueID value_id) { return _dictionary->at(value_id); }

  // returns the first value ID that refers to a value >= the search value
  // returns INVALID_VALUE_ID if all values are smaller than the search value
  ValueID lower_bound(T value) const {
    for (size_t value_index = 0; value_index < _attribute_vector->size(); value_index++) {
      if (get(value_index) >= value) {
        return ValueID{value_index};
      }
    }
    return INVALID_VALUE_ID;
  }

  // same as lower_bound(T), but accepts an AllTypeVariant
  ValueID lower_bound(const AllTypeVariant& value) const { return lower_bound(type_cast<T>(value)); }

  // returns the first value ID that refers to a value > the search value
  // returns INVALID_VALUE_ID if all values are smaller than or equal to the search value
  ValueID upper_bound(T value) const {
    for (size_t value_index = 0; value_index < _attribute_vector->size(); value_index++) {
      if (get(value_index) > value) {
        return ValueID{value_index};
      }
    }
    return INVALID_VALUE_ID;
  }

  // same as upper_bound(T), but accepts an AllTypeVariant
  ValueID upper_bound(const AllTypeVariant& value) const { return upper_bound(type_cast<T>(value)); }

  // return the number of unique_values (dictionary entries)
  size_t unique_values_count() const { return _dictionary->size(); }

  // return the number of entries
  size_t size() const override { return _attribute_vector->size(); }

  // returns the calculated memory usage
  size_t estimate_memory_usage() const final {
    auto dictionary_size = sizeof(T) * _dictionary->size();
    auto attribute_vector_size = _attribute_vector->width() * _attribute_vector->size();
    return dictionary_size + attribute_vector_size;
  }

 protected:
  std::shared_ptr<std::vector<T>> _dictionary;
  std::shared_ptr<BaseAttributeVector> _attribute_vector;
};

}  // namespace opossum
