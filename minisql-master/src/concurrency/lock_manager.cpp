#include "concurrency/lock_manager.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>

#include "common/rowid.h"
#include "concurrency/txn.h"
#include "concurrency/txn_manager.h"

void LockManager::SetTxnMgr(TxnManager *txn_mgr) { txn_mgr_ = txn_mgr; }

/**
 * TODO: Student Implement
 */
bool LockManager::LockShared(Txn *txn, const RowId &rid) {
    if (txn->GetIsolationLevel() == IsolationLevel::kReadUncommitted) {
        txn->SetState(TxnState::kAborted);
        throw TxnAbortException(txn->GetTxnId(), AbortReason::kLockSharedOnReadUncommitted);
    }
    LockPrepare(txn, rid);
    std::unique_lock<std::mutex> lock(latch_);
    if (txn->GetSharedLockSet().count(rid) != 0 || txn->GetExclusiveLockSet().count(rid) != 0) {
        return true;
    }
    auto &req_queue = lock_table_[rid];
    req_queue.EmplaceLockRequest(txn->GetTxnId(), LockMode::kShared);
    auto req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    while (req_queue.is_writing_ || req_queue.is_upgrading_) {
        req_queue.cv_.wait(lock);
        CheckAbort(txn, req_queue);
        if (req_queue.req_list_iter_map_.find(txn->GetTxnId()) == req_queue.req_list_iter_map_.end()) {
            return false;
        }
        req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    }
    req_iter->granted_ = LockMode::kShared;
    req_queue.sharing_cnt_++;
    txn->GetSharedLockSet().emplace(rid);
    return true;
}

/**
 * TODO: Student Implement
 */
bool LockManager::LockExclusive(Txn *txn, const RowId &rid) {
    LockPrepare(txn, rid);
    std::unique_lock<std::mutex> lock(latch_);
    if (txn->GetExclusiveLockSet().count(rid) != 0) {
        return true;
    }
    if (txn->GetSharedLockSet().count(rid) != 0) {
        lock.unlock();
        return LockUpgrade(txn, rid);
    }
    auto &req_queue = lock_table_[rid];
    req_queue.EmplaceLockRequest(txn->GetTxnId(), LockMode::kExclusive);
    auto req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    while (req_queue.is_writing_ || req_queue.sharing_cnt_ > 0) {
        req_queue.cv_.wait(lock);
        CheckAbort(txn, req_queue);
        if (req_queue.req_list_iter_map_.find(txn->GetTxnId()) == req_queue.req_list_iter_map_.end()) {
            return false;
        }
        req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    }
    req_iter->granted_ = LockMode::kExclusive;
    req_queue.is_writing_ = true;
    txn->GetExclusiveLockSet().emplace(rid);
    return true;
}

/**
 * TODO: Student Implement
 */
bool LockManager::LockUpgrade(Txn *txn, const RowId &rid) {
    LockPrepare(txn, rid);
    std::unique_lock<std::mutex> lock(latch_);
    if (txn->GetExclusiveLockSet().count(rid) != 0) {
        return true;
    }
    if (txn->GetSharedLockSet().count(rid) == 0) {
        return false;
    }
    auto &req_queue = lock_table_[rid];
    if (req_queue.is_upgrading_) {
        txn->SetState(TxnState::kAborted);
        throw TxnAbortException(txn->GetTxnId(), AbortReason::kUpgradeConflict);
    }
    req_queue.is_upgrading_ = true;
    auto req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    req_iter->lock_mode_ = LockMode::kExclusive;
    while (req_queue.is_writing_ || req_queue.sharing_cnt_ > 1) {
        req_queue.cv_.wait(lock);
        CheckAbort(txn, req_queue);
        if (req_queue.req_list_iter_map_.find(txn->GetTxnId()) == req_queue.req_list_iter_map_.end()) {
            return false;
        }
        req_iter = req_queue.GetLockRequestIter(txn->GetTxnId());
    }
    req_queue.sharing_cnt_--;
    req_queue.is_writing_ = true;
    req_queue.is_upgrading_ = false;
    req_iter->granted_ = LockMode::kExclusive;
    txn->GetSharedLockSet().erase(rid);
    txn->GetExclusiveLockSet().emplace(rid);
    req_queue.cv_.notify_all();
    return true;
}

/**
 * TODO: Student Implement
 */
bool LockManager::Unlock(Txn *txn, const RowId &rid) {
    std::unique_lock<std::mutex> lock(latch_);
    auto table_iter = lock_table_.find(rid);
    if (table_iter == lock_table_.end()) {
        return false;
    }
    auto &req_queue = table_iter->second;
    auto req_map_iter = req_queue.req_list_iter_map_.find(txn->GetTxnId());
    if (req_map_iter == req_queue.req_list_iter_map_.end()) {
        return false;
    }
    auto req_iter = req_map_iter->second;
    if (req_iter->granted_ == LockMode::kShared) {
        req_queue.sharing_cnt_--;
        txn->GetSharedLockSet().erase(rid);
    } else if (req_iter->granted_ == LockMode::kExclusive) {
        req_queue.is_writing_ = false;
        txn->GetExclusiveLockSet().erase(rid);
    }
    if (req_iter->lock_mode_ == LockMode::kExclusive && req_iter->granted_ == LockMode::kShared) {
        req_queue.is_upgrading_ = false;
    }
    req_queue.EraseLockRequest(txn->GetTxnId());
    if (txn->GetState() == TxnState::kGrowing) {
        txn->SetState(TxnState::kShrinking);
    }
    req_queue.cv_.notify_all();
    return true;
}

/**
 * TODO: Student Implement
 */
void LockManager::LockPrepare(Txn *txn, const RowId &rid) {
    if (txn->GetState() == TxnState::kAborted) {
        throw TxnAbortException(txn->GetTxnId(), AbortReason::kDeadlock);
    }
    if (txn->GetState() == TxnState::kShrinking) {
        txn->SetState(TxnState::kAborted);
        throw TxnAbortException(txn->GetTxnId(), AbortReason::kLockOnShrinking);
    }
}

/**
 * TODO: Student Implement
 */
void LockManager::CheckAbort(Txn *txn, LockManager::LockRequestQueue &req_queue) {
    if (txn->GetState() != TxnState::kAborted) {
        return;
    }
    auto iter = req_queue.req_list_iter_map_.find(txn->GetTxnId());
    if (iter != req_queue.req_list_iter_map_.end()) {
        if (iter->second->granted_ == LockMode::kNone) {
            req_queue.EraseLockRequest(txn->GetTxnId());
        } else if (iter->second->lock_mode_ == LockMode::kExclusive && iter->second->granted_ == LockMode::kShared) {
            req_queue.is_upgrading_ = false;
        }
    }
    req_queue.cv_.notify_all();
    throw TxnAbortException(txn->GetTxnId(), AbortReason::kDeadlock);
}

/**
 * TODO: Student Implement
 */
void LockManager::AddEdge(txn_id_t t1, txn_id_t t2) {
    waits_for_[t1].insert(t2);
}

/**
 * TODO: Student Implement
 */
void LockManager::RemoveEdge(txn_id_t t1, txn_id_t t2) {
    auto iter = waits_for_.find(t1);
    if (iter == waits_for_.end()) {
        return;
    }
    iter->second.erase(t2);
    if (iter->second.empty()) {
        waits_for_.erase(iter);
    }
}

/**
 * TODO: Student Implement
 */
bool LockManager::HasCycle(txn_id_t &newest_tid_in_cycle) {
    newest_tid_in_cycle = INVALID_TXN_ID;
    std::set<txn_id_t> nodes;
    for (const auto &edge: waits_for_) {
        nodes.insert(edge.first);
        for (auto to: edge.second) {
            nodes.insert(to);
        }
    }
    std::unordered_map<txn_id_t, int> state;
    std::vector<txn_id_t> path;
    std::function<bool(txn_id_t)> dfs = [&](txn_id_t node) {
        state[node] = 1;
        path.emplace_back(node);
        auto iter = waits_for_.find(node);
        if (iter != waits_for_.end()) {
            for (auto next: iter->second) {
                if (state[next] == 0) {
                    if (dfs(next)) {
                        return true;
                    }
                } else if (state[next] == 1) {
                    auto pos = std::find(path.begin(), path.end(), next);
                    newest_tid_in_cycle = next;
                    for (; pos != path.end(); ++pos) {
                        newest_tid_in_cycle = std::max(newest_tid_in_cycle, *pos);
                    }
                    return true;
                }
            }
        }
        path.pop_back();
        state[node] = 2;
        return false;
    };
    for (auto node: nodes) {
        if (state[node] == 0 && dfs(node)) {
            return true;
        }
    }
    return false;
}

void LockManager::DeleteNode(txn_id_t txn_id) {
    waits_for_.erase(txn_id);

    auto *txn = txn_mgr_->GetTransaction(txn_id);
    if (txn == nullptr) {
        return;
    }

    for (const auto &row_id: txn->GetSharedLockSet()) {
        for (const auto &lock_req: lock_table_[row_id].req_list_) {
            if (lock_req.granted_ == LockMode::kNone) {
                RemoveEdge(lock_req.txn_id_, txn_id);
            }
        }
    }

    for (const auto &row_id: txn->GetExclusiveLockSet()) {
        for (const auto &lock_req: lock_table_[row_id].req_list_) {
            if (lock_req.granted_ == LockMode::kNone) {
                RemoveEdge(lock_req.txn_id_, txn_id);
            }
        }
    }
}

/**
 * TODO: Student Implement
 */
void LockManager::RunCycleDetection() {
    while (enable_cycle_detection_) {
        std::this_thread::sleep_for(cycle_detection_interval_);
        std::unique_lock<std::mutex> lock(latch_);
        waits_for_.clear();
        for (auto &lock_entry: lock_table_) {
            auto &req_queue = lock_entry.second;
            for (auto &wait_req: req_queue.req_list_) {
                bool waiting = wait_req.granted_ == LockMode::kNone || wait_req.lock_mode_ != wait_req.granted_;
                if (!waiting) {
                    continue;
                }
                for (auto &hold_req: req_queue.req_list_) {
                    if (hold_req.txn_id_ == wait_req.txn_id_ || hold_req.granted_ == LockMode::kNone) {
                        continue;
                    }
                    bool conflict = wait_req.lock_mode_ == LockMode::kExclusive ||
                                    hold_req.granted_ == LockMode::kExclusive;
                    if (conflict) {
                        AddEdge(wait_req.txn_id_, hold_req.txn_id_);
                    }
                }
            }
        }
        txn_id_t victim = INVALID_TXN_ID;
        while (HasCycle(victim)) {
            Txn *txn = txn_mgr_->GetTransaction(victim);
            if (txn != nullptr) {
                txn->SetState(TxnState::kAborted);
            }
            DeleteNode(victim);
            victim = INVALID_TXN_ID;
        }
        for (auto &lock_entry: lock_table_) {
            lock_entry.second.cv_.notify_all();
        }
    }
}

/**
 * TODO: Student Implement
 */
std::vector<std::pair<txn_id_t, txn_id_t>> LockManager::GetEdgeList() {
    std::vector<std::pair<txn_id_t, txn_id_t>> result;
    for (const auto &edge: waits_for_) {
        for (auto to: edge.second) {
            result.emplace_back(edge.first, to);
        }
    }
    return result;
}
