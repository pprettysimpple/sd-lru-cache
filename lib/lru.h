#pragma once

#include <memory>
#include <map>
#include <list>

#ifdef DEBUG
#include <cassert>
#include <iostream>
#endif

template <typename TKey, typename TValue, size_t Capacity>
class TLRUCache {

    struct TNode {
        template <typename TKey_, typename TValue_>
        TNode(TKey_ key, TValue_&& value)
                : Key(std::forward<TKey_>(key))
                , Value(std::forward<TValue_>(value))
        {
        }

        TKey    Key;
        TValue  Value;
    };

public:
    TLRUCache() noexcept = default;

    TLRUCache(const TLRUCache&) = delete;
    TLRUCache(TLRUCache&&) = delete;
    TLRUCache& operator=(const TLRUCache&) = delete;
    TLRUCache& operator=(TLRUCache&&) = delete;

    TValue* Get(const TKey& key) noexcept;

    void Put(const TKey& key, TValue value);
private:
    void DropLRU();

#ifdef DEBUG
    bool CheckState() const;
#endif

    // may be optimized with:
    // using TNodeRef = typename std::list<TNode>::iterator;
    // std::map<TNodeRef, TNodeRef> Map;
    // but I am very lazy...
    std::map<TKey, typename std::list<TNode>::iterator>  Map;
    std::list<TNode>        List;
};

template<typename TKey, typename TValue, size_t Capacity>
TValue* TLRUCache<TKey, TValue, Capacity>::Get(const TKey& key) noexcept {
#ifdef DEBUG
    assert(CheckState());
#endif
    auto it = Map.find(key);
    return it == Map.end() ? nullptr : &it->second->Value;
}

template<typename TKey, typename TValue, size_t Capacity>
void TLRUCache<TKey, TValue, Capacity>::Put(const TKey& key, TValue value) {
#ifdef DEBUG
    assert(CheckState());
#endif

    auto it = Map.find(key);
    if (it != Map.end()) {
        auto listIter = it->second;
        listIter->Value = std::move(value);
        if (listIter != List.begin()) {
            List.splice(List.begin(), List, listIter, std::next(listIter));
        }
        return;
    }

    if (Map.size() == Capacity) {
        DropLRU();
    }

    List.template emplace_back(key, std::move(value));
    Map[key] = std::prev(List.end());

#ifdef DEBUG
    assert(CheckState());
#endif
}

template<typename TKey, typename TValue, size_t Capacity>
void TLRUCache<TKey, TValue, Capacity>::DropLRU() {
#ifdef DEBUG
    assert(CheckState());
#endif
    Map.erase(List.front().Key);
    List.pop_front();
#ifdef DEBUG
    assert(CheckState());
#endif
}

#ifdef DEBUG
template<typename TKey, typename TValue, size_t Capacity>
bool TLRUCache<TKey, TValue, Capacity>::CheckState() const {
    for (const auto& node : List) {
        if (!(Map.count(node.Key))) {
            return false;
        }
    }
    return true;
}
#endif
