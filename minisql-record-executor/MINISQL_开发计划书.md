# MiniSQL 开发计划书

## 1. 文档目的

这份计划书基于两部分信息整理：

1. 语雀项目说明 `#0 ~ #10`
2. 你当前目录下实际拉取的代码框架 `d:\fire wheel\code\minisql-master`

目标不是重复实验指导，而是把“项目要求”映射成“这份代码里具体该改哪些文件、按什么顺序做、怎么分工最稳”。

---

## 2. 当前代码框架结论

### 2.1 代码根目录

当前 MiniSQL 框架位于：

- `d:\fire wheel\code\minisql-master`

### 2.2 代码结构

- `src/`
  - 核心源码
- `src/include/`
  - 头文件和接口定义
- `test/`
  - 模块测试和执行器测试
- `thirdparty/`
  - `gtest`、`glog`

### 2.3 构建和测试方式

根据本地 `README.md` 和 `CMakeLists.txt`：

```bash
mkdir build
cd build
cmake ..
make -j
./test/minisql_test
```

注意：

- 该项目官方说明不支持原生 Windows 编译，推荐 `WSL Ubuntu 20+`
- 所有测试统一编译成 `minisql_test`
- 也支持按单模块测试构建，例如 `make lru_replacer_test`

### 2.4 本地 TODO 热点

本地框架里真正需要你们实现的核心 TODO 集中在这些文件：

- `src/page/bitmap_page.cpp`
- `src/storage/disk_manager.cpp`
- `src/buffer/lru_replacer.cpp`
- `src/buffer/buffer_pool_manager.cpp`
- `src/record/row.cpp`
- `src/record/column.cpp`
- `src/record/schema.cpp`
- `src/storage/table_heap.cpp`
- `src/storage/table_iterator.cpp`
- `src/page/b_plus_tree_page.cpp`
- `src/page/b_plus_tree_internal_page.cpp`
- `src/page/b_plus_tree_leaf_page.cpp`
- `src/index/b_plus_tree.cpp`
- `src/index/index_iterator.cpp`
- `src/catalog/table.cpp`
- `src/catalog/indexes.cpp`
- `src/catalog/catalog.cpp`
- `src/executor/execute_engine.cpp`
- `src/concurrency/lock_manager.cpp`
- `src/include/recovery/log_rec.h`
- `src/include/recovery/recovery_manager.h`

这和语雀章节对应关系基本一致，说明框架和指导文档是对得上的。

---

## 3. 整体项目目标

这个项目本质上是做一个迷你数据库内核，不是单纯写 SQL 语法。

最终你们要交付的是一个可以运行的 MiniSQL 系统，具备：

- 数据库创建、切换、删除
- 表创建、删除、展示
- 记录插入、删除、更新、查询
- 基于 `B+` 树的索引创建、删除、点查、范围查
- 元数据持久化
- 命令行 SQL 执行
- 满足验收演示流程

项目主线应当理解为：

`磁盘与缓冲 -> 记录 -> 索引 -> 目录 -> 执行器`

也就是：

1. `DiskManager / BufferPoolManager`
2. `RecordManager`
3. `IndexManager`
4. `CatalogManager`
5. `Executor / ExecuteEngine`

`Recovery` 和 `LockManager` 是后续扩展模块，应该在主线稳定后再做。

---

## 4. 模块与本地代码映射

### 4.1 模块总览

| 语雀章节 | 模块 | 本地主要文件 | 对应测试 |
|---|---|---|---|
| `#1` | Disk + Buffer Pool | `src/page/bitmap_page.cpp` `src/storage/disk_manager.cpp` `src/buffer/lru_replacer.cpp` `src/buffer/buffer_pool_manager.cpp` | `test/storage/disk_manager_test.cpp` `test/buffer/lru_replacer_test.cpp` `test/buffer/buffer_pool_manager_test.cpp` |
| `#2` | Record Manager | `src/record/*.cpp` `src/storage/table_heap.cpp` `src/storage/table_iterator.cpp` | `test/record/tuple_test.cpp` `test/storage/table_heap_test.cpp` |
| `#3` | Index Manager | `src/page/b_plus_tree_*.cpp` `src/index/b_plus_tree.cpp` `src/index/index_iterator.cpp` | `test/index/b_plus_tree_test.cpp` `test/index/b_plus_tree_index_test.cpp` `test/index/index_iterator_test.cpp` |
| `#4` | Catalog Manager | `src/catalog/table.cpp` `src/catalog/indexes.cpp` `src/catalog/catalog.cpp` | `test/catalog/catalog_test.cpp` |
| `#5` | Executor | `src/executor/*.cpp` `src/executor/execute_engine.cpp` | `test/execution/executor_test.cpp` |
| `#6` | Recovery Manager | `src/include/recovery/log_rec.h` `src/include/recovery/recovery_manager.h` | `test/recovery/recovery_manager_test.cpp` |
| `#7` | Lock Manager | `src/concurrency/lock_manager.cpp` | `test/concurrency/lock_manager_test.cpp` |

---

## 5. 每个模块该做什么，应该怎么实现

## 5.1 模块一：Disk Manager 与 Buffer Pool Manager

### 功能目标

这一层负责“页”的生命周期管理，是整个数据库最底层。

需要实现：

- 位图页分配与回收
- 逻辑页号和物理页号映射
- 磁盘页读写
- 缓冲池页获取与淘汰
- `LRU` 替换
- 页的 `pin/unpin`、脏页刷新

### 本地文件

- `src/page/bitmap_page.cpp`
- `src/storage/disk_manager.cpp`
- `src/buffer/lru_replacer.cpp`
- `src/buffer/buffer_pool_manager.cpp`

### 应实现的核心接口

#### `BitmapPage`

- `AllocatePage`
- `DeAllocatePage`
- `IsPageFree`

实现建议：

- 用位操作管理空闲页
- 维护页内元信息，例如：
  - 已分配页数
  - 下一个可尝试空闲位
- 先保证正确性，再做局部优化

#### `DiskManager`

- `AllocatePage`
- `DeAllocatePage`
- `IsPageFree`
- `MapPageId` 相关逻辑

实现建议：

- 先完全理解这层页布局：
  - `Meta Page`
  - `Bitmap Page`
  - 数据页区
- 把“逻辑页号”和“物理页号”转换公式单独写清楚
- `ReadPage/WritePage` 依赖映射逻辑是否正确
- 这里最容易错的是：
  - 页号偏移
  - extent 边界
  - meta page 更新时机

#### `LRUReplacer`

- `Victim`
- `Pin`
- `Unpin`
- `Size`

实现建议：

- 标准做法：`list + hash`
- 需要保证：
  - 一个 frame 不能重复进入可淘汰队列
  - 被 pin 的 frame 必须从 LRU 中移除
  - `Victim` 取“最久未使用”

#### `BufferPoolManager`

- `FetchPage`
- `NewPage`
- `UnpinPage`
- `FlushPage`
- `DeletePage`

实现建议：

- 典型流程：
  1. 先查 `page_table_`
  2. 不在内存则找 `free_list_` 或 `replacer`
  3. 若 victim 是脏页，先刷盘
  4. 载入新页
  5. 更新 `page_table_`、`pin_count_`、`is_dirty_`
- 建议单独写一个“取可用 frame”的辅助逻辑，避免重复
- 这个模块是后续全部模块的地基，必须先测稳

### 完成标准

- 通过：
  - `disk_manager_test`
  - `lru_replacer_test`
  - `buffer_pool_manager_test`

### 风险点

- 页号映射错一位，后面所有模块都会诡异失败
- `Unpin` 和脏页状态维护错误，会导致 B+ 树和堆表测试连环出错

---

## 5.2 模块二：Record Manager

### 功能目标

这一层负责“记录/元组”的表示和堆表存储。

需要实现：

- `Row / Column / Schema` 序列化与反序列化
- `TableHeap` 插入、更新、删除、读取
- `TableIterator` 遍历

### 本地文件

- `src/record/row.cpp`
- `src/record/column.cpp`
- `src/record/schema.cpp`
- `src/storage/table_heap.cpp`
- `src/storage/table_iterator.cpp`

### 应实现的核心接口

#### 对象序列化

需要先完成：

- `Row::SerializeTo / DeserializeFrom / GetSerializedSize`
- `Column::SerializeTo / DeserializeFrom / GetSerializedSize`
- `Schema::SerializeTo / DeserializeFrom / GetSerializedSize`

实现建议：

- 严格按“写入顺序 = 读取顺序”
- `Row` 里注意 `NULL bitmap`
- `char(n)` 字段长度处理要一致
- 这里先把 tuple 相关测试跑通，再碰堆表

#### `TableHeap`

重点接口：

- `InsertTuple`
- `UpdateTuple`
- `ApplyDelete`
- `GetTuple`
- `Begin`
- `End`
- `DeleteTable`

实现建议：

- 插入：
  - 从第一页开始做 `first fit`
  - 找不到能容纳的页就分配新页
- 更新：
  - 若页内空间足够，原位更新
  - 若空间不足，可以先按实验要求返回失败，交给上层删除再插入
- 删除：
  - 分清逻辑删除和物理删除
- 读取：
  - 根据 `rid(page_id, slot_num)` 精确定位

#### `TableIterator`

实现建议：

- 迭代器内部至少要知道：
  - 当前 `TableHeap`
  - 当前 `RowId`
  - 当前事务指针
- `operator++` 需要支持：
  - 同页找下一个有效 tuple
  - 到页尾后跳到下一页
- `End()` 一般对应无效 `rid`

### 完成标准

- 通过：
  - `tuple_test`
  - `table_heap_test`

### 风险点

- 内存释放和对象所有权
- `Row` 反序列化后字段对象归属
- `UpdateTuple` 返回语义不清晰，注意和语雀说明保持一致

---

## 5.3 模块三：Index Manager

### 功能目标

这一层实现磁盘版 `B+` 树索引，是验收性能差异的核心。

需要实现：

- B+ 树页结构
- 叶子页 / 内部页操作
- 插入、删除、查找
- 分裂、合并、借位
- 索引迭代器

### 本地文件

- `src/page/b_plus_tree_page.cpp`
- `src/page/b_plus_tree_internal_page.cpp`
- `src/page/b_plus_tree_leaf_page.cpp`
- `src/index/b_plus_tree.cpp`
- `src/index/index_iterator.cpp`

### 应实现的核心接口

#### B+ 树页

- `BPlusTreePage`
- `BPlusTreeInternalPage`
- `BPlusTreeLeafPage`

实现建议：

- 先把页内数组操作写稳：
  - 插入
  - 删除
  - 查找位置
  - 移动半边元素
- 内部节点第一个 key 无效，这个细节一定别忘
- `GetMinSize()` 要考虑：
  - 根节点
  - 叶子节点
  - 非叶子节点

#### `BPlusTree`

重点接口：

- `Insert`
- `Remove`
- `GetValue`
- `FindLeafPage`
- `Begin/End`
- `Split`
- `InsertIntoParent`
- `CoalesceOrRedistribute`
- `AdjustRoot`

实现建议：

- 不要一口气写完整树
- 正确顺序应该是：
  1. 单叶子节点查找
  2. 单叶子节点插入
  3. 叶子分裂
  4. 内部节点插入
  5. 删除
  6. 合并与借位
  7. 根缩减
- 每一步都跑测试

#### `IndexIterator`

实现建议：

- 依赖叶子页链表
- 范围扫描本质上就是：
  - 从某叶子页某位置开始
  - 顺着 `next_page_id` 走

### 完成标准

- 通过：
  - `b_plus_tree_test`
  - `b_plus_tree_index_test`
  - `index_iterator_test`

### 风险点

- `Unpin` 忘记做
- 插入分裂后父指针更新不完整
- 删除合并时父节点 key 同步错误
- 根页号持久化忘记更新

---

## 5.4 模块四：Catalog Manager

### 功能目标

这一层负责数据库模式信息，也就是表和索引的“目录”。

需要实现：

- 表元数据持久化
- 索引元数据持久化
- 表和索引对象恢复
- 向执行层提供查询接口

### 本地文件

- `src/catalog/table.cpp`
- `src/catalog/indexes.cpp`
- `src/catalog/catalog.cpp`

### 应实现的核心接口

#### 元数据尺寸与初始化

- `TableMetadata::GetSerializedSize`
- `IndexMetadata::GetSerializedSize`
- `CatalogMeta::GetSerializedSize`
- `IndexInfo::Init`

实现建议：

- 这里不是纯算法题，重点是对象重建
- `IndexInfo::Init` 关键流程：
  1. 绑定 `meta_data`
  2. 绑定 `table_info`
  3. 根据 `key_map` 从表 schema 映射出 `key_schema`
  4. 创建具体索引对象

#### `CatalogManager`

重点接口：

- `CreateTable`
- `GetTable`
- `GetTables`
- `CreateIndex`
- `GetIndex`
- `GetTableIndexes`
- `DropTable`
- `DropIndex`

实现建议：

- 内存索引结构建议至少维护：
  - `table_names_ -> table_id`
  - `tables_ -> TableInfo`
  - `index_names_`
  - `indexes_`
- 构造函数里要区分：
  - `init = true`：新建数据库
  - `init = false`：从 catalog meta page 恢复
- drop 操作要同步删除：
  - 内存对象
  - catalog meta entry
  - 对应页

### 完成标准

- 通过：
  - `catalog_test`

### 风险点

- `Schema` 深拷贝 / 浅拷贝错误，容易二次析构
- 只更新了内存，没有持久化

---

## 5.5 模块五：Planner 与 Executor

### 功能目标

这部分决定“SQL 能不能真正执行”。

语雀说明里已经明确：

- `Planner` 核心框架基本已给
- 你们主要需要补的是执行器和 `ExecuteEngine` 的 DDL/命令处理

### 本地文件

- `src/executor/seq_scan_executor.cpp`
- `src/executor/index_scan_executor.cpp`
- `src/executor/insert_executor.cpp`
- `src/executor/update_executor.cpp`
- `src/executor/delete_executor.cpp`
- `src/executor/values_executor.cpp`
- `src/executor/execute_engine.cpp`

### 应实现的核心接口

#### 五类执行器

- `SeqScanExecutor`
- `IndexScanExecutor`
- `InsertExecutor`
- `UpdateExecutor`
- `DeleteExecutor`

实现建议：

- `SeqScan`
  - 遍历 `TableHeap`
  - 对每条记录套 `predicate`
- `IndexScan`
  - 从 index 拿到 `rid`
  - 再回表取 tuple
- `Insert`
  - 插表
  - 维护相关索引
- `Update`
  - 找到旧记录
  - 改记录
  - 同步更新所有相关索引
- `Delete`
  - 删除记录
  - 从相关索引中删 key

#### `ExecuteEngine`

本地明确留空的主要函数：

- `ExecuteCreateTable`
- `ExecuteDropTable`
- `ExecuteShowIndexes`
- `ExecuteCreateIndex`
- `ExecuteDropIndex`
- `ExecuteExecfile`
- `ExecuteQuit`

实现建议：

- `CreateTable`
  - 从 AST 提取表名、列定义、主键、unique
  - 组装 `Column` 和 `Schema`
  - 调 `CatalogManager::CreateTable`
- `CreateIndex`
  - 从 AST 提取表名、索引名、列名
  - 调 `CatalogManager::CreateIndex`
- `Execfile`
  - 逐行读取 SQL 文件并复用 `Execute`
- `Quit`
  - 返回 `DB_QUIT`

### 验收相关重点

这部分直接决定是否能完成语雀 `#9` 里的演示：

- `create database`
- `use`
- `create table`
- `insert`
- `select`
- `update`
- `delete`
- `create index`
- `drop index`
- `drop table`

### 完成标准

- 通过：
  - `executor_test`
- 并能在 `main` 里手动执行 SQL

### 风险点

- 只改表不改索引
- `update/delete` 对索引维护不一致
- `execfile` 没处理好多条 SQL 的执行输出

---

## 5.6 模块六：Recovery Manager

### 功能目标

这是内存版恢复，不是完整 WAL 系统。

需要实现：

- 日志结构 `LogRec`
- `Init`
- `RedoPhase`
- `UndoPhase`

### 本地文件

- `src/include/recovery/log_rec.h`
- `src/include/recovery/recovery_manager.h`

### 实现建议

- 先把日志类型设计全：
  - `BEGIN`
  - `COMMIT`
  - `ABORT`
  - `INSERT`
  - `DELETE`
  - `UPDATE`
- `RedoPhase`
  - 从 checkpoint 后按 LSN 顺序重放
- `UndoPhase`
  - 对活跃未提交事务逆序回滚
- 因为这部分是内存 KV 模拟，重点在状态机，不在页级存储

### 完成标准

- 通过：
  - `recovery_manager_test`

### 建议优先级

- 主线完成后再做

---

## 5.7 模块七：Lock Manager

### 功能目标

这是并发 Bonus，目标是支持事务加锁与死锁检测。

### 本地文件

- `src/concurrency/lock_manager.cpp`

### 应实现内容

- `LockShared`
- `LockExclusive`
- `LockUpgrade`
- `Unlock`
- `LockPrepare`
- `CheckAbort`
- `AddEdge`
- `RemoveEdge`
- `HasCycle`
- `GetEdgeList`
- `RunCycleDetection`

### 实现建议

- 先做“无死锁检测”的锁管理
- 再补后台等待图检测
- 必须严格按确定性顺序做 DFS
- 被中止事务要正确唤醒等待线程

### 完成标准

- 通过：
  - `lock_manager_test`

### 建议优先级

- 最后做

---

## 6. 推荐开发顺序

这是最重要的一节。顺序错了，团队会反复返工。

### 第一阶段：环境和基座

1. 在 `WSL` 跑通编译
2. 跑通所有现有测试，确认基线
3. 实现 `BitmapPage`
4. 实现 `DiskManager`
5. 实现 `LRUReplacer`
6. 实现 `BufferPoolManager`

原因：

- 这一步不稳定，后面全白做

### 第二阶段：记录层

1. 实现 `Column / Schema / Row` 序列化
2. 实现 `TableHeap`
3. 实现 `TableIterator`

原因：

- 没有堆表，就没法给索引和执行器提供真实数据

### 第三阶段：索引层

1. 完成 B+ 树页结构
2. 完成查找
3. 完成插入
4. 完成分裂
5. 完成删除与合并
6. 完成 `IndexIterator`

原因：

- 这是验收中“性能差异”的主要来源

### 第四阶段：目录层

1. 实现元数据序列化尺寸
2. 实现 `IndexInfo::Init`
3. 实现 `CatalogManager`

原因：

- 执行器需要它提供表和索引信息

### 第五阶段：执行层

1. `SeqScanExecutor`
2. `InsertExecutor`
3. `DeleteExecutor`
4. `UpdateExecutor`
5. `IndexScanExecutor`
6. `ExecuteEngine` 的 DDL / `execfile` / `quit`

原因：

- 到这一步系统才能从“模块测试通过”变成“能演示”

### 第六阶段：增强模块

1. `RecoveryManager`
2. `LockManager`
3. 性能优化
4. 验收脚本和答辩准备

---

## 7. 详细里程碑

## 7.1 五阶段里程碑

### 里程碑 M0：环境就绪

- 能在 `WSL` 编译
- 能跑 `minisql_test`
- 团队统一代码风格和分支策略

### 里程碑 M1：底层页管理完成

- `disk_manager_test`
- `lru_replacer_test`
- `buffer_pool_manager_test`

### 里程碑 M2：记录层完成

- `tuple_test`
- `table_heap_test`

### 里程碑 M3：索引和目录完成

- `b_plus_tree_test`
- `b_plus_tree_index_test`
- `index_iterator_test`
- `catalog_test`

### 里程碑 M4：SQL 执行完成

- `executor_test`
- `main` 交互执行基本 SQL

### 里程碑 M5：验收可演示

- 能按语雀 `#9` 流程走完
- 支持批量导入 SQL 文件
- 支持索引前后耗时对比

---

## 8. 合理分工方案

语雀建议 `1~3` 人一组。这里给最合理的 3 人方案，同时给 2 人备选。

## 8.1 三人方案，推荐

### 成员 A：存储与缓冲负责人

负责：

- `BitmapPage`
- `DiskManager`
- `LRUReplacer`
- `BufferPoolManager`
- 统一处理页号、刷盘、pin/unpin 问题

对应文件：

- `src/page/bitmap_page.cpp`
- `src/storage/disk_manager.cpp`
- `src/buffer/lru_replacer.cpp`
- `src/buffer/buffer_pool_manager.cpp`

对应测试：

- `disk_manager_test`
- `lru_replacer_test`
- `buffer_pool_manager_test`

要求：

- 必须给组内其他人提供稳定可用的底层接口

### 成员 B：记录与目录负责人

负责：

- `Row / Column / Schema`
- `TableHeap / TableIterator`
- `CatalogManager`
- 表和索引元数据恢复

对应文件：

- `src/record/*.cpp`
- `src/storage/table_heap.cpp`
- `src/storage/table_iterator.cpp`
- `src/catalog/*.cpp`

对应测试：

- `tuple_test`
- `table_heap_test`
- `catalog_test`

要求：

- 提供稳定的表存储和元数据接口给索引层、执行层

### 成员 C：索引与执行负责人

负责：

- `B+` 树
- `IndexIterator`
- 各类 Executor
- `ExecuteEngine`
- 验收 SQL 流程联调

对应文件：

- `src/page/b_plus_tree_*.cpp`
- `src/index/*.cpp`
- `src/executor/*.cpp`

对应测试：

- `b_plus_tree_test`
- `b_plus_tree_index_test`
- `index_iterator_test`
- `executor_test`

要求：

- 最后承担系统整合和验收脚本准备

### 三人协作原则

- A 先交付稳定 BufferPool 接口
- B 和 C 早期就要对齐：
  - `RowId`
  - `Schema`
  - 索引键映射
- C 不要等所有模块全部结束再接入，应尽早做最小链路联调

## 8.2 两人方案，备选

### 成员 A

- 模块 `#1 + #2`
- 即：
  - 存储层
  - 记录层

### 成员 B

- 模块 `#3 + #4 + #5`
- 即：
  - 索引层
  - 目录层
  - 执行层

### 两人方案风险

- B 压力会明显更大
- 建议放弃 `Recovery` 和 `LockManager`，优先主线验收

---

## 9. 每周/每阶段建议安排

如果按比较稳的节奏推进，建议这样安排：

### 第 1 阶段

- 拉代码
- 跑通构建
- 看测试
- 完成模块 `#1`

### 第 2 阶段

- 完成模块 `#2`
- 保证堆表能稳定插入和遍历

### 第 3 阶段

- 完成模块 `#3`
- 至少先跑通插入和点查

### 第 4 阶段

- 完成模块 `#4`
- 表和索引元数据可持久化恢复

### 第 5 阶段

- 完成模块 `#5`
- 跑通核心 SQL

### 第 6 阶段

- 跑验收流程
- 修 bug
- 录入性能对比
- 准备答辩

如果时间很紧，必须砍掉的顺序是：

1. `LockManager`
2. `RecoveryManager`
3. 额外优化项

不能砍的是：

1. `#1`
2. `#2`
3. `#3`
4. `#4`
5. `#5`

---

## 10. 联调策略

不要等所有人都“自认为写完”再联调。

推荐联调顺序：

1. 先联调 `BufferPool + Disk`
2. 再联调 `TableHeap` 插入与读取
3. 再联调 `B+树` 插入与查找
4. 再联调 `Catalog`
5. 最后联调 `Executor`

每次联调只解决一层问题，不要多层一起查。

例如：

- `select` 查不到数据，不要先怀疑 parser
- 先查：
  - 表里有没有插进去
  - `rid` 是否正确
  - 索引是否更新
  - catalog 是否映射对

---

## 11. 验收准备建议

语雀 `#9` 的验收流程说明得很清楚，建议单独准备：

- 一个标准数据库名
- 一个标准 `account` 表
- 一份批量插入 `sql.txt`
- 一组固定查询语句
- 一组索引前后耗时对比

至少要确保这些命令稳定：

- `create database`
- `use`
- `create table`
- `insert`
- `select *`
- 条件查询
- `create index`
- `drop index`
- `update`
- `delete`
- `drop table`

建议额外做一个“验收脚本清单”，现场照单演示，不要临场现想。

---

## 12. 最终建议

### 最稳的技术路线

1. 先做 `#1`
2. 再做 `#2`
3. 再做 `#3`
4. 再做 `#4`
5. 最后做 `#5`

### 最常见错误

- 上来就写执行器
- B+ 树没写稳就强接 catalog
- 不看测试直接凭感觉写
- 忘记 `UnpinPage`
- 只做功能，不做索引同步
- 直到最后一周才联调

### 最现实的目标

如果你们目标是稳过验收，优先级应当是：

1. 主线模块全部可运行
2. SQL 基本功能完整
3. 索引查询明显快于全表扫描
4. 演示流程稳定

Bonus 模块不是当前第一优先级。

---

## 13. 附：建议任务认领表

| 优先级 | 模块 | 负责人 | 开始条件 | 完成条件 |
|---|---|---|---|---|
| P0 | 环境与测试基线 | 全员 | 拉取代码 | 能编译、能跑测试 |
| P1 | Disk + Buffer | A | 环境完成 | 3 个底层测试通过 |
| P1 | Row/Schema/Heap | B | Buffer 基本可用 | tuple + table_heap 测试通过 |
| P1 | B+Tree | C | Buffer 可用，RowId 明确 | 3 个索引测试通过 |
| P2 | Catalog | B | Heap 与 Index 稳定 | catalog_test 通过 |
| P2 | Executor | C | Catalog 可用 | executor_test 通过 |
| P3 | 验收 SQL 流程 | 全员 | Executor 可跑 | 现场演示命令稳定 |
| P4 | Recovery | 任选 | 主线完成 | recovery 测试通过 |
| P4 | LockManager | 任选 | 主线完成 | lock_manager 测试通过 |

---

如果后续继续推进，建议下一步直接做两件事：

1. 生成一个“按文件逐个 TODO 的任务拆解表”
2. 从 `#1 Disk + Buffer` 开始正式实现

