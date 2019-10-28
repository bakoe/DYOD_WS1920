#include "storage_manager.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utils/assert.hpp"

namespace opossum {

StorageManager& StorageManager::get() {
  static StorageManager instance;
  return instance;
}

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) {
  _table_names.push_back(name);
  _tables.push_back(table);
}

void StorageManager::drop_table(const std::string& name) {
  for (size_t table_index = 0; table_index < _table_names.size(); table_index++) {
    if (_table_names[table_index].compare(name) == 0) {
      _tables.erase(_tables.begin() + table_index);
      _table_names.erase(_table_names.begin() + table_index);
      return;
    }
  }
  throw std::exception();
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  for (size_t table_index = 0; table_index < _table_names.size(); table_index++) {
    if (_table_names[table_index].compare(name) == 0) {
      return _tables[table_index];
    }
  }
  throw std::exception();
}

bool StorageManager::has_table(const std::string& name) const {
  for (size_t table_index = 0; table_index < _table_names.size(); table_index++) {
    if (_table_names[table_index].compare(name) == 0) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> StorageManager::table_names() const { return _table_names; }

void StorageManager::print(std::ostream& out) const {
  out << "table name"
      << " | ";
  out << "column count"
      << " | ";
  out << "row count"
      << " | ";
  out << "chunk count" << std::endl;

  for (size_t table_index = 0; table_index < _table_names.size(); table_index++) {
    auto const table = _tables[table_index];
    out << _table_names[table_index] << " | ";
    out << table->column_count() << " | ";
    out << table->row_count() << " | ";
    out << table->chunk_count() << std::endl;
  }
}

void StorageManager::reset() {
  // TODO(anyone): how can I actually delete the Singleton instance?
  _table_names.clear();
  _tables.clear();
}

}  // namespace opossum
