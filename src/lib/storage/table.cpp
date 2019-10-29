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
#include "chunk.hpp"

namespace opossum {

Table::Table(ChunkOffset chunk_size) : m_max_chunk_size (chunk_size){
  auto chunk = std::make_shared<Chunk>();
  chunks.push_back(chunk);
}

void Table::add_column(const std::string& name, const std::string& type) {
  m_column_names.push_back(name);
  m_column_types.push_back(type);

  auto it = chunks.begin();
  while (it != chunks.end()) {
    (*it)->add_segment(make_shared_by_data_type<BaseSegment,ValueSegment>(type));
    it++;
  }
}

void Table::append(std::vector<AllTypeVariant> values) {
  auto chunk = chunks.back();
  if(chunk->size() >= m_max_chunk_size) {
    chunk = std::make_shared<Chunk>();

    auto it = m_column_types.begin();
    while(it != m_column_types.end()) {
      chunk->add_segment(make_shared_by_data_type<BaseSegment,ValueSegment>(*it));
      ++it;
    }

    chunks.push_back(chunk);
  }
  chunk->append(values);

}

uint16_t Table::column_count() const {
  return (*chunks[0]).column_count();
}

uint64_t Table::row_count() const {
  return (chunks.size() - 1) * max_chunk_size() + chunks.back()->size();
}

ChunkID Table::chunk_count() const {
  return ChunkID{chunks.size()};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto it = m_column_names.begin();
  while (it != m_column_names.end()) {
    if((*it) == column_name) {
      return ColumnID{std::distance(m_column_names.begin(), it)};
    }
    it++;
  }
  throw std::exception();
}

uint32_t Table::max_chunk_size() const {
  return m_max_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  return m_column_names;
}

const std::string& Table::column_name(ColumnID column_id) const {
  return m_column_names[column_id];
}

const std::string& Table::column_type(ColumnID column_id) const {
  return m_column_types[column_id];
}

Chunk& Table::get_chunk(ChunkID chunk_id) {
  return *chunks[chunk_id];
}

const Chunk& Table::get_chunk(ChunkID chunk_id) const {
  return *chunks[chunk_id];
}

}  // namespace opossum
