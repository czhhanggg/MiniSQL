# MiniSQL Record Manager / Planner / Executor 源码导读

这份文档只聚焦你这次完成的两块：

- `record manager`：记录的表示、页内存储、表堆、磁盘页分配
- `planner + executor`：从语法树生成计划，再按 Volcano 模型执行

---

## 1. 先给结论：推荐浏览顺序

如果你想最快建立整体认识，建议按下面顺序看：

1. `test/record/tuple_test.cpp`
2. `src/include/record/field.h`
3. `src/include/record/column.h`
4. `src/include/record/schema.h`
5. `src/include/record/row.h`
6. `src/record/types.cpp`
7. `src/record/column.cpp`
8. `src/record/schema.cpp`
9. `src/record/row.cpp`
10. `src/include/page/page.h`
11. `src/include/page/table_page.h`
12. `src/page/table_page.cpp`
13. `src/include/storage/table_heap.h`
14. `src/storage/table_heap.cpp`
15. `src/include/storage/table_iterator.h`
16. `src/storage/table_iterator.cpp`
17. `src/include/page/bitmap_page.h`
18. `src/page/bitmap_page.cpp`
19. `src/include/storage/disk_manager.h`
20. `src/storage/disk_manager.cpp`
21. `test/storage/table_heap_test.cpp`
22. `src/include/executor/execute_context.h`
23. `src/include/planner/expressions/*.h`
24. `src/include/planner/statement/*.h`
25. `src/include/planner/planner.h`
26. `src/planner/planner.cpp`
27. `src/include/executor/plans/*.h`
28. `src/include/executor/executors/abstract_executor.h`
29. `src/executor/values_executor.cpp`
30. `src/executor/seq_scan_executor.cpp`
31. `src/executor/insert_executor.cpp`
32. `src/executor/delete_executor.cpp`
33. `src/executor/update_executor.cpp`
34. `src/executor/index_scan_executor.cpp`
35. `src/executor/execute_engine.cpp`
36. `test/execution/executor_test.cpp`
37. `test/execution/execute_engine_sql_test.cpp`

如果你只是想抓主线，可以压缩成：

1. `row / schema / field`
2. `table_page`
3. `table_heap + iterator`
4. `planner statement + planner.cpp`
5. `plan nodes`
6. `seq_scan / values / insert / update / delete / index_scan`
7. `execute_engine.cpp`

---

## 2. Record Manager：整体主线

这一块的调用链是：

`Field/Column/Schema/Row`  
-> 定义“一条记录长什么样、怎么序列化”

`TablePage`  
-> 定义“一页里怎么摆放多条记录”

`TableHeap`  
-> 定义“一张表由多个 TablePage 串起来，怎么插删改查”

`TableIterator`  
-> 定义“怎么顺序扫整张表”

`BitmapPage + DiskManager`  
-> 定义“磁盘页怎么分配、释放、映射到文件”

---

## 3. Record 层每个文件干什么

### `src/include/record/field.h`

最底层的数据值对象。

- 表示一个单元格的值，支持 `int / float / char / null`
- 内部维护真实数据、长度、是否为 null、字符串是否由自己管理内存
- 对外暴露：
  - `SerializeTo / DeserializeFrom`
  - `GetSerializedSize`
  - 各种比较操作 `CompareEquals` 等
  - `toString`

可以把它理解成“运行时的值对象”。

### `src/include/record/types.h` + `src/record/types.cpp`

类型系统实现。

- `Type` 是抽象基类
- `TypeInt / TypeFloat / TypeChar` 分别实现：
  - 序列化
  - 反序列化
  - 比较
  - 取长度/取数据
- `Field` 自己不直接处理不同类型，而是委托给 `Type::GetInstance(type_id_)`

关键意义：

- 把“值对象”与“类型行为”拆开
- 所有比较、编码规则都集中在这里

### `src/include/record/column.h` + `src/record/column.cpp`

列定义。

- 保存列名、类型、长度、列下标、是否可空、是否唯一
- 支持列元数据的序列化/反序列化

它描述的是“表结构里的一个列”，不是具体值。

### `src/include/record/schema.h` + `src/record/schema.cpp`

表结构定义。

- 本质上是 `vector<Column *>`
- 支持按列名查列下标
- 支持：
  - `DeepCopySchema`
  - `ShallowCopySchema`
  - 序列化/反序列化

这里特别重要的一点：

- 表的 schema 用于 `Row` 的编码/解码
- 索引也会从表 schema 里裁出一个 key schema

### `src/include/record/row.h` + `src/record/row.cpp`

一条记录。

- 内部保存 `RowId + vector<Field *>`
- 负责按照 schema 把整行序列化到页里
- header 里先写：
  - 字段数
  - null bitmap
- 后面再顺序写非空字段数据

关键函数：

- `SerializeTo`
- `DeserializeFrom`
- `GetSerializedSize`
- `GetKeyFromRow`

`GetKeyFromRow` 很关键，它负责把整行裁成索引键，供索引插入/删除使用。

---

## 4. 页与表：每个文件干什么

### `src/include/page/page.h`

所有页对象的公共基类。

- 持有一页内存 `data_[PAGE_SIZE]`
- 维护 `page_id / pin_count / dirty / latch`
- 提供读写锁接口 `RLatch/WLatch`

这一层不关心“表页还是索引页”，只负责页这个抽象。

### `src/include/page/table_page.h` + `src/page/table_page.cpp`

真正的记录页实现，采用 slotted page。

页格式大意：

- 页头保存：
  - 当前页 id
  - 前后页 id
  - free space pointer
  - tuple count
- 槽目录保存：
  - tuple offset
  - tuple size
- 真正的 tuple 从页尾向前写

关键函数：

- `Init`：初始化页头
- `InsertTuple`：找空槽/追加槽位，写入 tuple
- `MarkDelete`：只打删除标记
- `ApplyDelete`：真正把 tuple 从页内压缩掉
- `RollbackDelete`：撤销删除标记
- `UpdateTuple`：页内原地更新，必要时移动 free space 区域
- `GetTuple`
- `GetFirstTupleRid / GetNextTupleRid`

这是 record manager 的核心文件，建议重点看。

### `src/include/storage/table_heap.h` + `src/storage/table_heap.cpp`

表堆，负责把多个 `TablePage` 串成一张表。

逻辑职责：

- 插入时：
  - 从第一页开始找能放下 tuple 的页
  - 放不下就新建页，挂到链表末尾
- 删除/更新/读取时：
  - 通过 `rid.page_id` 直接定位页
- 顺序扫描时：
  - 从第一页开始找到第一条有效 tuple

关键函数：

- `InsertTuple`
- `MarkDelete`
- `UpdateTuple`
- `ApplyDelete`
- `GetTuple`
- `Begin / End`

它是“表级接口”，对上层 executor 来说，已经不需要关心单页细节。

### `src/include/storage/table_iterator.h` + `src/storage/table_iterator.cpp`

表迭代器。

- 封装顺序扫描状态
- `operator++` 负责：
  - 先在当前页找下一个 tuple
  - 没有的话跳到后继页
  - 再把新 rid 对应的 tuple 读出来

这部分让 `SeqScanExecutor` 可以像 STL iterator 一样扫描表。

### `src/include/page/bitmap_page.h` + `src/page/bitmap_page.cpp`

位图页，负责记录一个 extent 里哪些逻辑页已分配。

- `AllocatePage`
- `DeAllocatePage`
- `IsPageFree`

本质上是磁盘页分配器的底层数据结构。

### `src/include/storage/disk_manager.h` + `src/storage/disk_manager.cpp`

磁盘管理器。

- 提供逻辑页读写接口 `ReadPage / WritePage`
- 提供页分配与回收 `AllocatePage / DeAllocatePage`
- 用 `BitmapPage` 管理 extent 中的空闲页
- 用 `MapPageId` 把逻辑页号映射到物理页号

这里的重点不是 SQL，而是数据库文件布局：

- meta page
- bitmap page
- data pages

### `src/common/instance.cpp` + `src/include/common/instance.h`

数据库实例装配。

- 创建 `DiskManager`
- 创建 `BufferPoolManager`
- 初始化 catalog 相关固定页
- 创建 `CatalogManager`
- 对外提供 `MakeExecuteContext`

可以理解成单个数据库文件的运行时入口。

### `src/include/page/header_page.h/.cpp`、`src/include/page/index_roots_page.h/.cpp`

这两个不属于 record manager 主线，但会在系统里配套出现。

- `HeaderPage`
  - 存 name -> root_id 记录
  - 更像通用目录页
- `IndexRootsPage`
  - 存 `index_id -> root_page_id`
  - 供索引根页管理使用

如果你当前只讲 record manager，这两份可以略读。

---

## 5. Planner：每个文件干什么

planner 的职责不是执行，而是把 parser 生成的 AST 转成：

`AST -> Statement -> PlanNode`

### `src/include/planner/expressions/abstract_expression.h`

表达式基类。

- 所有谓词、常量、列引用都统一成表达式树
- 提供 `Evaluate(row)` 接口

### `column_value_expression.h`

列引用表达式。

- 表示“取某一行的第几列”
- `Evaluate` 时直接从 `Row` 里取字段

### `constant_value_expression.h`

常量表达式。

- 表示 `1`、`"alice"`、`NULL` 这类字面量

### `comparison_expression.h`

比较表达式。

- 表示 `= <> < <= > >= is not`
- 结果返回 `Field(kTypeInt, CmpBool)`

### `logic_expression.h`

逻辑表达式。

- 表示 `AND / OR`
- 把左右子表达式的结果按三值逻辑合并

### `src/include/planner/statement/abstract_statement.h`

Statement 基类。

它做了两件很重要的事：

- 绑定 catalog 上的 schema 信息
- 提供生成表达式树的辅助函数：
  - `MakeColumnValueExpression`
  - `MakeConstantValueExpression`
  - `MakePredicate`

所以它其实承担了半个 binder 的角色。

### `select_statement.h`

把 SELECT 对应的语法树节点转成结构化语义对象。

保存的信息包括：

- `table_name_`
- `column_list_`
- `where_`
- `column_in_condition_`
- `has_or`

这里最关键的是：

- `MakeColumnList` 负责投影列
- `MakePredicate` 负责 where 条件
- 额外统计了 where 中涉及哪些列，供 planner 判断能否走索引

### `insert_statement.h`

把 INSERT 语法树转成：

- 目标表名
- 原始 values 列表 `raw_values_`

后面 `ValuesPlanNode` 就吃这个结果。

### `delete_statement.h`

保存：

- 目标表
- where 条件

### `update_statement.h`

保存：

- 目标表
- where 条件
- `update_attrs`

其中 `update_attrs` 是：

- `列下标 -> 更新表达式`

### `src/include/planner/planner.h` + `src/planner/planner.cpp`

真正的 planner 入口。

核心逻辑：

- `PlanQuery`
  - 根据 AST 类型分发到 `PlanSelect/Insert/Delete/Update`
- `PlanSelect`
  - 先做输出 schema
  - 再看 where 里涉及的列上有没有单列索引
  - 如果没有，或者有 `OR`，就走 `SeqScanPlanNode`
  - 否则走 `IndexScanPlanNode`
- `PlanInsert`
  - `ValuesPlanNode -> InsertPlanNode`
- `PlanDelete`
  - `SeqScanPlanNode -> DeletePlanNode`
- `PlanUpdate`
  - `SeqScanPlanNode -> UpdatePlanNode`

这里的特点很明显：

- planner 很轻量
- 规则型优化只有一个：`SELECT` 时尝试用索引

---

## 6. Plan Node：每个文件干什么

这些文件都在 `src/include/executor/plans/`。

### `abstract_plan.h`

计划节点基类。

- 保存输出 schema
- 保存 children
- 定义 `PlanType`

### `seq_scan_plan.h`

顺序扫描计划。

- 表名
- 可选过滤谓词

### `index_scan_plan.h`

索引扫描计划。

- 表名
- 可用索引列表
- `need_filter_`
- 原始谓词

`need_filter_` 的意思是：

- 虽然命中了部分索引，但不一定完整覆盖所有条件
- 因此 executor 可能还要回表再过滤一次

### `values_plan.h`

常量行计划。

- 保存 INSERT 的 values 列表

### `insert_plan.h`

插入计划。

- 指向子计划
- 目标表名

### `delete_plan.h`

删除计划。

- 指向子计划
- 目标表名

### `update_plan.h`

更新计划。

- 指向子计划
- 目标表名
- `update_attrs`

---

## 7. Executor：每个文件干什么

这一层是：

`PlanNode -> Executor -> Next() 一行一行吐结果`

### `src/include/executor/execute_context.h`

执行上下文。

executor 运行时需要的共享资源都从这里取：

- 当前事务
- catalog
- buffer pool manager

### `src/include/executor/executors/abstract_executor.h`

所有 executor 的共同接口。

- `Init()`
- `Next(Row *, RowId *)`
- `GetOutputSchema()`

典型 Volcano 模型。

### `values_executor.cpp`

最简单的 executor。

- 直接把 `ValuesPlanNode` 里的表达式求值
- 拼成一行行 `Row`
- 供 `InsertExecutor` 消费

### `seq_scan_executor.cpp`

顺序扫描执行器。

流程：

1. `Init` 时拿到 `TableInfo`
2. 从 `TableHeap::Begin()` 拿迭代器
3. `Next` 时逐行扫描
4. 如果有谓词，先 `predicate->Evaluate`
5. 如果输出 schema 与表 schema 不一致，做一次投影拷贝

它对应 SELECT/DELETE/UPDATE 的“找候选行”阶段。

### `insert_executor.cpp`

插入执行器。

流程：

1. 从 child executor 取一行要插入的数据
2. 先检查相关索引是否会冲突
3. 调 `TableHeap::InsertTuple`
4. 插入成功后更新每个索引

关键点：

- 它不自己生成行，数据来源于 child，通常是 `ValuesExecutor`
- 它同时负责“表数据 + 索引数据”的一致更新

### `delete_executor.cpp`

删除执行器。

流程：

1. 从 child executor 拿到要删的行和 rid
2. 调 `TableHeap::MarkDelete`
3. 从所有索引中删掉对应 entry

这里现在做的是“标记删除 + 索引同步删除”。

### `update_executor.cpp`

更新执行器。

流程：

1. 从 child executor 拿到旧行
2. `GenerateUpdatedTuple` 生成新行
3. 调 `TableHeap::UpdateTuple`
4. 删除旧索引项，插入新索引项

`GenerateUpdatedTuple` 的逻辑很清楚：

- 没被更新的列沿用旧值
- 被更新的列用表达式重新求值

### `index_scan_executor.cpp`

索引扫描执行器。

流程：

1. `Init` 时根据谓词先算出所有候选 `RowId`
2. `Next` 时按 `RowId` 回表取 tuple
3. 如果 `need_filter_ == true`，再做一次谓词过滤
4. 最后做投影输出

这里最值得注意的是 `IndexScan()`：

- 比较表达式：对命中的单列索引直接 `ScanKey`
- 逻辑表达式：递归扫描左右子树，再做交集

这个实现说明你这里的 index scan 是“先算 rid 集合，再逐个回表”。

---

## 8. Execute Engine：总调度入口

### `src/include/executor/execute_engine.h`

定义整个 SQL 执行器外观接口。

- 管理多个数据库实例 `dbs_`
- 维护当前数据库 `current_db_`
- 暴露：
  - `Execute(ast)`
  - `ExecutePlan(plan, ...)`

### `src/executor/execute_engine.cpp`

这是总控制器，负责两条主线：

### 8.1 非查询类语句直接在这里处理

包括：

- `create/drop/use/show database`
- `create/drop/show table`
- `create/drop/show index`
- `execfile`
- `quit`

其中你这次补的重点包括：

- `ExecuteCreateTable`
  - 解析列定义
  - 解析主键
  - 组装 `Schema`
  - 调 catalog 创建表
  - 自动给主键和 unique 列补建索引
- `ExecuteDropTable`
- `ExecuteShowIndexes`
- `ExecuteCreateIndex`
- `ExecuteDropIndex`
- `ExecuteExecfile`
- `ExecuteQuit`

### 8.2 DML/查询走 planner + executor

`Execute()` 对 `select/insert/delete/update` 的处理是：

1. 创建 `ExecuteContext`
2. `Planner planner(context)`
3. `planner.PlanQuery(ast)`
4. `ExecutePlan(planner.plan_, ...)`
5. 如果是查询，再格式化输出结果表格

### 8.3 `CreateExecutor`

这是 plan 到 executor 的工厂函数。

映射关系是：

- `SeqScanPlanNode -> SeqScanExecutor`
- `IndexScanPlanNode -> IndexScanExecutor`
- `ValuesPlanNode -> ValuesExecutor`
- `InsertPlanNode -> InsertExecutor`
- `DeletePlanNode -> DeleteExecutor`
- `UpdatePlanNode -> UpdateExecutor`

这段很适合拿来讲“执行器树是怎么递归构建出来的”。

---

## 9. 一条完整主线：以 SQL 为例

### `select * from t1 where id = 1`

执行链：

1. parser 生成 AST
2. `Planner::PlanQuery`
3. `SelectStatement::SyntaxTree2Statement`
4. `MakePredicate` 生成表达式树
5. `Planner::PlanSelect`
6. 选择 `SeqScanPlanNode` 或 `IndexScanPlanNode`
7. `ExecuteEngine::CreateExecutor`
8. `SeqScanExecutor` 或 `IndexScanExecutor`
9. executor 调 `TableHeap`
10. `TableHeap` 调 `TablePage`
11. `TablePage` 反序列化出 `Row`
12. executor 返回结果集

### `insert into t1 values(...)`

执行链：

1. `InsertStatement` 提取原始 values
2. `Planner::PlanInsert`
3. `ValuesPlanNode -> InsertPlanNode`
4. `CreateExecutor`
5. `ValuesExecutor` 产出待插入行
6. `InsertExecutor` 做唯一性检查
7. `TableHeap::InsertTuple`
8. `TablePage::InsertTuple`
9. 成功后插入索引 entry

### `update t1 set name = "bob" where id = 1`

执行链：

1. `UpdateStatement` 保存 `update_attrs`
2. `Planner::PlanUpdate`
3. child 先是 `SeqScanPlanNode`
4. `SeqScanExecutor` 找到旧行
5. `UpdateExecutor::GenerateUpdatedTuple`
6. `TableHeap::UpdateTuple`
7. `TablePage::UpdateTuple`
8. 更新索引

### `delete from t1 where id = 1`

执行链：

1. `DeleteStatement`
2. `Planner::PlanDelete`
3. child `SeqScanPlanNode`
4. `SeqScanExecutor` 找到目标行
5. `DeleteExecutor`
6. `TableHeap::MarkDelete`
7. `TablePage::MarkDelete`
8. 删除索引 entry

---

## 10. 建议你讲解时的组织方式

如果你要给别人讲这两块，建议按这个顺序说：

1. 先讲 record 的数据抽象
   - `Field`
   - `Column`
   - `Schema`
   - `Row`
2. 再讲页内布局
   - `TablePage`
3. 再讲表级管理
   - `TableHeap`
   - `TableIterator`
4. 再讲磁盘页分配
   - `BitmapPage`
   - `DiskManager`
5. 再讲 planner 的中间层
   - `Statement`
   - `Expression`
   - `PlanNode`
6. 最后讲 executor 执行链
   - `SeqScan`
   - `IndexScan`
   - `Insert`
   - `Update`
   - `Delete`
   - `ExecuteEngine`

这样最自然，因为它符合真实调用方向：底层存储 -> 上层执行。

---

## 11. 最值得重点看的几个文件

如果时间不够，只看下面这些：

- `src/record/row.cpp`
- `src/page/table_page.cpp`
- `src/storage/table_heap.cpp`
- `src/planner/planner.cpp`
- `src/include/planner/statement/abstract_statement.h`
- `src/executor/seq_scan_executor.cpp`
- `src/executor/index_scan_executor.cpp`
- `src/executor/insert_executor.cpp`
- `src/executor/update_executor.cpp`
- `src/executor/delete_executor.cpp`
- `src/executor/execute_engine.cpp`

这几份基本能把你这次做的核心逻辑串起来。

---

## 12. 对应测试怎么辅助阅读

- `test/record/tuple_test.cpp`
  - 看 `Field` 和 `Row` 的序列化/反序列化是否正确
- `test/storage/table_heap_test.cpp`
  - 看 `TableHeap` 插入和读取是否正确
- `test/execution/executor_test.cpp`
  - 看每个 executor 单独工作是否正确
- `test/execution/execute_engine_sql_test.cpp`
  - 看整条 SQL 链路是否打通

建议阅读方式：

先看测试在“想验证什么”，再回源码看“它是怎么做到的”。

