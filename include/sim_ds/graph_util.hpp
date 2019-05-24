//
//  graph_util.hpp
//
//  Created by 松本拓真 on 2019/05/24.
//

#ifndef Graph_hpp
#define Graph_hpp

#include "basic.hpp"

namespace sim_ds {
namespace graph_util {
    
const size_t kRootIndex = 0;
    

template <class NODE>
class _Edge {
    using node_type = NODE;
    using Self = _Edge<node_type>;
    
private:
    std::string label_;
    id_type target_ = kRootIndex;
    
public:
    _Edge(std::string_view label, id_type target) : label_(label), target_(target) {}
    
    std::string_view label() const {return std::string_view(label_);}
    id_type target() {return target_;}
    
};


template <typename ITEM_TYPE>
class _Node {
    using Item = ITEM_TYPE;
    using Self = _Node<Item>;
    using Edge = _Edge<Self>;
    
private:
    std::unordered_map<uint8_t, Edge> edges_;
    size_t item_index_;
    
public:
    _Node(size_t item_index = -1) : item_index_(item_index) {}
    
    bool add(std::string_view label, size_t target_index) {
        auto c = label.front();
        return edges_.try_emplace(c, label, target_index).second;
    }
    
    bool add(uint8_t c, size_t target_index) {
        std::string label;
        label.push_back(c);
        return add(label, target_index);
    }
    
    id_type target(uint8_t c) {
        auto it = edges_.find(c);
        if (it == edges_.end())
            return kRootIndex;
        return it->second.target();
    }
    
    id_type item_index() const {return item_index_;}
    
    void set_item_index(id_type item_index) {
        item_index_ = item_index;
    }
    
    bool terminal() const {return item_index() != -1;}
    
    template <class ACTION>
    void for_each(ACTION action) const {
        for (auto& e : edges_) {
            action(e);
        }
    }
    
};


template <typename ITEM_TYPE = bool>
class Trie {
    using item_type = ITEM_TYPE;
    using Self = Trie<item_type>;
    using Node = _Node<item_type>;
    
private:
    std::vector<Node> container_;
    std::vector<item_type> storage_;
    
public:
    Trie() {
        container_.emplace_back();
    }
    
    item_type* insert(std::string_view key, item_type item) {
        size_t node = kRootIndex;
        for (uint8_t c : key) {
            auto& n = container_[node];
            auto target = n.target(c);
            if (target != kRootIndex) {
                node = target;
            } else {
                n.add(c, container_.size());
                node = container_.size();
                container_.emplace_back();
            }
        }
        
        auto& terminal = container_[node];
        if (terminal.item_index() == -1) {
            // Insert new item.
            storage_.push_back(item);
            terminal.set_item_index(storage_.size()-1);
        } else {
            // Update item.
            storage_[terminal.item_index()] = item;
        }
        return &storage_[terminal.item_index()];
    }
    
    item_type* traverse(std::string_view key) {
        size_t node = kRootIndex;
        for (uint8_t c : key) {
            auto& n = container_[node];
            auto target = n.target(c);
            if (target == kRootIndex)
                return nullptr;
            node = target;
        }
        
        auto& terminal = container_[node];
        if (terminal.item_index() == -1) {
            return nullptr;
        } else {
            return &storage_[terminal.item_index()];
        }
    }
    
    template <class NODE_ACTION, class EDGE_ACTION>
    void dfs(NODE_ACTION node_action, EDGE_ACTION edge_action) const {
        dfs(kRootIndex, node_action, edge_action);
    }
    
    template <class NODE_ACTION, class EDGE_ACTION>
    void dfs(size_t node, NODE_ACTION node_action, EDGE_ACTION edge_action) const {
        auto& n = container_[node];
        node_action(n);
        n.for_each([&](auto& edge) {
            edge_action(edge);
            dfs(edge.target_index(), node_action, edge_action);
        });
    }
    
    template <class NODE_ACTION, class EDGE_ACTION>
    void bfs(NODE_ACTION node_action, EDGE_ACTION edge_action) const {
        std::queue<size_t> nodes;
        nodes.push(kRootIndex);
        while (not nodes.empty()) {
            auto& n = container_[nodes.front()];
            nodes.pop();
            
            node_action(n);
            n.for_each([&](auto& edge) {
                edge_action(edge);
                nodes.push(edge.target());
            });
        }
    }
    
};

}
}

#endif /* graph_util_hpp */
