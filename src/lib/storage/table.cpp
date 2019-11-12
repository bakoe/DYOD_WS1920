#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "value_segment.hpp"

#include "dictionary_segment.hpp"
#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

Table::Table(uint32_t chunk_size) {
  _maximum_chunk_size = chunk_size;
  // Automatically create the first chunk when creating a Table
  auto first_chunk = std::make_shared<Chunk>();
  _chunks.push_back(first_chunk);
}

void Table::add_column(const std::string& name, const std::string& type) {
  // Store the name and type of the to-be-added column
  _column_names.push_back(name);
  _column_types.push_back(type);

  // Add a ValueSegment for the new column to each of the chunks
  for (ChunkID chunk_index = ChunkID{0}; chunk_index < _chunks.size(); chunk_index++) {
    auto segment = make_shared_by_data_type<BaseSegment, ValueSegment>(type);
    _chunks[chunk_index]->add_segment(segment);
  }
}

void Table::append(const std::vector<AllTypeVariant> values) {
  // Get the last "free" chunk while potentially creating a new one if the last one is full
  std::shared_ptr<Chunk> last_chunk = _chunks.back();
  if (last_chunk->size() == _maximum_chunk_size) {
    auto new_chunk = std::make_shared<Chunk>();
    for (auto const& column_type : _column_types) {
      new_chunk->add_segment(make_shared_by_data_type<BaseSegment, ValueSegment>(column_type));
    }
    _chunks.push_back(new_chunk);
    last_chunk = new_chunk;
  }

  // Append the to-be-appended values to the last chunk
  last_chunk->append(values);
}

uint16_t Table::column_count() const { return _column_names.size(); }

uint64_t Table::row_count() const {
  uint64_t row_count = 0;
  for (ChunkID chunk_index = ChunkID{0}; chunk_index < _chunks.size(); chunk_index++) {
    row_count += _chunks[chunk_index]->size();
  }
  return row_count;
}

ChunkID Table::chunk_count() const { return ChunkID{_chunks.size()}; }

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto iterator = std::find(_column_names.begin(), _column_names.end(), column_name);
  if (iterator != _column_names.end()) return ColumnID{std::distance(_column_names.begin(), iterator)};
  throw std::exception();
}

uint32_t Table::max_chunk_size() const { return _maximum_chunk_size; }

const std::vector<std::string>& Table::column_names() const { return _column_names; }

const std::string& Table::column_name(ColumnID column_id) const { return _column_names[column_id]; }

const std::string& Table::column_type(ColumnID column_id) const { return _column_types[column_id]; }

Chunk& Table::get_chunk(ChunkID chunk_id) { return *_chunks[chunk_id]; }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return *_chunks[chunk_id]; }

// TODO(anyone): Check if this is the right way to create the mutex
std::mutex chunk_access_mutex;

void Table::compress_chunk(ChunkID chunk_id) {
  auto newChunk = std::make_shared<Chunk>();
  auto oldChunk = _chunks.at(chunk_id);

  // TODO(anyone): Replace all camelCase variables with sneaky_eaky_case

  std::vector<std::thread> threads(oldChunk->column_count());
  std::vector<std::shared_ptr<BaseSegment>> compressed_segments(oldChunk->column_count());

  auto compress_segment = [&compressed_segments](const std::string& column_type,
                                                 const std::shared_ptr<BaseSegment>& uncompressed_segment,
                                                 const ColumnID& segment_index) {
    compressed_segments[segment_index] =
        make_shared_by_data_type<BaseSegment, DictionarySegment>(column_type, uncompressed_segment);
  };

  for (auto segment_index = ColumnID{0}; segment_index < oldChunk->column_count(); segment_index++) {
    threads[segment_index] =
        std::thread(compress_segment, column_type(segment_index), oldChunk->get_segment(segment_index), segment_index);
  }

  for (auto thread_index = ColumnID{0}; thread_index < threads.size(); thread_index++) {
    threads[thread_index].join();
    newChunk->add_segment(compressed_segments[thread_index]);
  }

  // TODO(anyone): Check if this is the right way to lock (without unlocking?) the mutex
  chunk_access_mutex.lock();
  _chunks[chunk_id] = newChunk;
}

}  // namespace opossum
