#include "quad_model_index_provider.h"

#include "graph_models/quad_model/quad_model.h"

using namespace Paths;

bool QuadModelIndexProvider::node_exists(uint64_t node_id) {
    auto node_iter = quad_model.nodes->get_range(interruption_requested, {node_id}, {node_id});
    return node_iter.next() != nullptr;
}


std::unique_ptr<EdgeIter> QuadModelIndexProvider::get_btree_iter(uint64_t type_id, bool inverse) {
    // B+Tree settings
    Record<4> min_ids = {type_id, 0, 0, 0};
    Record<4> max_ids = {type_id, UINT64_MAX, UINT64_MAX, UINT64_MAX};

    // Get iter from correct B+Tree
    if (inverse) {
        return std::make_unique<BTreeIndexIterator<4>>(
            quad_model.type_to_from_edge->get_range(interruption_requested, min_ids, max_ids),
            1,
            2
        );
    } else {
        return std::make_unique<BTreeIndexIterator<4>>(
            quad_model.type_from_to_edge->get_range(interruption_requested, min_ids,max_ids),
            1,
            2
        );
    }
}


std::unique_ptr<EdgeIter> QuadModelIndexProvider::get_btree_iter(uint64_t type_id, bool inverse, uint64_t node_id) {
    // B+Tree settings
    Record<4> min_ids = {type_id, node_id, 0, 0};
    Record<4> max_ids = {type_id, node_id, UINT64_MAX, UINT64_MAX};

    // Get iter from correct B+Tree
    if (inverse) {
        return std::make_unique<BTreeIndexIterator<4>>(
            quad_model.type_to_from_edge->get_range(interruption_requested, min_ids, max_ids),
            1,
            2
        );
    } else {
        return std::make_unique<BTreeIndexIterator<4>>(
            quad_model.type_from_to_edge->get_range(interruption_requested, min_ids,max_ids),
            1,
            2
        );
    }
}


std::unique_ptr<EdgeIter> QuadModelIndexProvider::get_iter(uint64_t type_id, bool inverse, uint64_t node_id) {
    if (inverse) {
        // auto assigned_index = t_inv_info.find(type_id);
        // if (assigned_index->second == IndexType::BTREE) {
            return get_btree_iter(type_id, inverse, node_id);
        // }
        // auto index = quad_model.get_index(type_id, inverse, assigned_index->second, interruption_requested);
        // return index->get_iter(node_id, interruption_requested);
    } else {
        // auto assigned_index = t_info.find(type_id);
        // if (assigned_index->second == IndexType::BTREE) {
            return get_btree_iter(type_id, inverse, node_id);
        // }
        // auto index = quad_model.get_index(type_id, inverse, assigned_index->second, interruption_requested);
        // return index->get_iter(node_id, interruption_requested);
    }
}

std::unique_ptr<EdgeIter> QuadModelIndexProvider::get_iter(uint64_t type_id, bool inverse) {
    if (inverse) {
        return get_btree_iter(type_id, inverse);
    } else {
        return get_btree_iter(type_id, inverse);
    }
}