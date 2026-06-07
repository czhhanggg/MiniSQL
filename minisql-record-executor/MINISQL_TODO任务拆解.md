# MiniSQL 按文件 TODO 任务拆解

## 1. 文档用途

这份文档是对 [MINISQL_开发计划书](d:\fire wheel\code\minisql-master\MINISQL_开发计划书.md) 的进一步细化。

如果上一份文档回答的是：

- “项目整体要做什么”
- “模块该怎么分工”

那么这一份回答的是：

- “每个文件具体还差什么”
- “每个 TODO 该怎么下手”
- “每个文件改完应该跑哪个测试”

这份文档适合直接拿来认领任务。

---

## 2. 总体认领建议

建议按以下 3 条主线认领：

### 主线 A：底层存储

- `bitmap_page.cpp`
- `disk_manager.cpp`
- `lru_replacer.cpp`
- `buffer_pool_manager.cpp`

### 主线 B：记录与目录

- `row.cpp`
- `column.cpp`
- `schema.cpp`
- `table_heap.cpp`
- `table_iterator.cpp`
- `table.cpp`
- `indexes.cpp`
- `catalog.cpp`

### 主线 C：索引与执行

- `b_plus_tree_page.cpp`
- `b_plus_tree_internal_page.cpp`
- `b_plus_tree_leaf_page.cpp`
- `b_plus_tree.cpp`
- `index_iterator.cpp`
- `execute_engine.cpp`
- 各执行器联调

`lock_manager.cpp` 和 `recovery/*.h` 最后认领。

---

## 3. 文件级任务清单

## 3.1 `src/page/bitmap_page.cpp`

### 需要完成

- `AllocatePage`
- `DeAllocatePage`
- `IsPageFree`
- `IsPageFreeLow`

### 实现目标

- 把一个位图页当成若干 bit 的分配表
- `0` 表示空闲，`1` 表示已占用
- 支持分配一个空闲页并返回 offset
- 支持回收一个 offset
- 支持检查 offset 是否空闲

### 实现建议

- 先理解 `BitmapPage<PageSize>::GetMaxSupportedSize()`
- 典型做法：
  - `byte_index = page_offset / 8`
  - `bit_index = page_offset % 8`
- `AllocatePage` 可以利用页内维护的：
  - `page_allocated_`
  - `next_free_page_`
- 分配成功后：
  - 把 bit 置 1
  - 更新 `page_allocated_`
  - 移动 `next_free_page_`

### 验收测试

- `test/storage/disk_manager_test.cpp`
  - `BitMapPageTest`

### 风险点

- 重复释放
- `next_free_page_` 失效
- bit 操作写反

---

## 3.2 `src/storage/disk_manager.cpp`

### 需要完成

- `AllocatePage`
- `DeAllocatePage`
- `IsPageFree`
- `MapPageId`

### 实现目标

- 用 extent + bitmap 机制管理整份数据库文件中的数据页
- 对上层暴露连续的逻辑页号

### 实现建议

- 先理解物理布局：
  - `META_PAGE_ID`
  - 每个 extent 的 bitmap page
  - extent 中的数据页
- `MapPageId` 是本文件最重要的辅助函数

建议先把映射公式单独写出来：

- 一个 extent 可管理 `BITMAP_SIZE` 个逻辑页
- extent 编号：
  - `extent_id = logical_page_id / BITMAP_SIZE`
- extent 内 offset：
  - `page_offset = logical_page_id % BITMAP_SIZE`
- 每个 extent 占：
  - `1` 个 bitmap page
  - `BITMAP_SIZE` 个 data page

物理页通常应考虑：

- 0 号页是 meta page
- 每个 extent 前面有一个 bitmap page

### `AllocatePage` 建议步骤

1. 从 meta page 看现有 extent 是否有空闲页
2. 若有，在对应 bitmap page 中分配
3. 若没有，扩展新 extent
4. 更新 meta page 中：
   - 总分区数
   - 总已分配页数
   - 每个 extent 已使用页数
5. 返回逻辑页号

### `DeAllocatePage` 建议步骤

1. 找到逻辑页所在 extent
2. 加载 bitmap page
3. 清除该 bit
4. 更新 meta page 计数

### 测试

- `test/storage/disk_manager_test.cpp`
  - `FreePageAllocationTest`

### 风险点

- 物理页号公式算错
- 新建 extent 时忘记更新 meta page
- 读写 bitmap page 时页号偏移错误

---

## 3.3 `src/buffer/lru_replacer.cpp`

### 需要完成

- 构造函数内部初始化
- `Victim`
- `Pin`
- `Unpin`
- `Size`

### 实现目标

- 维护当前“可淘汰页帧”的 LRU 队列

### 推荐数据结构

- `std::list<frame_id_t>` 维护顺序
- `unordered_map / unordered_set` 维护是否在 LRU 中及定位

### 实现建议

- `Pin(frame_id)`：
  - 如果 frame 在可淘汰队列里，删掉
- `Unpin(frame_id)`：
  - 如果 frame 不在可淘汰队列里，插到队尾
- `Victim`：
  - 取队首最久未使用页
- `Size`：
  - 返回当前可淘汰元素数量

### 测试

- `test/buffer/lru_replacer_test.cpp`

### 风险点

- 同一个 frame 重复插入
- `Pin` 后没真正移除

---

## 3.4 `src/buffer/buffer_pool_manager.cpp`

### 需要完成

- `FetchPage`
- `NewPage`
- `DeletePage`
- `UnpinPage`
- `FlushPage`

### 实现目标

- 管理内存页缓存
- 在磁盘和内存之间搬运页

### 本文件关键状态

- `page_table_`
- `free_list_`
- `replacer_`
- `pages_`

### `FetchPage` 建议步骤

1. 查 `page_table_`
2. 若命中：
   - `pin_count_++`
   - `replacer_->Pin(frame)`
   - 返回页
3. 若未命中：
   - 从 `free_list_` 找 frame
   - 若无空闲，再从 `replacer_` 找 victim
4. victim 若是脏页则刷盘
5. 从 `page_table_` 移除旧页映射
6. 从磁盘读入新页
7. 初始化元数据：
   - `page_id_`
   - `pin_count_ = 1`
   - `is_dirty_ = false`
8. 更新 `page_table_`

### `NewPage` 建议步骤

1. 调 `AllocatePage`
2. 找可用 frame
3. victim 若脏则刷盘
4. 清空 `data_`
5. 初始化新页元数据
6. 更新 `page_table_`
7. 返回新页

### `DeletePage` 建议步骤

1. 调 `DeallocatePage`
2. 查页是否在 buffer pool
3. 不在则直接成功
4. 在但 `pin_count_ != 0` 则失败
5. 否则清理元数据、移出 `page_table_`、放回 `free_list_`

### `UnpinPage`

- 找不到页返回 `false`
- `pin_count_ == 0` 也应返回 `false`
- `pin_count_--`
- 若 `is_dirty = true` 则置脏
- 若 `pin_count_` 降到 0，交给 `replacer_->Unpin`

### `FlushPage`

- 找不到返回 `false`
- 把页内容写盘
- 清 `is_dirty_`

### 测试

- `test/buffer/buffer_pool_manager_test.cpp`

### 风险点

- victim 脏页没刷
- `page_table_` 新旧映射冲突
- `pin_count_` 维护错误

---

## 3.5 `src/record/column.cpp`

### 需要完成

- `SerializeTo`
- `GetSerializedSize`
- `DeserializeFrom`

### 实现目标

- 持久化列定义

### 序列化内容建议

- magic number
- 列名
- 类型
- 长度
- table index
- nullable
- unique

### 注意

- `char(n)` 和非 `char` 字段构造函数不同
- 反序列化时根据类型决定调用哪个构造函数

### 测试

- `test/record/tuple_test.cpp`

---

## 3.6 `src/record/schema.cpp`

### 需要完成

- `SerializeTo`
- `GetSerializedSize`
- `DeserializeFrom`

### 实现目标

- 持久化 schema，也就是列数组

### 实现建议

- 写入：
  - magic
  - 列数
  - 每一列序列化内容
- 读取：
  - 反序列化出每一列
  - 组装成 `Schema`

### 测试

- `test/record/tuple_test.cpp`

---

## 3.7 `src/record/row.cpp`

### 需要完成

- `SerializeTo`
- `DeserializeFrom`
- `GetSerializedSize`

### 实现目标

- 持久化一行记录

### 实现建议

- `Row` 序列化时需要考虑 `NULL bitmap`
- 每个字段按 schema 顺序写
- `Field` 的实现框架里已给，可以参照调用

### 典型布局建议

- field 数量或必要元信息
- null bitmap
- 各 field 内容

### 测试

- `test/record/tuple_test.cpp`

### 风险点

- `NULL` 字段占位处理错误
- `GetSerializedSize` 和实际写入不一致

---

## 3.8 `src/storage/table_heap.cpp`

### 需要完成

- `InsertTuple`
- `UpdateTuple`
- `ApplyDelete`
- `GetTuple`
- `Begin`
- `End`

### 实现目标

- 基于 `TablePage` 双向链表管理堆表

### `InsertTuple` 建议步骤

1. 先检查 tuple 是否超过页大小
2. 从 `first_page_id_` 开始找能容纳的 `TablePage`
3. 找到就插入
4. 找不到就分配新页并链接到表尾
5. 把新生成的 `rid` 写回 `row`

### `UpdateTuple`

- 先定位旧 tuple
- 调页内更新逻辑
- 若页内放不下，按实验说明允许返回失败给上层处理

### `ApplyDelete`

1. `FetchPage(rid.page_id)`
2. `ApplyDelete(rid, txn, ...)`
3. `Unpin`

### `GetTuple`

- 根据 `rid` 取页
- 从页中读 tuple 到 `row`

### `Begin / End`

- `Begin` 应找到第一条有效记录
- `End` 返回无效 `rid` 的迭代器

### 测试

- `test/storage/table_heap_test.cpp`

### 风险点

- 新页链表前后指针维护错
- 删除后遍历失效

---

## 3.9 `src/storage/table_iterator.cpp`

### 需要完成

- 构造函数
- 拷贝构造
- 析构
- `==`
- `!=`
- `operator*`
- `operator->`
- `operator=`
- 前置 `++`
- 后置 `++`

### 实现目标

- 遍历 `TableHeap`

### 推荐成员

- `TableHeap *`
- `RowId current_rid`
- `Txn *`
- 当前 `Row` 缓存

### `operator++` 建议逻辑

1. 在当前页找下一个 slot
2. 没有则跳下一页
3. 一直找到下一个有效 tuple
4. 找不到则置为 end

### 测试

- `test/storage/table_heap_test.cpp`

---

## 3.10 `src/page/b_plus_tree_page.cpp`

### 需要完成

- `IsLeafPage`
- `IsRootPage`
- `SetPageType`
- `GetMaxSize`
- `SetMaxSize`
- `GetMinSize`
- `GetParentPageId`

### 实现目标

- 提供 B+ 树公共页头行为

### 关键点

- `IsRootPage` 一般看 `parent_page_id_ == INVALID_PAGE_ID`
- `GetMinSize` 不能简单固定为 `max/2`
  - 根页和普通页不同
  - 叶子页和内部页不同

### 测试

- 会被所有 `b_plus_tree_*` 测试覆盖

---

## 3.11 `src/page/b_plus_tree_internal_page.cpp`

### 需要完成

- `Init`
- `Lookup`
- `PopulateNewRoot`
- `InsertNodeAfter`
- `MoveHalfTo`
- `CopyNFrom`
- `Remove`
- `RemoveAndReturnOnlyChild`
- `MoveAllTo`
- `MoveFirstToEndOf`
- `CopyLastFrom`
- `MoveLastToFrontOf`
- `CopyFirstFrom`

### 实现目标

- 内部节点数组管理

### 开发顺序建议

1. `Init`
2. `Lookup`
3. `InsertNodeAfter`
4. `MoveHalfTo / CopyNFrom`
5. 删除相关
6. 合并与借位相关

### 关键点

- 第一个 key 无效
- size 对内部节点表示“指针数”
- 页面迁移时要更新子页的 parent id

### 测试

- `test/index/b_plus_tree_test.cpp`

---

## 3.12 `src/page/b_plus_tree_leaf_page.cpp`

### 需要完成

- `Init`
- `KeyIndex`
- `Insert`
- `MoveHalfTo`
- `CopyNFrom`
- `Lookup`
- `RemoveAndDeleteRecord`
- `MoveAllTo`
- `MoveFirstToEndOf`
- `CopyLastFrom`
- `MoveLastToFrontOf`
- `CopyFirstFrom`

### 实现目标

- 叶子页有序存储 `key -> RowId`

### 开发顺序建议

1. `Init`
2. `KeyIndex`
3. `Lookup`
4. `Insert`
5. `MoveHalfTo`
6. 删除与借位

### 关键点

- `next_page_id_` 维护叶子链
- 范围扫描依赖这个链

### 测试

- `test/index/b_plus_tree_test.cpp`
- `test/index/index_iterator_test.cpp`

---

## 3.13 `src/index/b_plus_tree.cpp`

### 需要完成

- 构造函数
- `Destroy`
- `IsEmpty`
- `GetValue`
- `Insert`
- `StartNewTree`
- `InsertIntoLeaf`
- 两个 `Split`
- `InsertIntoParent`
- `Remove`
- `CoalesceOrRedistribute`
- 两个 `Coalesce`
- 两个 `Redistribute`
- `AdjustRoot`
- `Begin`
- `Begin(key)`
- `End`
- `FindLeafPage`
- `UpdateRootPageId`

### 实现目标

- 整棵 B+ 树正确维护

### 推荐实现顺序

#### 第一步：只做查找

- `IsEmpty`
- `FindLeafPage`
- `GetValue`

#### 第二步：只做单叶子页插入

- `StartNewTree`
- `InsertIntoLeaf`
- `Insert`

#### 第三步：做叶分裂和父插入

- `Split(LeafPage)`
- `InsertIntoParent`

#### 第四步：做内部节点分裂

- `Split(InternalPage)`

#### 第五步：做删除

- `Remove`
- `CoalesceOrRedistribute`
- `Coalesce`
- `Redistribute`
- `AdjustRoot`

#### 第六步：做迭代器入口

- `Begin`
- `Begin(key)`
- `End`

### 关键点

- 每次 root 变化都要 `UpdateRootPageId`
- 每次 `FetchPage` 后最终必须 `Unpin`

### 测试

- `test/index/b_plus_tree_test.cpp`
- `test/index/b_plus_tree_index_test.cpp`

---

## 3.14 `src/index/index_iterator.cpp`

### 需要完成

- `operator*`
- `operator++`

### 实现目标

- 沿叶子页链顺序遍历索引项

### `operator++` 建议逻辑

1. 若当前页还有下一个 item，`item_index++`
2. 否则看 `next_page_id`
3. 若有下一页：
   - `Unpin` 当前页
   - `Fetch` 下一页
   - `item_index = 0`
4. 若没有下一页：
   - 置为 end

### 测试

- `test/index/index_iterator_test.cpp`

---

## 3.15 `src/catalog/table.cpp`

### 需要完成

- `TableMetadata::GetSerializedSize`

### 现状

- 这个函数其实已经写了返回值
- 虽然注释还是 TODO，但逻辑上已经基本完成

### 你们要做的事

- 确认该返回值和实际序列化布局一致
- 不必把它当成重点工作项

### 测试

- `test/catalog/catalog_test.cpp`

---

## 3.16 `src/catalog/indexes.cpp`

### 需要完成

- `IndexMetadata::GetSerializedSize`
- `IndexInfo::Init` 在头文件 `src/include/catalog/indexes.h`

### `GetSerializedSize` 建议

应包含：

- magic
- index id
- index name
- table id
- key count
- 每个 key map 项

### `IndexInfo::Init` 建议步骤

1. 保存 `meta_data_`
2. 根据 `table_info` 拿到表 schema
3. 通过 `meta_data_->GetKeyMapping()` 构造 `key_schema_`
4. 调 `CreateIndex`

### 风险点

- `ShallowCopySchema` / `DeepCopySchema` 选错
- key schema 的列顺序不对

### 测试

- `test/catalog/catalog_test.cpp`

---

## 3.17 `src/catalog/catalog.cpp`

### 需要完成

- `CatalogMeta::GetSerializedSize`
- `CatalogManager` 构造函数
- `CreateTable`
- `GetTable(name)`
- `GetTables`
- `CreateIndex`
- `GetIndex`
- `GetTableIndexes`
- `DropTable`
- `DropIndex`
- `FlushCatalogMetaPage`
- `LoadTable`
- `LoadIndex`
- `GetTable(table_id)`

### 实现目标

- 让数据库重启后还能恢复表和索引

### 构造函数建议

- `init = true`
  - 创建新的 `CatalogMeta`
  - 初始化 catalog meta page
- `init = false`
  - 从 `CATALOG_META_PAGE_ID` 读出 meta
  - 逐个 `LoadTable`
  - 逐个 `LoadIndex`

### `CreateTable` 建议步骤

1. 判重
2. 分配 `table_id`
3. 创建 `TableHeap`
4. 创建 `TableMetadata`
5. 分配 meta page 并写入
6. 建立内存映射
7. 更新 `catalog_meta_`
8. 刷新 catalog meta page

### `CreateIndex` 建议步骤

1. 检查表存在
2. 检查索引名是否重复
3. 把列名列表映射成 column index 列表
4. 创建 `IndexMetadata`
5. `IndexInfo::Init`
6. 分配索引元页并持久化
7. 将已有表记录导入索引
8. 更新内存映射和 catalog meta

### 测试

- `test/catalog/catalog_test.cpp`

### 风险点

- 建了索引元数据但没把已有记录导入索引
- `DropTable` 忘记连带删索引

---

## 3.18 `src/executor/execute_engine.cpp`

### 需要完成

- `ExecuteCreateTable`
- `ExecuteDropTable`
- `ExecuteShowIndexes`
- `ExecuteCreateIndex`
- `ExecuteDropIndex`
- `ExecuteExecfile`
- `ExecuteQuit`

### 实现目标

- 补上不走 Planner 的管理类 SQL

### `ExecuteCreateTable`

需要做：

1. 从 AST 取表名
2. 解析列定义
3. 识别主键列
4. 识别 `unique`
5. 生成 `Column` 数组与 `Schema`
6. 调 `CatalogManager::CreateTable`
7. 如果主键/unique 需要自动建索引，创建对应索引

### `ExecuteDropTable`

- 判当前数据库是否已选择
- 调 `CatalogManager::DropTable`

### `ExecuteShowIndexes`

- 遍历当前库表
- 每张表取 `GetTableIndexes`
- 格式化输出

### `ExecuteCreateIndex`

- 从 AST 提取：
  - 索引名
  - 表名
  - 列名列表
  - 索引类型
- 调 `CatalogManager::CreateIndex`

### `ExecuteDropIndex`

- 语雀 Q&A 明确说过：
  - parser 里 `drop index` 可能只有 index name 没有 table name
- 可选做法：
  - 对所有表搜索该 index name
  - 或假设索引名全局唯一

### `ExecuteExecfile`

- 打开 SQL 文件
- 逐条解析执行
- 统计总耗时

### `ExecuteQuit`

- 返回 `DB_QUIT`

### 测试

- `test/execution/executor_test.cpp`

### 验收强关联

- 这是最直接影响现场演示的文件之一

---

## 3.19 `src/concurrency/lock_manager.cpp`

### 需要完成

- `LockShared`
- `LockExclusive`
- `LockUpgrade`
- `Unlock`
- `LockPrepare`
- `CheckAbort`
- `AddEdge`
- `RemoveEdge`
- `HasCycle`
- `RunCycleDetection`
- `GetEdgeList`

### 实现目标

- 事务加锁
- 两阶段锁
- 死锁检测

### 推荐顺序

1. `LockPrepare`
2. `LockShared`
3. `LockExclusive`
4. `Unlock`
5. `LockUpgrade`
6. 再做等待图和死锁检测

### 测试

- `test/concurrency/lock_manager_test.cpp`

### 风险点

- 事务状态从 `GROWING` 到 `SHRINKING` 转换
- `UpgradeConflict`
- 后台线程中止事务后唤醒等待者

---

## 3.20 `src/include/recovery/log_rec.h`

### 需要完成

- `LogRec` 结构体内容
- `CreateInsertLog`
- `CreateDeleteLog`
- `CreateUpdateLog`
- `CreateBeginLog`
- `CreateCommitLog`
- `CreateAbortLog`

### 实现目标

- 建立统一的内存日志记录结构

### 建议字段

- `txn_id`
- `type`
- `lsn`
- `prev_lsn`
- insert/delete/update 所需 old/new key/value

### 测试

- `test/recovery/recovery_manager_test.cpp`

---

## 3.21 `src/include/recovery/recovery_manager.h`

### 需要完成

- `Init`
- `RedoPhase`
- `UndoPhase`

### 实现目标

- 基于 checkpoint 和日志恢复 KV 数据

### 建议逻辑

- `Init`
  - 载入 checkpoint 的持久状态
  - 载入 active txn 表
- `RedoPhase`
  - 从 checkpoint 后按 LSN 重放
  - 更新数据库和活动事务表
- `UndoPhase`
  - 对未提交事务按 `prev_lsn` 回滚

### 测试

- `test/recovery/recovery_manager_test.cpp`

---

## 4. 按测试反推优先级

## 第一批必须先通过

- `test/storage/disk_manager_test.cpp`
- `test/buffer/lru_replacer_test.cpp`
- `test/buffer/buffer_pool_manager_test.cpp`

对应文件：

- `bitmap_page.cpp`
- `disk_manager.cpp`
- `lru_replacer.cpp`
- `buffer_pool_manager.cpp`

## 第二批

- `test/record/tuple_test.cpp`
- `test/storage/table_heap_test.cpp`

对应文件：

- `row.cpp`
- `column.cpp`
- `schema.cpp`
- `table_heap.cpp`
- `table_iterator.cpp`

## 第三批

- `test/index/b_plus_tree_test.cpp`
- `test/index/b_plus_tree_index_test.cpp`
- `test/index/index_iterator_test.cpp`

对应文件：

- `b_plus_tree_page.cpp`
- `b_plus_tree_internal_page.cpp`
- `b_plus_tree_leaf_page.cpp`
- `b_plus_tree.cpp`
- `index_iterator.cpp`

## 第四批

- `test/catalog/catalog_test.cpp`

对应文件：

- `table.cpp`
- `indexes.cpp`
- `catalog.cpp`

## 第五批

- `test/execution/executor_test.cpp`

对应文件：

- `execute_engine.cpp`
- 执行器联调

## 最后批

- `test/recovery/recovery_manager_test.cpp`
- `test/concurrency/lock_manager_test.cpp`

---

## 5. 直接认领模板

你们可以直接照这个表认领：

| 负责人 | 文件 | 目标 | 前置依赖 | 对应测试 |
|---|---|---|---|---|
| A | `bitmap_page.cpp` | 位图页分配回收 | 无 | `disk_manager_test` |
| A | `disk_manager.cpp` | 逻辑页分配与映射 | `bitmap_page` | `disk_manager_test` |
| A | `lru_replacer.cpp` | LRU 淘汰 | 无 | `lru_replacer_test` |
| A | `buffer_pool_manager.cpp` | 缓冲池读写淘汰 | `disk_manager` `lru` | `buffer_pool_manager_test` |
| B | `column.cpp` | 列序列化 | 无 | `tuple_test` |
| B | `schema.cpp` | schema 序列化 | `column` | `tuple_test` |
| B | `row.cpp` | 行序列化 | `field/schema` | `tuple_test` |
| B | `table_heap.cpp` | 堆表增删改查 | `buffer` `row` | `table_heap_test` |
| B | `table_iterator.cpp` | 堆表遍历 | `table_heap` | `table_heap_test` |
| C | `b_plus_tree_page.cpp` | B+树页头行为 | `buffer` | `b_plus_tree_test` |
| C | `b_plus_tree_internal_page.cpp` | 内部节点操作 | `b_plus_tree_page` | `b_plus_tree_test` |
| C | `b_plus_tree_leaf_page.cpp` | 叶子节点操作 | `b_plus_tree_page` | `b_plus_tree_test` |
| C | `b_plus_tree.cpp` | 树插删查 | `internal/leaf page` | `b_plus_tree_test` |
| C | `index_iterator.cpp` | 范围迭代 | `leaf page` | `index_iterator_test` |
| B | `indexes.cpp` | 索引元信息与 Init | `schema` `b+tree` | `catalog_test` |
| B | `catalog.cpp` | 表/索引目录管理 | `heap` `index` | `catalog_test` |
| C | `execute_engine.cpp` | DDL/execfile/quit | `catalog` `executor` | `executor_test` |
| D/任选 | `log_rec.h` | 恢复日志结构 | 主线完成后 | `recovery_manager_test` |
| D/任选 | `recovery_manager.h` | redo/undo | `log_rec` | `recovery_manager_test` |
| D/任选 | `lock_manager.cpp` | 锁与死锁检测 | 主线完成后 | `lock_manager_test` |

---

## 6. 最后建议

最稳的落地方式不是“每人拿一个大模块闷头写完”，而是：

1. 先统一接口理解
2. 每完成一个文件就跑对应单测
3. 每完成一层就联调一次

如果你们下一步真要开写，推荐从下面顺序开始：

1. `bitmap_page.cpp`
2. `disk_manager.cpp`
3. `lru_replacer.cpp`
4. `buffer_pool_manager.cpp`

因为这四个文件没稳，后面所有 bug 都会变成假 bug。

