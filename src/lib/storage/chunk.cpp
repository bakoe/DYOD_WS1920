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

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) { _segments.push_back(segment); }

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  // TODO(anyone): Make sure the DebugAssert works correctly.
  DebugAssert((values.size() == column_count()),
              "The number of Elements added does not match the number of Segments in this Chunk.");

  for (uint16_t valueIndex = 0; valueIndex < values.size(); valueIndex++) {
    _segments[valueIndex]->append(values[valueIndex]);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const { return _segments[column_id]; }

uint16_t Chunk::column_count() const { return _segments.size(); }

uint32_t Chunk::size() const {
  if (_segments.size() <= 0) {
    return 0;
  }

  return _segments[0]->size();
}

}  // namespace opossum
