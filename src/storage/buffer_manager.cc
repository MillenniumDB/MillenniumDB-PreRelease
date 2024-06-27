#include "buffer_manager.h"

#include <cstdlib>
#include <type_traits>

#include "macros/aligned_alloc.h"
#include "misc/fatal_error.h"
#include "query/query_context.h"
#include "storage/file_manager.h"

// memory for the object
static typename std::aligned_storage<sizeof(BufferManager), alignof(BufferManager)>::type buffer_manager_buf;

// global object
BufferManager& buffer_manager = reinterpret_cast<BufferManager&>(buffer_manager_buf);


BufferManager::BufferManager(uint64_t vpage_buffer_pool_size,
                             uint64_t ppage_buffer_pool_size_per_worker,
                             uint64_t upage_buffer_pool_size,
                             uint64_t workers) :
    vp_pool(new VPage[vpage_buffer_pool_size]),
    vp_data(reinterpret_cast<char*>(
            MDB_ALIGNED_ALLOC(VPage::SIZE, vpage_buffer_pool_size * VPage::SIZE))),
    vp_pool_size(vpage_buffer_pool_size),
    pp_pool(new PPage[ppage_buffer_pool_size_per_worker * workers]),
    pp_data(reinterpret_cast<char*>(
            MDB_ALIGNED_ALLOC(PPage::SIZE, ppage_buffer_pool_size_per_worker * workers * PPage::SIZE))),
    pp_pool_size(ppage_buffer_pool_size_per_worker),
    up_pool(new UPage[upage_buffer_pool_size]),
    up_data(reinterpret_cast<char*>(
             MDB_ALIGNED_ALLOC(UPage::SIZE, upage_buffer_pool_size * UPage::SIZE))),
    up_pool_size(upage_buffer_pool_size)
{
    if (vp_data == nullptr || vp_pool == nullptr) {
        FATAL_ERROR("Could not allocate versioned page buffers, try using a smaller size");
    }

    if (pp_data == nullptr || pp_pool == nullptr) {
        FATAL_ERROR("Could not allocate private page buffers, try using a smaller size");
    }

    if (up_data == nullptr || up_pool == nullptr) {
        FATAL_ERROR("Could not allocate unversioned page buffers, try using a smaller size");
    }

    for (uint64_t i = 0; i <vp_pool_size; i++) {
        vp_pool[i].set_bytes(&vp_data[i * VPage::SIZE]);
    }

    for (uint64_t i = 0; i < up_pool_size; i++) {
        up_pool[i].set_bytes(&up_data[i * UPage::SIZE]);
    }

    for (uint64_t i = 0; i < pp_pool_size * workers; i++) {
        pp_pool[i].set_bytes(&pp_data[i * PPage::SIZE]);
    }

    pp_map.resize(workers);
    pp_clocks.resize(workers);

    vp_map.reserve(vpage_buffer_pool_size);
    up_map.reserve(upage_buffer_pool_size);
}


BufferManager::~BufferManager() {
    flush();
    delete[] (vp_pool);
    delete[] (up_pool);
    delete[] (pp_pool);
    MDB_ALIGNED_FREE(vp_data);
    MDB_ALIGNED_FREE(up_data);
    MDB_ALIGNED_FREE(pp_data);
}


void BufferManager::init(uint64_t versioned_page_buffer_size_in_bytes,
                         uint64_t private_page_buffer_pool_size_per_worker_in_bytes,
                         uint64_t unversioned_page_buffer_size_in_bytes,
                         uint64_t workers)
{
    // placement new
    new (&buffer_manager) BufferManager(
        versioned_page_buffer_size_in_bytes / VPage::SIZE,
        private_page_buffer_pool_size_per_worker_in_bytes / PPage::SIZE,
        unversioned_page_buffer_size_in_bytes / UPage::SIZE,
        workers
    );
}


void BufferManager::flush() {
    // flush() is always called at destruction.
    assert(vp_pool != nullptr);
    for (uint64_t i = 0; i < vp_pool_size; i++) {
        VPage& page = vp_pool[i];
        assert(page.pins == 0);
        if (page.dirty && page.next_version == nullptr) {
            file_manager.flush(page);
        }
    }

    assert(up_pool != nullptr);
    for (uint64_t i = 0; i < up_pool_size; i++) {
        UPage& page = up_pool[i];
        assert(page.pins == 0);
        if (page.dirty) {
            file_manager.flush(page);
        }
    }
}


// We assume this executes on one thread at a time, controlled by vp_mutex
VPage& BufferManager::get_vpage_available() {
    while (true) {
        vp_clock++;
        vp_clock = vp_clock < vp_pool_size ? vp_clock : 0;

        auto& page = vp_pool[vp_clock];

        if (page.pins != 0) {
            continue;
        }
        if (page.second_chance) {
            page.second_chance = false;
            continue;
        }
        if (page.prev_version == nullptr && page.next_version == nullptr) {
            if (page.page_id.file_id.id != FileId::UNASSIGNED) {
                vp_map.erase(page.page_id);
            }
            if (page.dirty) {
                // TODO: reduce counter of version writing pending for page version
                // (we know this is the last version and there is no previous version)
                file_manager.flush(page);
            }
            return page;
        }


        running_version_count_mutex.lock();
        auto it = running_version_count.find(page.version_number);
        bool version_not_being_used = it == running_version_count.end();
        running_version_count_mutex.unlock();


        if (version_not_being_used) {
            if (page.dirty) {
                // TODO: reduce counter of version writing pending for page version
                // (we know this is the last version and there is no previous version)
            }

            if (page.prev_version != nullptr) {
                page.prev_version->next_version = page.next_version;
            } else { // page is the first in the linked list
                // we know page.next_version != nullptr
                // if it is the first version and there are more versions we need to
                // edit vp_map to point to the new oldest version
                if (page.page_id.file_id.id != FileId::UNASSIGNED) {
                    auto it2 = vp_map.find(page.page_id);
                    assert(it2 != vp_map.end());
                    vp_map.erase(it2);
                    vp_map.insert({ page.page_id, page.next_version });
                }
            }

            if (page.next_version != nullptr) {
                page.next_version->prev_version = page.prev_version;
            } else { // page is the last in the linked list
                // flush when dirty and there is no next version
                if (page.dirty) {
                    file_manager.flush(page);

                    // we know page.prev_version != nullptr
                    VPage* p = page.prev_version;
                    // all previous dirty vp_map are no longer dirty because a newer version
                    // was written to disk, and we only have one update at a time, so previous versions
                    // must have ended
                    do {
                        if (p->dirty) {
                            p->dirty = false;
                            // TODO: reduce counter of version writing pending for page p version
                            // (we know this is the last version and there is no previous version)
                        }

                        p = page.prev_version;
                    } while (p != nullptr);
                }
            }

            return page;
        }
    }
    return vp_pool[vp_clock];
}


PPage& BufferManager::get_ppage_available(uint_fast32_t thread_pos) {
    PPage* page;

    auto& clock = pp_clocks[thread_pos];
    while (true) {
        // when pins == 0 the are no synchronization problems with pins and usage
        page = &pp_pool[(thread_pos * pp_pool_size) + clock];
        if (page->pins == 0) {
            if (!page->second_chance) {
                break;
            } else {
                page->second_chance = false;
            }
        }
        clock++;
        clock = clock < pp_pool_size ? clock : 0;
    }
    return *page;
}


// use query_context result_version if it exists, otherwise use start_version
VPage& BufferManager::get_page_readonly(FileId file_id, uint64_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    uint64_t start_version  = get_query_ctx().start_version;
    uint64_t result_version = get_query_ctx().result_version;

    vp_mutex.lock();
    auto it = vp_map.find(page_id);

    if (it == vp_map.end()) {
        auto& page = get_vpage_available();

        page.reassign(page_id);
        page.version_number = start_version;
        page.prev_version = nullptr;
        page.next_version = nullptr;
        vp_map.insert({ page_id, &page });

        file_manager.read_existing_page(page_id, page.get_bytes());
        vp_mutex.unlock();

        return page;
    } else {
        // page is the buffer, search the corresponding version
        VPage* page = it->second;

        while (page->next_version != nullptr && page->next_version->version_number <= result_version) {
            page = page->next_version;
        }

        assert(page->version_number <= result_version);

        page->pin();
        vp_mutex.unlock();

        return *page;
    }
}


bool BufferManager::need_edit_version(const VPage& page) {
    uint64_t result_version = get_query_ctx().result_version;
    return page.version_number != result_version;
}


// use query_context result_version (creating it if not exists)
VPage& BufferManager::get_page_editable(FileId file_id, uint64_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    uint64_t start_version  = get_query_ctx().start_version;
    uint64_t result_version = get_query_ctx().result_version;

    std::lock_guard<std::mutex> lck(vp_mutex);
    auto it = vp_map.find(page_id);

    if (it == vp_map.end()) {
        auto& old_page = get_vpage_available();
        auto& new_page = get_vpage_available();

        old_page.reassign_page_id(page_id);
        new_page.reassign(page_id);

        old_page.version_number = start_version;
        old_page.prev_version = nullptr;
        old_page.next_version = &new_page;

        new_page.version_number = result_version;
        new_page.prev_version = &old_page;
        new_page.next_version = nullptr;
        new_page.dirty = true;

        vp_map.insert({ page_id, &old_page });

        file_manager.read_existing_page(page_id, old_page.get_bytes());
        std::memcpy(new_page.get_bytes(), old_page.get_bytes(), VPage::SIZE);

        current_modifications.push_back(page_id);

        return new_page;
    } else {
        // page is in the buffer, search the corresponding version
        VPage* page = it->second;

        while (page->next_version != nullptr && page->next_version->version_number <= result_version) {
            page = page->next_version;
        }

        if (page->version_number != result_version) {
            auto& new_page = get_vpage_available();

            new_page.reassign(page_id);

            page->next_version = &new_page;

            new_page.version_number = result_version;
            new_page.prev_version = page;
            new_page.next_version = nullptr;
            new_page.dirty = true;

            current_modifications.push_back(page_id);

            std::memcpy(new_page.get_bytes(), page->get_bytes(), VPage::SIZE);
            return new_page;
        } else {
            page->pin();
            return *page;
        }
    }
}


VPage& BufferManager::append_vpage(FileId file_id) {
    uint64_t result_version = get_query_ctx().result_version;

    std::lock_guard<std::mutex> lck(vp_mutex);

    auto& new_page = get_vpage_available(); // need to have vp_mutex locked

    auto page_number = file_manager.append_page(file_id, new_page.get_bytes());
    PageId page_id(file_id, page_number);
    new_page.reassign(page_id);

    new_page.version_number = result_version;
    new_page.prev_version = nullptr;
    new_page.next_version = nullptr;

    new_page.dirty = true;

    vp_map.insert({ page_id, &new_page });

    current_modifications.push_back(page_id);
    return new_page;
}


// We assume this executes on one thread at a time, controlled by up_mutex
UPage& BufferManager::get_upage_available() {
    while (true) {
        up_clock++;
        up_clock = up_clock < up_pool_size ? up_clock : 0;

        auto& page = up_pool[up_clock];

        if (page.pins != 0) {
            continue;
        }
        if (page.second_chance) {
            page.second_chance = false;
            continue;
        }
        return page;
    }
}


UPage& BufferManager::get_unversioned_page(FileId file_id, uint64_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    up_mutex.lock();
    auto it = up_map.find(page_id);

    if (it == up_map.end()) {
        auto& page = get_upage_available();

        if (page.page_id.file_id.id != FileId::UNASSIGNED) {
            up_map.erase(page.page_id);
        }

        if (page.dirty) {
            file_manager.flush(page);
        }

        page.reassign(page_id);
        up_map.insert({ page_id, &page });

        file_manager.read_existing_page(page_id, page.get_bytes());
        up_mutex.unlock();

        return page;
    } else {
        UPage* page = it->second;
        page->pin();
        up_mutex.unlock();

        return *page;
    }
}


UPage& BufferManager::append_unversioned_page(FileId file_id) noexcept {
    up_mutex.lock();

    auto& new_page = get_upage_available();
    auto page_number = file_manager.append_page(file_id, new_page.get_bytes());
    PageId page_id(file_id, page_number);
    new_page.reassign(page_id);
    new_page.dirty = true;

    up_map.insert({ page_id, &new_page });
    up_mutex.unlock();

    return new_page;
}


PPage& BufferManager::get_ppage(TmpFileId tmp_file_id, uint64_t page_number) noexcept {
    const PageId page_id(tmp_file_id.file_id, page_number);
    const auto thread_pos = tmp_file_id.private_buffer_pos;

    // We don't need a mutex here because tmp pp_map[thread_pos] is
    // assigned to one specific thread.
    // If we change this in the future we might need a mutex lock.

    auto it = pp_map[thread_pos].find(page_id);
    if (it == pp_map[thread_pos].end()) {
        auto& page = get_ppage_available(thread_pos);
        if (page.page_id.file_id.id != FileId::UNASSIGNED) {
            pp_map[thread_pos].erase(page.page_id);
        }

        if (page.dirty) {
            file_manager.flush(page);
        }
        page.reassign(page_id);

        file_manager.read_tmp_page(page_id, page.get_bytes());
        pp_map[thread_pos].insert({ page_id, &page });
        return page;
    } else {
        it->second->pins++;
        return *it->second;
    }
}


void BufferManager::remove_tmp(TmpFileId tmp_file_id) {
    assert(pp_pool != nullptr);
    auto offset = pp_pool_size * tmp_file_id.private_buffer_pos;
    for (uint64_t i = 0; i < pp_pool_size; i++) {
        auto page_id = pp_pool[offset + i].page_id;
        if (page_id.file_id == tmp_file_id.file_id) {
            pp_map[tmp_file_id.private_buffer_pos].erase(page_id);
            pp_pool[offset + i].reset();
        }
    }
}


BufferManager::VersionScope::~VersionScope() {
    buffer_manager.terminate(*this);
}


void BufferManager::terminate(const VersionScope& version_scope) {
    std::lock_guard<std::mutex> lck(running_version_count_mutex);
    auto it1 = running_version_count.find(version_scope.start_version);
    assert(it1 != running_version_count.end());

    it1->second--;

    if (it1->second == 0) {
        running_version_count.erase(it1);
    }
    if (version_scope.is_editable) {
        auto it2 = running_version_count.find(version_scope.start_version + 1);
        assert(it2 != running_version_count.end());

        it2->second--;

        if (it2->second == 0) {
            running_version_count.erase(it2);
        }

        last_stable_version++;

        // TODO: write to log
        // for (auto& page_id : current_modifications) {
        // }
        current_modifications.clear();
    }
}
