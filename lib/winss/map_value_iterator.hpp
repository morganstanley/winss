/*
 * Copyright 2016-2017 Morgan Stanley
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIB_WINSS_MAP_VALUE_ITERATOR_HPP_
#define LIB_WINSS_MAP_VALUE_ITERATOR_HPP_

#include <map>

namespace winss {
/**
 * Use std::map as the map.
 *
 * \tparam Key The type of map key.
 * \tparam Value The type of map value.
 */
template<typename Key, typename Value>
using Map = std::map<Key, Value>;

/**
 * Use std::map::iterator as the map iterator.
 *
 * \tparam Key The type of map key.
 * \tparam Value The type of map value.
 */
template<typename Key, typename Value>
using MapIterator = typename Map<Key, Value>::iterator;

/**
 * An iterator for keys of a map.
 *
 * \tparam Key The type of map key.
 * \tparam Value The type of map value.
 */
template<typename Key, typename Value>
class MapKeyIterator : public MapIterator<Key, Value> {
 public:
    /**
     * Construct a new map key iterator.
     */
    MapKeyIterator() : MapIterator<Key, Value>() {}

    /**
     * Construct a map key iterator using a base iterator.
     *
     * \param it The map iterator.
     */
    explicit MapKeyIterator(MapIterator<Key, Value> it) :
        MapIterator<Key, Value>(it) {}

    /**
     * Get a pointer to the current key.
     *
     * \return A pointer to the key.
     */  
    Key* operator->() {
        return (Key* const)&(MapIterator<Key, Value>::operator->()->first);
    }

    /**
     * Get the dereferenced current key.
     *
     * \return The key.
     */  
    Key operator*() {
        return MapIterator<Key, Value>::operator*().first;
    }
};

/**
 * An iterator for values of a map.
 *
 * \tparam Key The type of map key.
 * \tparam Value The type of map value.
 */
template<typename Key, typename Value>
class MapValueIterator : public MapIterator<Key, Value> {
 public:
    /**
     * Construct a new map value iterator.
     */
    MapValueIterator() : MapIterator<Key, Value>() {}

    /**
     * Construct a map value iterator using a base iterator.
     *
     * \param it The map iterator.
     */
    explicit MapValueIterator(MapIterator<Key, Value> it) :
        MapIterator<Key, Value>(it) {}

    /**
     * Get a pointer to the current value.
     *
     * \return A pointer to the value.
     */  
    Value* operator->() {
        return (Value* const)&(MapIterator<Key, Value>::operator->()->second);
    }

    /**
     * Get the dereferenced current value.
     *
     * \return The value.
     */
    Value operator*() {
        return MapIterator<Key, Value>::operator*().second;
    }
};
}  // namespace winss

#endif  // LIB_WINSS_MAP_VALUE_ITERATOR_HPP_
