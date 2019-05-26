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
    
constexpr size_t kRootIndex = 0;
constexpr uint8_t kLeafChar = '\0';
    

template <class NODE>
class _Edge {
    using Node = NODE;
    using Self = _Edge<Node>;
    
private:
    std::string label_;
    id_type target_ = kRootIndex;
    
public:
    _Edge(std::string_view label, id_type target) : label_(label), target_(target) {}
    
    std::string_view label() const {return std::string_view(label_);}
    id_type target() const {return target_;}
    
};


template <typename ITEM_TYPE>
class _Node {
    using Item = ITEM_TYPE;
    using Self = _Node<Item>;
    using Edge = _Edge<Self>;
    
private:
    std::map<uint8_t, Edge> edges_;
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
    
    id_type target(uint8_t c) const {
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
    void for_each_edge(ACTION action) const {
        for (auto& e : edges_) {
            action(e.first, e.second);
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
    
    size_t size() const {
        return container_.size();
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
        // Add leaf label as '\0'.
        auto& n = container_[node];
        auto target = n.target(kLeafChar);
        if (target != kRootIndex) {
            node = target;
        } else {
            n.add(kLeafChar, container_.size());
            node = container_.size();
            container_.emplace_back();
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
        auto item_index = _traverse(key);
        return item_index != -1 ? &storage_[item_index] : nullptr;
    }
    
    template <class NODE_ACTION>
    void dfs(NODE_ACTION node_action) const {
        dfs(kRootIndex, 0, node_action);
    }
    
    template <class NODE_ACTION>
    void dfs(size_t node, size_t depth, NODE_ACTION node_action) const {
        auto& n = container_[node];
        node_action(n, depth);
        n.for_each_edge([&](auto c, auto& edge) {
            dfs(edge.target(), depth+1, node_action);
        });
    }
    
    template <class NODE_ACTION>
    void bfs(NODE_ACTION node_action) const {
        std::queue<std::pair<size_t, size_t>> nodes;
        nodes.emplace(kRootIndex, 0);
        while (not nodes.empty()) {
            auto p = nodes.front();
            nodes.pop();
            auto index = p.first;
            auto depth = p.second;
            auto& n = container_[index];
            
            node_action(n, depth);
            n.for_each_edge([&](auto c, auto edge) {
                nodes.emplace(edge.target(), depth+1);
            });
        }
    }
    
    const Node& node(size_t id) const {
        return container_[id];
    }
    
    const Node& root() const {
        return node(kRootIndex);
    }
    
private:
    size_t _traverse(std::string_view key) const {
        size_t node = kRootIndex;
        for (uint8_t c : key) {
            auto& n = container_[node];
            auto target = n.target(c);
            if (target == kRootIndex)
                return -1;
            node = target;
        }
        auto& n = container_[node];
        auto target = n.target('\0');
        if (target == kRootIndex)
            return -1;
        node = target;
        
        auto& terminal = container_[node];
        return terminal.item_index();
    }
    
};

}
}

#endif /* graph_util_hpp */
