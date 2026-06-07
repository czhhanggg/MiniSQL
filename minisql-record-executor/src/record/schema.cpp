#include "record/schema.h"

/**
 * TODO: Student Implement
 */
uint32_t Schema::SerializeTo(char *buf) const {
  char *start = buf;
  MACH_WRITE_UINT32(buf, SCHEMA_MAGIC_NUM);
  buf += 4;
  MACH_WRITE_UINT32(buf, static_cast<uint32_t>(columns_.size()));
  buf += 4;
  for (auto column : columns_) {
    buf += column->SerializeTo(buf);
  }
  return static_cast<uint32_t>(buf - start);
}

uint32_t Schema::GetSerializedSize() const {
  uint32_t size = 8;
  for (auto column : columns_) {
    size += column->GetSerializedSize();
  }
  return size;
}

uint32_t Schema::DeserializeFrom(char *buf, Schema *&schema) {
  char *start = buf;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  buf += 4;
  ASSERT(magic_num == SCHEMA_MAGIC_NUM, "Failed to deserialize schema.");
  uint32_t column_count = MACH_READ_UINT32(buf);
  buf += 4;
  std::vector<Column *> columns;
  columns.reserve(column_count);
  for (uint32_t i = 0; i < column_count; i++) {
    Column *column = nullptr;
    buf += Column::DeserializeFrom(buf, column);
    columns.push_back(column);
  }
  schema = new Schema(columns);
  return static_cast<uint32_t>(buf - start);
}
