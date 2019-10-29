#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) {
  segments.push_back(segment);
  // Implementation goes here
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  DebugAssert(values.size() == column_count(), "Size of values parameter and chunks column count must be equal!");

  auto it = values.begin();
  auto column_it = segments.begin();
  while (it != values.end()) {
    (*column_it)->append(*it);
    it++;
    column_it++;
  }

}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const {
  return segments[column_id];
}

uint16_t Chunk::column_count() const {
  // Implementation goes here
  return segments.size();
}

uint32_t Chunk::size() const {
  // Implementation goes here
  if (column_count() == 0) {
    return 0;
  } else {
    // TODO: can we rely on first column to be representing the whole chunk? Or do we have to search for the "biggest segment" maybe?
    return get_segment(ColumnID{0})->size();
  }
}

}  // namespace opossum
