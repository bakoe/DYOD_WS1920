#include <memory>

#include "reference_segment.hpp"

namespace opossum {

ReferenceSegment::ReferenceSegment(const std::shared_ptr<const Table> referenced_table,
                                   const ColumnID referenced_column_id, const std::shared_ptr<const PosList> pos)
    : _referenced_table(referenced_table), _referenced_column_id(referenced_column_id), _pos(pos) {}

AllTypeVariant ReferenceSegment::operator[](const ChunkOffset chunk_offset) const {
  RowID position = _pos->at(chunk_offset);
  const auto& chunk = _referenced_table->get_chunk(position.chunk_id);
  const auto& segment = chunk.get_segment(_referenced_column_id);
  return (*segment)[position.chunk_offset];
}

size_t ReferenceSegment::size() const { return _pos->size(); }

const std::shared_ptr<const PosList> ReferenceSegment::pos_list() const { return _pos; }

const std::shared_ptr<const Table> ReferenceSegment::referenced_table() const { return _referenced_table; }

ColumnID ReferenceSegment::referenced_column_id() const { return _referenced_column_id; }

size_t ReferenceSegment::estimate_memory_usage() const {
  return sizeof(std::shared_ptr<PosList>) + sizeof(ColumnID) + sizeof(std::shared_ptr<Table>);
}

}  // namespace opossum
