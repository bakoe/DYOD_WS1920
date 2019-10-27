#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "value_segment.hpp"

#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

Table::Table(uint32_t chunk_size) {
  _max_chunk_size = chunk_size;
  const auto first_chunk = std::make_shared<Chunk>();
  _chunks.push_back(first_chunk);
}

void Table::add_column(const std::string& name, const std::string& type) {
  for(auto const& chunk: _chunks) {
    chunk->add_segment(make_shared_by_data_type<BaseSegment,ValueSegment>(type));
  }
  _column_names.push_back(name);
  _column_types.push_back(type);
}

void Table::append(std::vector<AllTypeVariant> values) {
  auto last_chunk = _chunks.back();
  if (last_chunk->size() < _max_chunk_size) {
    last_chunk->append(values);
    return;
  }

  // TODO(anyone): 
  const auto chunk = std::make_shared<Chunk>();
  for(auto const& column_type: _column_types) {
    chunk->add_segment(make_shared_by_data_type<BaseSegment,ValueSegment>(column_type));
  }
  chunk->append(values);
  _chunks.push_back(chunk);
}

uint16_t Table::column_count() const {
  if (_chunks.size() == 0) {
    return 0;
  }
  return _chunks[0]->column_count();
}

uint64_t Table::row_count() const {
  uint64_t number_of_rows = 0;
  for(auto const& chunk: _chunks) {
    number_of_rows += chunk->size();
  }
  return number_of_rows;
}

ChunkID Table::chunk_count() const {
  return (ChunkID)_chunks.size();
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  // Implementation goes here
  for (ColumnID column_name_index = (ColumnID)0; column_name_index < _column_names.size(); column_name_index++) {
    if (_column_names[column_name_index].compare(column_name) == 0) {
      return column_name_index;
    }
  }
  throw std::exception();
}

uint32_t Table::max_chunk_size() const {
  return _max_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  return _column_names;
}

const std::string& Table::column_name(ColumnID column_id) const {
  return _column_names[column_id];
}

const std::string& Table::column_type(ColumnID column_id) const {
  return _column_types[column_id];
}

Chunk& Table::get_chunk(ChunkID chunk_id) {
  return *_chunks[chunk_id];
}

const Chunk& Table::get_chunk(ChunkID chunk_id) const { throw std::runtime_error("Implement Table::get_chunk"); }

}  // namespace opossum
