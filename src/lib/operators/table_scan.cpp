#include "table_scan.hpp"
#include <resolve_type.hpp>
#include <storage/dictionary_segment.hpp>
#include "storage/table.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
                     const AllTypeVariant search_value)
    : AbstractOperator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) {}

TableScan::~TableScan() {
  // TODO Implement
}

ColumnID TableScan::column_id() const { return _column_id; };

ScanType TableScan::scan_type() const { return _scan_type; };

const AllTypeVariant& TableScan::search_value() const { return _search_value; };

std::shared_ptr<const Table> TableScan::_on_execute() {
  for (ChunkID chunk_id{0}; chunk_id < _input_table_left()->chunk_count(); ++chunk_id) {
    const auto& chunk = _input_table_left()->get_chunk(chunk_id);

    const auto& segment = chunk.get_segment(_column_id);

    resolve_data_type("std::string", [&](auto type) {
      using Type = typename decltype(type)::type;
      auto value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
      if (value_segment != nullptr) {
        // TODO Handle ValueSegment with data type Type (here: std::string) accordingly
      }
      auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
      if (dictionary_segment != nullptr) {
        // TODO Handle DictionarySegment with data type Type (here: std::string) accordingly
      }
      // TODO Add additional check for possible ReferenceSegment type when the ReferenceSegment is implemented
    });

    // TODO Add resolve_data_type() calls for other data types such as double, integer etc.(?)
  }

  // TODO Create a new Table containing one Chunk with a ReferenceSegment (if there are results; empty otherwise)
  // TODO pointing to the results of the operator; return a shared_ptr to it.
  return nullptr;
};

}  // namespace opossum
