#include "tensor_buffer_manager.h"

#include <sys/stat.h>

#include "macros/aligned_alloc.h"
#include "misc/fatal_error.h"
#include "storage/file_manager.h"

using namespace std;

TensorBufferManager::TensorBufferManager(FileId file_id, uint64_t tensor_page_buffer_pool_size, bool preload_pages) :
    file_id               (file_id),
    clock_pos             (0),
    tensor_page_pool      (new TensorPage[tensor_page_buffer_pool_size]),
    tensor_data           (reinterpret_cast<char*>(MDB_ALIGNED_ALLOC(TensorPage::SIZE, tensor_page_buffer_pool_size * TensorPage::SIZE))),
    tensor_page_pool_size (tensor_page_buffer_pool_size)
{
    if (tensor_data == nullptr) {
        FATAL_ERROR("Could not allocate tensor page buffers, try using a smaller size");
    }

    for (uint64_t i = 0; i < tensor_page_pool_size; ++i) {
        tensor_page_pool[i].set_bytes(&tensor_data[i * TensorPage::SIZE]);
    }

    pages_map.reserve(tensor_page_pool_size);

    if (preload_pages) {
        // Sequentially read pages from disk until the pool is full
        struct stat buf;
        fstat(file_id.id, &buf);
        const auto file_size = buf.st_size;
        const auto max_preload = std::min(tensor_page_pool_size, file_size / TensorPage::SIZE);

        for (uint64_t i = 0; i < max_preload; ++i) {
            const PageId page_id(file_id, i);
            auto& page = tensor_page_pool[i];
            page.reassign_preload(page_id);
            file_manager.read_existing_page(page.page_id, page.get_bytes());
            pages_map.insert({ page.page_id, &page });
        }
    }
}


TensorBufferManager::~TensorBufferManager() {
    flush();
    delete[](tensor_page_pool);
    MDB_ALIGNED_FREE(tensor_data);
}


void TensorBufferManager::flush() {
    // flush() is always called at destruction.
    assert(tensor_page_pool != nullptr);
    for (uint64_t i = 0; i < tensor_page_pool_size; ++i) {
        auto& page = tensor_page_pool[i];
        assert(page.pins == 0);
        if (page.dirty) {
            file_manager.flush(page);
        }
    }
}


uint_fast32_t TensorBufferManager::count_pages() const {
    return file_manager.count_pages(file_id);
}


TensorPage& TensorBufferManager::get_tensor_page_available() {
    TensorPage* page;

    while (true) {
        page = &tensor_page_pool[clock_pos];
        // when pins == 0 the are no synchronization problems with pins and usage
        if (tensor_page_pool[clock_pos].pins == 0) {
            if (!page->second_chance) {
                break;
            } else {
                page->second_chance = false;
            }
        }
        ++clock_pos;
        clock_pos = clock_pos < tensor_page_pool_size ? clock_pos : 0;
    }
    return *page;
}


TensorPage& TensorBufferManager::get_page(uint_fast32_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    auto it = pages_map.find(page_id);

    if (it == pages_map.end()) {
        auto& page = get_tensor_page_available();

        if (page.page_id.file_id.id != FileId::UNASSIGNED) {
            pages_map.erase(page.page_id);
        }

        if (page.dirty) {
            file_manager.flush(page);
        }
        page.reassign(page_id);

        file_manager.read_existing_page(page_id, page.get_bytes());
        pages_map.insert({ page_id, &page });

        return page;
    } else { // page is the buffer
        TensorPage* page = it->second;
        page->pin();
        return *it->second;
    }
}


TensorPage& TensorBufferManager::append_page() {
    auto& new_page    = get_tensor_page_available();
    auto  page_number = file_manager.append_page(file_id, new_page.get_bytes());
    PageId page_id(file_id, page_number);
    new_page.reassign(page_id);
    new_page.dirty = true;

    pages_map.insert({ page_id, &new_page });

    return new_page;
}


TensorPage& TensorBufferManager::get_or_append_page(uint_fast32_t page_number) {
    if (page_number < count_pages()) {
        // The page exists
        return get_page(page_number);
    }

    assert(page_number == count_pages() && "Should have been called with page_number == count_pages()");
    // The page does not exist, we need to append it
    return append_page();
}
