//
//  graph_util.hpp
//
//  Created by 松本拓真 on 2019/05/24.
//

#ifndef Graph_hpp
#define Graph_hpp

#include "sim_ds/basic.hpp"

namespace sim_ds {
namespace graph_util {
    
constexpr size_t kRootIndex = 0;
constexpr uint8_t kLeafChar = '\0';


template <class GraphType>
class _Node {
public:
    using graph_type = GraphType;
    using size_type = typename graph_type::size_type;
    
    static constexpr size_type kItemEmpty = graph_type::kItemEmpty;
    
private:
    size_type item_index_;
    std::map<uint8_t, size_type> edges_;
    
public:
    _Node() : item_index_(kItemEmpty) {}
    _Node(size_type item_index) : item_index_(item_index) {}
    
    bool add(uint8_t c, size_type target_index) {
        return edges_.try_emplace(c, target_index).second;
    }
    
    size_type target(uint8_t c) const {
        auto it = edges_.find(c);
        if (it == edges_.end())
            return kRootIndex;
        return it->second;
    }
    
    size_type item_index() const {return item_index_;}
    
    void set_item_index(size_type item_index) {
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


template <typename ItemType, typename SizeType = size_t>
class Trie {
public:
    using item_type = ItemType;
    using size_type = SizeType;
    using Self = Trie<item_type, size_type>;
    using node_type = _Node<Self>;
    
    static constexpr size_type kItemEmpty = std::numeric_limits<size_type>::max();
    
private:
    std::vector<node_type> container_;
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
        if (terminal.item_index() == kItemEmpty) {
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
        n.for_each_edge([&](auto c, auto target) {
            dfs(target, depth+1, node_action);
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
            n.for_each_edge([&](auto c, auto target) {
                nodes.emplace(target, depth+1);
            });
        }
    }
    
    const node_type& node(size_t id) const {
        return container_[id];
    }
    
    const node_type& root() const {
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
