#include "random_access_table.h"

#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "storage/index/random_access_table/random_access_table_block.h"

using namespace std;

template <size_t N>
RandomAccessTable<N>::RandomAccessTable(const string& name) :
    file_id(file_manager.get_file_id(name)) { }


template <size_t N>
Record<N>* RandomAccessTable<N>::operator[](uint64_t pos) {
    auto block        = pos / RandomAccessTableBlock<N>::max_records;
    auto pos_in_block = pos % RandomAccessTableBlock<N>::max_records;

    auto current_block = make_unique<RandomAccessTableBlock<N>>(buffer_manager.get_page_readonly(file_id, block));
    if (pos_in_block < *current_block->record_count) {
        return (*current_block)[pos_in_block];
    } else {
        return nullptr;
    }
}


template <size_t N>
void RandomAccessTable<N>::append_record(const Record<N>& record) {
    auto& last_block_page = buffer_manager.get_page_readonly(file_id, file_manager.count_pages(file_id));
    auto last_block = make_unique<RandomAccessTableBlock<N>>(last_block_page);
    if (*last_block->record_count < last_block->max_records) {
        last_block = make_unique<RandomAccessTableBlock<N>>(buffer_manager.get_page_editable(file_id, file_manager.count_pages(file_id)));
    } else {
        last_block = make_unique<RandomAccessTableBlock<N>>(buffer_manager.append_vpage(file_id));
    }
    last_block->append_record(record);
}

template class RandomAccessTable<3>;
