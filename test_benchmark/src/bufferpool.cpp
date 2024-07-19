/*
 * Copyright (C) 2019-2023 DBIS Group - TU Ilmenau, All Rights Reserved.
 *
 * This file is part of the Poseidon package.
 *
 * Poseidon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Poseidon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Poseidon. If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include "defs.hpp"
#include "bufferpool.hpp"
#include "exceptions.hpp"

bufferpool::bufferpool(std::size_t bsize) : bsize_(bsize), slots_(bsize_), p_reads_(0), l_reads_(0) {
    //spdlog::info("creating bufferpool with {} pages", bsize);
    buffer_ = new page[bsize_];
    slots_.set(); // set everything to 1 == unused
}

bufferpool::~bufferpool() {
    flush_all();
    delete [] buffer_;
}

void bufferpool::pin_page(paged_file::page_id pid) {
    std::unique_lock lock(mutex_);
    auto iter = ptable_.find(pid);
    if (iter != ptable_.end()) {
        iter->second.pinned_ = true;
    }
    //else
    //    spdlog::info("cannot pin page {}", pid);

}

void bufferpool::unpin_page(paged_file::page_id pid) {
    std::unique_lock lock(mutex_);
    auto iter = ptable_.find(pid);
    if (iter != ptable_.end()) {
        iter->second.pinned_ = false;
    }
    //else
    //    spdlog::info("cannot unpin page {}", pid);

}

void bufferpool::register_file(uint8_t file_id, paged_file_ptr pf) {
    assert(file_id < MAX_PFILES);
    files_[file_id] = pf;
}

paged_file_ptr bufferpool::get_file(uint8_t file_id) {
    assert(file_id < MAX_PFILES && files_[file_id]);
    return files_[file_id];
}

void bufferpool::scan_file(uint8_t file_id, std::function<void(page *p)> cb) {
    assert(file_id < MAX_PFILES && files_[file_id]);
    auto pos = slots_.find_first();
    if (pos == boost::dynamic_bitset<>::npos) {
        evict_page();
        pos = slots_.find_first();
    }
    slots_.flip(pos);

    files_[file_id]->scan_pages(buffer_[pos], [&](page& pg, paged_file::page_id pid) {
        l_reads_++;
        p_reads_++;
        cb(&pg);
    });
    slots_.flip(pos);
}
 
std::pair<page*, paged_file::page_id> bufferpool::last_valid_page(uint8_t file_id) {
    assert(file_id < MAX_PFILES && files_[file_id]);
    auto pid = files_[file_id]->last_valid_page();
    // spdlog::info("last_valid_page: {}", pid);
    return std::make_pair(fetch_page(pid | (static_cast<uint64_t>(file_id) << 60)), pid);
}

page *bufferpool::fetch_page(paged_file::page_id pid) {
    std::unique_lock lck(mutex_); // TODO: EXPENSIVE!!
    l_reads_++;
    auto iter = ptable_.find(pid);
    // spdlog::info("bufferpool::fetch_page {}", pid & 0xFFFFFFFFFFFFFFF);
    if (iter != ptable_.end()) {
        // move pid in lru_list_
        lru_list_.move_to_mru(iter->second.lru_node_);
        return iter->second.p_;
    }
    if (lru_list_.size() == bsize_) {
        // evict page from lru_list_.front();
        auto res = evict_page();
        assert(res == true);
    }

    // load page from file
    auto p = load_page_from_file(pid);

    // ... and to the LRU list
    auto *node = lru_list_.add_to_mru(pid);
    // ... add it to the hashtable
    ptable_.emplace(pid, buf_slot{ p.first, false, false, p.second, node});
    return p.first;
}
    
std::pair<page*, paged_file::page_id> bufferpool::allocate_page(uint8_t file_id) {
    assert(file_id < MAX_PFILES && files_[file_id]);
    paged_file::page_id pid = files_[file_id]->allocate_page();
    //spdlog::debug("bufferpool::allocate_page -> {} in file {} -> {}", pid, file_id, (pid | (static_cast<uint64_t>(file_id) << 60)));
    return std::make_pair(fetch_page(pid | (static_cast<uint64_t>(file_id) << 60)), pid);
}

void bufferpool::free_page(paged_file::page_id pid) {
    std::unique_lock lock(mutex_);

    auto iter = ptable_.find(pid);
    if (iter != ptable_.end()) {
        auto raw_pid = pid & 0xFFFFFFFFFFFFFFF;
        auto file_id = (pid & 0xF000000000000000) >> 60;
        assert(file_id < MAX_PFILES && files_[file_id]);
        //spdlog::debug("free_page: #{}(raw:{}|file_id:{})", pid, raw_pid, file_id);
        memset(iter->second.p_, 0, sizeof(PF_PAGE_SIZE));
        lru_list_.remove(iter->second.lru_node_);
        ptable_.erase(pid);

        files_[file_id]->free_page(raw_pid);
    }
}

void bufferpool::mark_dirty(paged_file::page_id pid) {
    std::unique_lock lock(mutex_);
    auto iter = ptable_.find(pid);
    if (iter != ptable_.end()) {
        iter->second.dirty_ = true;
    }
}

void bufferpool::flush_all() {
    uint64_t num = 0;

    std::unique_lock lock(mutex_);
    for (auto iter = ptable_.begin(); iter != ptable_.end(); iter++) {
        if (iter->second.dirty_) {
            auto pid = iter->first;
            write_page_to_file(pid, iter->second.p_);
            num++;
            iter->second.dirty_ = false;
        }
    }
    //if (num > 0)
    //    spdlog::info("bufferpool: {} pages flushed", num);
}

void bufferpool::flush_pages(uint8_t file_id) {
    std::unique_lock lock(mutex_);
    for (auto iter = ptable_.begin(); iter != ptable_.end(); iter++) {
        if (iter->second.dirty_) {
            auto pid = iter->first;
            auto page_file_id = (pid & 0xF000000000000000) >> 60;
            if (file_id == page_file_id) {
                write_page_to_file(pid, iter->second.p_);
                iter->second.dirty_ = false;
            }
        }
    }
}

void bufferpool::flush_page(paged_file::page_id pid, bool evict) {
    std::unique_lock lock(mutex_);
    auto iter = ptable_.find(pid);
    if (iter == ptable_.end())
        return;
    
    if (iter->second.dirty_) {
        auto pid2 = iter->first;
        write_page_to_file(pid2, iter->second.p_);
        iter->second.dirty_ = false;
    }
    if (evict) {
        lru_list_.remove(iter->second.lru_node_);
        slots_.set(iter->second.pos_);
        memset(iter->second.p_, 0, sizeof(PF_PAGE_SIZE));
        ptable_.erase(pid);
    }
}

void bufferpool::purge() {
    std::unique_lock lock(mutex_);
    slots_.set();
    // TODO: lru_list
    lru_list_.clear();
    ptable_.clear();
    memset(buffer_, 0, sizeof(page) * bsize_);
}

bool bufferpool::has_page(paged_file::page_id pid) {
    return ptable_.find(pid) != ptable_.end();
}

bool bufferpool::evict_page() {
    //spdlog::debug("\t---- evict_page...");
    std::unique_lock lock(mutex_);
    // TODO: obtain lru_list lock
    for (auto it1 = lru_list_.begin(); it1 != lru_list_.end(); ++it1) {
        auto pid = *it1;
        auto it2 = ptable_.find(pid);
        if (it2 != ptable_.end()) {
            if (it2->second.pinned_)
                continue;
            if (it2->second.dirty_) {
                //spdlog::info("bufferpool::evict dirty page {} of file {}", pid & 0xFFFFFFFFFFFFFFF, (pid & 0xF000000000000000) >> 60);
                // TODO: write WAL log record for UNDO
                write_page_to_file(pid, it2->second.p_);
            }
            //if (((pid & 0xF000000000000000) >> 60) == NODE_FILE_ID && (pid & 0xFFFFFFFFFFFFFFF) == 921)
            //    spdlog::info("bufferpool::evict page {} of file {} - pinned: {}", 
            //    pid & 0xFFFFFFFFFFFFFFF, (pid & 0xF000000000000000) >> 60, it2->second.pinned_);
            slots_.set(it2->second.pos_);
            memset(it2->second.p_, 0, sizeof(PF_PAGE_SIZE));
            lru_list_.remove(it2->second.lru_node_);
            ptable_.erase(pid);
            return true;
        }
    }
    return false;
}

std::pair<page *, std::size_t> bufferpool::load_page_from_file(paged_file::page_id pid) {
    // find empty slot
    std::unique_lock lck(mutex_);
    auto pos = slots_.find_first();
    if (pos >= bsize_) {
        throw bufferpool_overrun();
    }
    // mark slot as occupied
    slots_.set(pos, false);

    // remove file_id from pid
    auto raw_pid = pid & 0xFFFFFFFFFFFFFFF;

    // select file
    auto file_id = (pid & 0xF000000000000000) >> 60;
    assert(file_id < MAX_PFILES && files_[file_id]);
    //spdlog::debug("read page {}|{} from file {} -> position: {}", pid, raw_pid, file_id, pos);
    files_[file_id]->read_page(raw_pid, buffer_[pos]);
    return std::make_pair(&(buffer_[pos]), pos);
}

void bufferpool::write_page_to_file(paged_file::page_id pid, page *pg) {
    auto raw_pid = pid & 0xFFFFFFFFFFFFFFF;
    auto file_id = (pid & 0xF000000000000000) >> 60;
    assert(file_id < MAX_PFILES && files_[file_id]);
    files_[file_id]->write_page(raw_pid, *pg);
}

void bufferpool::dump() {
    std::cout << "----------- BUFFERPOOL -----------\n";
    for (auto iter = ptable_.begin(); iter != ptable_.end(); iter++) {
        auto pid = iter->first & 0xFFFFFFFFFFFFFFF;
        auto file_id = (iter->first & 0xF000000000000000) >> 60;

        std::cout << "page #" << pid << ", file=" << file_id << ", dirty=" << iter->second.dirty_ << std::endl;
    }
}

double bufferpool::hit_ratio() const {
    // spdlog::info("bufferpool: logical reads={}, physical reads={}", l_reads_, p_reads_);
    return (double) (l_reads_ - p_reads_) / (double) l_reads_;
}
