#include "record/column.h"

#include "glog/logging.h"

Column::Column(std::string column_name, TypeId type, uint32_t index, bool nullable, bool unique)
    : name_(std::move(column_name)), type_(type), table_ind_(index), nullable_(nullable), unique_(unique) {
  ASSERT(type != TypeId::kTypeChar, "Wrong constructor for CHAR type.");
  switch (type) {
    case TypeId::kTypeInt:
      len_ = sizeof(int32_t);
      break;
    case TypeId::kTypeFloat:
      len_ = sizeof(float_t);
      break;
    default:
      ASSERT(false, "Unsupported column type.");
  }
}

Column::Column(std::string column_name, TypeId type, uint32_t length, uint32_t index, bool nullable, bool unique)
    : name_(std::move(column_name)),
      type_(type),
      len_(length),
      table_ind_(index),
      nullable_(nullable),
      unique_(unique) {
  ASSERT(type == TypeId::kTypeChar, "Wrong constructor for non-VARCHAR type.");
}

Column::Column(const Column *other)
    : name_(other->name_),
      type_(other->type_),
      len_(other->len_),
      table_ind_(other->table_ind_),
      nullable_(other->nullable_),
      unique_(other->unique_) {}

/**
* TODO: Student Implement
*/
uint32_t Column::SerializeTo(char *buf) const {
  char *start = buf;
  MACH_WRITE_UINT32(buf, COLUMN_MAGIC_NUM);
  buf += 4;
  MACH_WRITE_UINT32(buf, static_cast<uint32_t>(name_.size()));
  buf += 4;
  MACH_WRITE_STRING(buf, name_);
  buf += name_.size();
  MACH_WRITE_TO(TypeId, buf, type_);
  buf += 4;
  MACH_WRITE_UINT32(buf, len_);
  buf += 4;
  MACH_WRITE_UINT32(buf, table_ind_);
  buf += 4;
  MACH_WRITE_UINT32(buf, nullable_ ? 1U : 0U);
  buf += 4;
  MACH_WRITE_UINT32(buf, unique_ ? 1U : 0U);
  buf += 4;
  return static_cast<uint32_t>(buf - start);
}

/**
 * TODO: Student Implement
 */
uint32_t Column::GetSerializedSize() const {
  return 4 + 4 + static_cast<uint32_t>(name_.size()) + 4 + 4 + 4 + 4 + 4;
}

/**
 * TODO: Student Implement
 */
uint32_t Column::DeserializeFrom(char *buf, Column *&column) {
  char *start = buf;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  buf += 4;
  ASSERT(magic_num == COLUMN_MAGIC_NUM, "Failed to deserialize column.");
  uint32_t name_len = MACH_READ_UINT32(buf);
  buf += 4;
  std::string name(buf, name_len);
  buf += name_len;
  auto type = MACH_READ_FROM(TypeId, buf);
  buf += 4;
  uint32_t len = MACH_READ_UINT32(buf);
  buf += 4;
  uint32_t table_ind = MACH_READ_UINT32(buf);
  buf += 4;
  bool nullable = MACH_READ_UINT32(buf) != 0;
  buf += 4;
  bool unique = MACH_READ_UINT32(buf) != 0;
  buf += 4;

  if (type == TypeId::kTypeChar) {
    column = new Column(name, type, len, table_ind, nullable, unique);
  } else {
    column = new Column(name, type, table_ind, nullable, unique);
  }
  return static_cast<uint32_t>(buf - start);
}
