#!/usr/bin/env bash
set -euo pipefail
OUT="sql.txt"
# 将输出写到仓库根目录下的 sql.txt，包含 USE db; 和 100000 条 INSERT 语句
echo 'use db1;' > "$OUT"
# 生成 100000 条记录：id 从 1 到 100000，name 为 user%06d，balance = id * 0.01
for i in $(seq 1 100000); do
  name=$(printf "user%06d" "$i")
  balance=$(awk -v i="$i" 'BEGIN{printf "%.2f", i*0.01}')
  printf 'insert into account values(%d,"%s",%s);\n' "$i" "$name" "$balance" >> "$OUT"
done

echo "Generated $OUT with 100000 INSERT statements."
