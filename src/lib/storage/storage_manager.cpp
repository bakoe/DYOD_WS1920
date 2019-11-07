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

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) { _tables.insert({name, table}); }

void StorageManager::drop_table(const std::string& name) {
  if (!(has_table(name))) throw std::exception();
  _tables.erase(name);
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (!(has_table(name))) throw std::exception();
  return _tables.at(name);
}

bool StorageManager::has_table(const std::string& name) const { return _tables.count(name); }

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> table_names;
  for (const auto& _table : _tables) {
    table_names.push_back(_table.first);
  }
  return table_names;
}

void StorageManager::print(std::ostream& out) const {
  for (const auto& _table : _tables) {
    const std::string table_name = _table.first;
    const std::shared_ptr<Table> table = _table.second;
    out << table_name << ", " << table->column_count() << ", " << table->row_count() << ", " << table->chunk_count();
    out << std::endl;
  }
}

void StorageManager::reset() { _tables.clear(); }

}  // namespace opossum
