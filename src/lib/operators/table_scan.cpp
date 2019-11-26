#include <memory>
#include <string>
#include <vector>

#include "resolve_type.hpp"
#include "storage/dictionary_segment.hpp"
#include "storage/reference_segment.hpp"
#include "storage/table.hpp"
#include "table_scan.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
                     const AllTypeVariant search_value)
    : AbstractOperator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) {}

TableScan::~TableScan() {}

ColumnID TableScan::column_id() const { return _column_id; }

ScanType TableScan::scan_type() const { return _scan_type; }

const AllTypeVariant& TableScan::search_value() const { return _search_value; }

std::shared_ptr<const Table> TableScan::_on_execute() {
  auto referenced_table = _input_table_left();
  bool is_referenced_table_replaced = false;
  auto matching_pos_list = std::make_shared<PosList>();

  // Is used to do the actual comparsion of values.
  auto matches_scan_criterion = [](const auto& existing_value, const auto& search_value,
                                   const ScanType& scan_type) -> bool {
    switch (scan_type) {
      case ScanType::OpEquals:
        return existing_value == search_value;
      case ScanType::OpNotEquals:
        return existing_value != search_value;
      case ScanType::OpLessThan:
        return existing_value < search_value;
      case ScanType::OpLessThanEquals:
        return existing_value <= search_value;
      case ScanType::OpGreaterThan:
        return existing_value > search_value;
      case ScanType::OpGreaterThanEquals:
        return existing_value >= search_value;
    }
  };

  for (ChunkID chunk_id{0}; chunk_id < _input_table_left()->chunk_count(); ++chunk_id) {
    const auto& chunk = _input_table_left()->get_chunk(chunk_id);
    const auto& segment = chunk.get_segment(_column_id);

    // Use column type to dynamically cast segments.
    resolve_data_type(_input_table_left()->column_type(_column_id), [&](auto type) {
      using Type = typename decltype(type)::type;

      std::shared_ptr<ValueSegment<Type>> value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
      if (value_segment != nullptr) {
        // For ValueSegments we can simply scan on the stored values.
        const auto& values = value_segment->values();
        for (ChunkOffset value_index = 0; value_index < values.size(); value_index++) {
          const auto& value = values[value_index];
          if (matches_scan_criterion(value, type_cast<Type>(_search_value), _scan_type)) {
            matching_pos_list->push_back(RowID{chunk_id, value_index});
          }
        }
        return;
      }
      auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
      if (dictionary_segment != nullptr) {
        std::shared_ptr<const BaseAttributeVector> attribute_vector = dictionary_segment->attribute_vector();

        ValueID lower_bound = dictionary_segment->lower_bound(_search_value);
        ValueID upper_bound = dictionary_segment->upper_bound(_search_value);

          switch (_scan_type) {
            case ScanType::OpEquals:
              if (lower_bound == upper_bound) {
                return;
              }
              break;
            case ScanType::OpNotEquals:
              if (lower_bound == 0 && upper_bound == dictionary_segment->unique_values_count()) {
                return;
              }
              break;
            case ScanType::OpLessThan:
              if (lower_bound == 0) {
                return;
              }
              break;
            case ScanType::OpLessThanEquals:
              if (upper_bound == 0) {
                return;
              }
              break;
            case ScanType::OpGreaterThan:
              if (upper_bound == dictionary_segment->unique_values_count()) {
                return;
              }
              break;
            case ScanType::OpGreaterThanEquals:
              if (lower_bound == dictionary_segment->unique_values_count()) {
                return;
              }
              break;
          }

          for (ChunkOffset attribute_vector_index = 0; attribute_vector_index < attribute_vector->size();
               attribute_vector_index++) {
            switch (_scan_type) {
              case ScanType::OpEquals:
                if (ValueID{attribute_vector->get(attribute_vector_index)} == lower_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
              break;
              case ScanType::OpNotEquals:
                if (ValueID{attribute_vector->get(attribute_vector_index)} < lower_bound ||
                    ValueID{attribute_vector->get(attribute_vector_index)} >= upper_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
                break;
              case ScanType::OpLessThan:
                if (ValueID{attribute_vector->get(attribute_vector_index)} < lower_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
                break;
              case ScanType::OpLessThanEquals:
                if (ValueID{attribute_vector->get(attribute_vector_index)} < upper_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
                break;
              case ScanType::OpGreaterThan:
                if (ValueID{attribute_vector->get(attribute_vector_index)} >= upper_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
                break;
              case ScanType::OpGreaterThanEquals:
                if (ValueID{attribute_vector->get(attribute_vector_index)} >= lower_bound) {
                  matching_pos_list->push_back(RowID{chunk_id, attribute_vector_index});
                }
                break;
            }

          }

        return;
      }
      auto reference_segment = std::dynamic_pointer_cast<ReferenceSegment>(segment);
      if (reference_segment != nullptr) {
        const auto& referenced_pos_list = reference_segment->pos_list();
        const auto& table_referenced_by_segment = reference_segment->referenced_table();
        for (const RowID& referenced_pos : *referenced_pos_list) {
          // TODO(students): Use a factored out version of the two code blocks declared above
          const auto& referenced_segment =
              table_referenced_by_segment->get_chunk(referenced_pos.chunk_id).get_segment(_column_id);
          // TODO(students): (Together with TODO above:) Remove this type_cast which leads to precision loss
          // TODO(students): Then isn't necessary anymore because we don't work on AllTypeVariant but on T
          const auto& value = type_cast<Type>((*referenced_segment)[referenced_pos.chunk_offset]);
          if (matches_scan_criterion(value, type_cast<Type>(_search_value), _scan_type)) {
            if (!is_referenced_table_replaced) {
              referenced_table = table_referenced_by_segment;
              is_referenced_table_replaced = true;
            }
            matching_pos_list->push_back(referenced_pos);
          }
        }
      }
      return;
    });
  }

  auto table = std::make_shared<Table>();

  // Create table column names and types according to input table.
  for (ColumnID column_id{0}; column_id < _input_table_left()->column_count(); ++column_id) {
    table->add_column_definition(_input_table_left()->column_name(column_id),
                                 _input_table_left()->column_type(column_id));
  }

  auto& chunk = table->get_chunk(ChunkID{0});

  // Create reference segments for every column and add them to the chunk.
  for (ColumnID column_id{0}; column_id < _input_table_left()->column_count(); ++column_id) {
    const auto segment = std::make_shared<ReferenceSegment>(referenced_table, column_id, matching_pos_list);
    chunk.add_segment(segment);
  }

  return table;
}

}  // namespace opossum
