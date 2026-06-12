# TableHeap / TablePage Bonus Implementation

## What changed

This bonus version optimizes `TableHeap` and `TablePage` by adding lightweight metadata for:

1. faster slot reuse inside a page
2. faster lookup of the first / next live tuple
3. faster page selection during insertion

The original project is kept untouched. The modified version lives in:

`d:\fire wheel\code\minisql-record-executor-bonus`

## TablePage metadata

`TablePage` keeps two 512-bit bitmaps in the page header:

- `occupied bitmap`: whether a slot currently stores tuple bytes
- `deleted bitmap`: whether that slot is logically deleted

It also stores:

- `live_tuple_count`
- `prev_insertable_page_id`
- `next_insertable_page_id`

These fields are used to avoid repeated linear scans over slot metadata.

## Why this helps

### Insert

- find a reusable empty slot from the bitmap instead of scanning every slot
- if no reusable slot exists, append a new slot as before

### Find / iterate

- `GetFirstTupleRid` finds the first live slot through bitmap scanning
- `GetNextTupleRid` jumps to the next live slot through bitmap scanning

### Delete

- logical delete only flips the deleted bit and decrements `live_tuple_count`
- physical delete clears the occupied/deleted bits and makes the slot immediately reusable

## TableHeap metadata

`TableHeap` now keeps:

- `last_page_id_`
- `insertable_head_page_id_`
- a doubly linked list of insertable pages via metadata stored in each `TablePage`

This changes insertion from:

- scan the whole heap page chain

to:

- scan only pages that can still host at least one more tuple

If no existing page works, append a new page directly through `last_page_id_`.

## Validation

Built and tested in:

- `build-bonus-mingw4`

Passed:

- `tuple_test`
- `table_heap_test`

Additional tests were added for:

- skipping deleted slots during page-level navigation
- reusing a physically deleted slot during heap insertion
