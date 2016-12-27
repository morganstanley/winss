#ifndef LIB_WINSS_MAP_VALUE_ITERATOR_HPP_
#define LIB_WINSS_MAP_VALUE_ITERATOR_HPP_

#include <map>

namespace winss {
template<typename Key, typename Value>
using Map = std::map<Key, Value>;

template<typename Key, typename Value>
using MapIterator = typename Map<Key, Value>::iterator;

template<typename Key, typename Value>
class MapKeyIterator : public MapIterator<Key, Value> {
 public:
    MapKeyIterator() : MapIterator<Key, Value>() {}
    explicit MapKeyIterator(MapIterator<Key, Value> it) :
        MapIterator<Key, Value>(it) {}

    Key* operator->() {
        return (Key* const)&(MapIterator<Key, Value>::operator->()->first);
    }
    Key operator*() {
        return MapIterator<Key, Value>::operator*().first;
    }
};

template<typename Key, typename Value>
class MapValueIterator : public MapIterator<Key, Value> {
 public:
    MapValueIterator() : MapIterator<Key, Value>() {}
    explicit MapValueIterator(MapIterator<Key, Value> it) :
        MapIterator<Key, Value>(it) {}

    Value* operator->() {
        return (Value* const)&(MapIterator<Key, Value>::operator->()->second);
    }
    Value operator*() {
        return MapIterator<Key, Value>::operator*().second;
    }
};
}  // namespace winss

#endif  // LIB_WINSS_MAP_VALUE_ITERATOR_HPP_
