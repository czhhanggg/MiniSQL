#include "record/row.h"

/**
 * TODO: Student Implement
 */
uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
  char *start = buf;
  uint32_t field_count = schema->GetColumnCount();
  uint32_t bitmap_size = (field_count + 7) / 8;

  MACH_WRITE_UINT32(buf, field_count);
  buf += 4;
  memset(buf, 0, bitmap_size);
  for (uint32_t i = 0; i < field_count; i++) {
    if (fields_[i]->IsNull()) {
      buf[i / 8] = static_cast<char>(buf[i / 8] | (1U << (i % 8)));
    }
  }
  buf += bitmap_size;

  for (uint32_t i = 0; i < field_count; i++) {
    if (!fields_[i]->IsNull()) {
      buf += fields_[i]->SerializeTo(buf);
    }
  }
  return static_cast<uint32_t>(buf - start);
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(fields_.empty(), "Non empty field in row.");
  char *start = buf;
  uint32_t field_count = MACH_READ_UINT32(buf);
  buf += 4;
  ASSERT(field_count == schema->GetColumnCount(), "Field count does not match schema.");
  uint32_t bitmap_size = (field_count + 7) / 8;
  unsigned char *null_bitmap = reinterpret_cast<unsigned char *>(buf);
  buf += bitmap_size;

  for (uint32_t i = 0; i < field_count; i++) {
    bool is_null = (null_bitmap[i / 8] & (1U << (i % 8))) != 0;
    Field *field = nullptr;
    buf += Field::DeserializeFrom(buf, schema->GetColumn(i)->GetType(), &field, is_null);
    fields_.push_back(field);
  }
  return static_cast<uint32_t>(buf - start);
}

uint32_t Row::GetSerializedSize(Schema *schema) const {
  ASSERT(schema != nullptr, "Invalid schema before serialize.");
  ASSERT(schema->GetColumnCount() == fields_.size(), "Fields size do not match schema's column size.");
  uint32_t size = 4 + (schema->GetColumnCount() + 7) / 8;
  for (auto field : fields_) {
    size += field->GetSerializedSize();
  }
  return size;
}

void Row::GetKeyFromRow(const Schema *schema, const Schema *key_schema, Row &key_row) const {
  auto columns = key_schema->GetColumns();
  std::vector<Field> fields;
  uint32_t idx;
  for (auto column : columns) {
    schema->GetColumnIndex(column->GetName(), idx);
    fields.emplace_back(*this->GetField(idx));
  }
  key_row = Row(fields);
}
