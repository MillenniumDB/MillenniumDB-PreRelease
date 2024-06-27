/*
 * Tree is the data structure used for storing the data for LSH ForestIndex. It is a full binary tree that stores
 * buckets of object_ids in the leaves. In order to descend the tree with a given tensor, it must be hashed at each
 * level with a random hyperplane for choosing the direction. Each node will always have either zero or two children.
 */
#pragma once

#include "storage/index/tensor_store/lsh/tree_node.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

class TensorStore;

namespace LSH {
enum class MetricType;

class Tree {
public:
    static constexpr uint_fast32_t GENERATE_PLANE_EPOCHS = 200UL;

    static inline auto random_engine =
      std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());

    static inline auto uniform_distribution = std::uniform_real_distribution<float>(0.0, 1.0);

    static inline uint64_t get_uniform_uint64(uint64_t min, uint64_t max) {
        return uniform_distribution(random_engine) * (max - min) + min;
    }

    static TreeNode* deserialize_angular_split_node(std::fstream& fs) {
        return new AngularSplitNode(fs);
    };

    static TreeNode* deserialize_minkowski_split_node(std::fstream& fs) {
        return new MinkowskiSplitNode(fs);
    }

    // Return all the object_ids stored in the node descendants
    static std::vector<uint64_t> descendants(TreeNode* node);

    // Initialize a new tree
    Tree(const TensorStore& tensor_store, MetricType metric_type, uint64_t max_bucket_size, uint64_t max_depth);

    // Load an existing serialized tree from a filestream
    Tree(std::fstream& fs, const TensorStore& tensor_store);

    ~Tree();

    // Build the tree for the TensorStore
    void build();

    // Get the leaf with the longest prefix match and its depth for the hashed query tensor
    std::pair<LeafNode*, uint64_t> descend(const std::vector<float>& query_tensor) const;

    // Serialize the tree into a filestream
    void serialize(std::fstream& fs) const;

private:
    const TensorStore& tensor_store;
    MetricType         metric_type;
    uint64_t           tensors_dim;
    uint64_t           max_bucket_size;
    uint64_t           max_depth;

    float (*similarity_fn)(const std::vector<float>&, const std::vector<float>&);

    TreeNode* root;

    // Used to prevent multiple allocations
    std::vector<float> tensor_buffer;

    // Split the leaf node into two children. Return a reference to each one ({ left, right })
    std::pair<LeafNode*, LeafNode*> create_split(LeafNode* leaf_node);

    // Returns the best plane for the split. It returns its normal and offset
    std::pair<std::vector<float>, float> generate_plane(const std::vector<uint64_t>& object_ids);

    // Deserialize the tree from a filestream
    void deserialize(std::fstream& fs);
};
} // namespace LSH
