/*
 * =====================================================================================
 *
 *       Filename:  lru_cache.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2018年02月19日 16时37分46秒
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  mail.yangjun@qq.com 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef UTILITY_LRU_CACHE_HPP_
#define UTILITY_LRU_CACHE_HPP_

#include <list>
#include <memory>
#include <unordered_map>
#include <utility>

namespace utility {

template<class Key, 
         class T, 
         class Hash = std::hash<Key>, 
         class KeyEqual = std::equal_to<Key>, 
         class Allocator = std::allocator<std::pair<const Key, T> > >
class lru_cache {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const Key, T> value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef Hash hasher;
    typedef Allocator allocator_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::allocator_traits<Allocator>::pointer pointer;
    typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;

    typedef std::list<std::pair<Key, T>, Allocator> list;
    typedef std::unordered_map<Key, 
                               typename list::iterator, 
                               Hash, 
                               KeyEqual, 
                               typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const Key, typename list::iterator> > > map;
    typedef typename list::iterator iterator;
    typedef typename list::const_iterator const_iterator;
    typedef typename list::reverse_iterator reverse_iterator;
    typedef typename list::const_reverse_iterator const_reverse_iterator;

    lru_cache() 
        : capacity_(8) {
    }

    explicit lru_cache(size_type capacity, 
                      const Hash& hash = Hash(), 
                      const KeyEqual& equal = KeyEqual(), 
                      const Allocator& alloc = Allocator())
        : list_(alloc), 
          map_(capacity, hash, equal, alloc),  
          capacity_(capacity) {
    }

    explicit lru_cache(const Allocator& alloc)
        : list_(alloc), 
          map_(alloc), 
          capacity_(8) {
    }

    template<class InputIt>
    lru_cache(InputIt first, InputIt last, 
             size_type capacity = 8, 
             const Hash& hash = Hash(), 
             const KeyEqual& equal = KeyEqual(), 
             const Allocator& alloc = Allocator())
        : list_(alloc), 
          map_(capacity, hash, equal, alloc), 
          capacity_(capacity) {
        insert(first, last);
    }

    lru_cache(const lru_cache& other) 
        : list_(other.list_), 
          map_(other.map_), 
          capacity_(other.capacity_) {
    }

    lru_cache(const lru_cache& other, const Allocator& alloc)
        : list_(other.list_, alloc), 
          map_(other.map_, alloc), 
          capacity_(other.capacity_) {
    }

    lru_cache(lru_cache&& other)
        : list_(std::move(other.list_)), 
          map_(std::move(other.map_)),
          capacity_(std::move(other.capacity_)) {
    }

    lru_cache(lru_cache&& other, const Allocator& alloc) 
        : list_(std::move(other.list_), alloc), 
          map_(std::move(other.map_), alloc), 
          capacity_(std::move(other.capacity_)) {
    }

    lru_cache(std::initializer_list<value_type> init, 
             size_type capacity = 8, 
             const Hash& hash = Hash(), 
             const KeyEqual& equal = KeyEqual(), 
             const Allocator& alloc = Allocator())
        : list_(alloc), 
          map_(capacity, hash, equal, alloc), 
          capacity_(capacity) {
        insert(init);
    }

    ~lru_cache() {
    }

    lru_cache& operator =(const lru_cache& other) {
        list_ = other.list_;
        map_ = other.map_;
        capacity_ = other.capacity_;
        return *this;
    } 

    lru_cache& operator =(lru_cache&& other) {
        list_ = std::move(other.list_);
        map_ = std::move(other.map_);
        capacity_ = std::move(other.capacity_);
        return *this;
    }

    lru_cache& operator =(std::initializer_list<value_type> ilist) {
        clear();
        insert(ilist);
        return *this;
    }

    allocator_type get_allocator() const { return list_.get_allocator(); }

    iterator begin() noexcept { return list_.begin(); }
    const_iterator begin() const noexcept { return list_.begin(); }
    const_iterator cbegin() const noexcept { return list_.cbegin(); }

    iterator end() noexcept { return list_.end(); }
    const_iterator end() const noexcept { return list_.end(); }
    const_iterator cend() const noexcept { return list_.cend(); }

    reverse_iterator rbegin() noexcept { return list_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return list_.rbegin(); }
    const_reverse_iterator crbegin() const noexcept { return list_.crbegin(); }

    reverse_iterator rend() noexcept { return list_.end(); }
    const_reverse_iterator rend() const noexcept { return list_.rend(); }
    const_reverse_iterator crend() const noexcept { return list_.crend(); }

    bool empty() const noexcept { return list_.empty(); }

    size_type size() const noexcept { return map_.size(); }

    size_type max_size() const noexcept { return std::min(list_.max_size, map_.max_size); }

    void reserve(size_type new_cap) { 
        capacity_ = new_cap; 
        map_.reserve(new_cap);
        while (map_.size() > capacity_) {
            erase(list_.begin());
        }
    }

    size_type capacity() const noexcept { return capacity_; };

    void clear() noexcept {
        map_.clear();
        list_.clear();
    };

    std::pair<iterator, bool> insert(const value_type& value) {
        auto list_it = list_.insert(list_.end(), value);
        auto map_insert_result = map_.insert(std::make_pair(value.first, list_it));
        if (!map_insert_result.second) {
            list_.erase(list_it);
            return std::make_pair(map_insert_result.first->second, false);
        }
        
        while (map_.size() > capacity_) {
            erase(list_.begin());
        }
        
        return std::make_pair(list_it, true);
    }

    template<class P>
    std::pair<iterator, bool> insert(P&& value) { 
        return emplace(std::forward<P>(value)); 
    }

    std::pair<iterator, bool> insert(const_iterator hint, const value_type& value) { 
        return emplace_hint(hint, value); 
    }

    template<class P>
    std::pair<iterator, bool> insert(const_iterator hint, value_type&& value) { 
        return emplace_hint(hint, std::forward<P>(value)); 
    }

    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        for (InputIt it = first; it != last; ++it) {
            insert(*it);
        }
    }

    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        auto list_it = list_.emplace(list_.end(), std::forward<Args>(args)...);
        auto map_insert_result = map_.emplace(std::make_pair(list_it->first, list_it));
        if (!map_insert_result.second) {
            list_.erase(list_it);
            return std::make_pair(map_insert_result.first->second, false);
        }
        
        while (map_.size() > capacity_) {
            erase(list_.begin());
        }
        
        return std::make_pair(list_it, true);
    }

    template<class... Args>
    std::pair<iterator, bool> emplace_hint(const_iterator hint, Args&&... args) {
        auto list_it = list_.emplace(list_.end(), std::forward<Args>(args)...);
        auto map_insert_result = map_.emplace_hint(map_.find(hint.first), std::make_pair(list_it->first, list_it));
        if (!map_insert_result.second) {
            list_.erase(list_it);
            return std::make_pair(map_insert_result.first->second, false);
        }
        
        while (map_.size() > capacity_) {
            erase(list_.begin());
        }
        
        return std::make_pair(list_it, true);
    }

    iterator erase(const_iterator pos) {
        map_.erase(pos->first); 
        return list_.erase(pos);
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator result;
        for (auto it = first; it != last; ++it) {
            result = erase(it);
        }
        return result;
    }

    size_type erase(const key_type& key) {
        auto map_it = map_.find(key);
        if (map_.end() == map_it) {
            return 0;
        }

        list_.erase(map_it->second);
        return map_.erase(key);
    }

    void swap(lru_cache& other) {
        std::swap(list_, other.list_);
        std::swap(map_, other.map_);
        std::swap(capacity_, other.capacity_);
    }

    T& at(const Key& key) { return map_.at(key)->second; }

    const T& at(const Key& key) const { return map_.at(key)->second; }

    T& operator [](const Key& key) { 
        auto map_it = map_.find(key);
        if (map_.end() != map_it) {
            auto list_it = list_.insert(list_.end(), *map_it->second);
            list_.erase(map_it->second);
            map_it->second = list_it;
            return list_it->second; 
        }

        auto list_it = list_.insert(list_.end(), std::make_pair(key, T()));
        auto map_insert_result = map_.insert(std::make_pair(key, list_it));
        if (!map_insert_result.second) {
            list_.erase(list_it);
            return map_insert_result.first->second->second;
        }

        while (map_.size() > capacity_) {
            erase(list_.begin());
        }

        return list_it->second;
    }

    T& operator [](Key&& key) {
        auto map_it = map_.find(key);
        if (map_.end() != map_it) {
            auto list_it = list_.insert(list_.end(), *map_it->second);
            list_.erase(map_it->second);
            map_it->second = list_it;
            return list_it->second;
        }

        auto list_it = list_.insert(list_.end(), std::make_pair(key, T()));
        auto map_insert_result = map_.insert(std::make_pair(key, list_it));
        if (!map_insert_result.second) {
            list_.erase(list_it);
            return map_insert_result.first->second->second;
        }

        while (map_.size() > capacity_) {
            erase(list_.begin());
        }

        return list_it->second;
    }

    size_type count(const Key& key) const { return map_.count(key); }

    template<class K>
    size_type count(const K& x) const { return map_.count(x); }

    iterator find(const Key& key) { 
        auto map_it = map_.find(key);
        if (map_.end() == map_it) {
            return list_.end();
        } else {
            auto list_it = list_.insert(list_.end(), *map_it->second);
            list_.erase(map_it->second);
            map_it->second = list_it;
            return list_it;
        }
    }

    const_iterator find(const Key& key) const {
        auto map_it = map_.find(key);
        if (map_.end() == map_it) {
            return list_.end();
        } else {
            return map_it->second;
        }
    }

    std::pair<iterator, iterator> equal_range(const Key& key) {
        auto map_equal_range_result = map_.equal_range(key);
        for (auto map_it = map_equal_range_result.first; 
                map_it != map_equal_range_result.second 
                && map_it != map_.end(); ++map_it) {
            auto list_it = list_.insert(list_.end(), *map_it->second);
            list_.erase(map_it->second);
            map_it->second = list_it;
        }

        return std::make_pair(map_equal_range_result.first->second, list_.end());
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
        auto map_equal_range_result = map_.equal_range(key);
        if (map_.end() != map_equal_range_result.second) {
            return std::make_pair(map_equal_range_result.first->second, 
                                  map_equal_range_result.second->second);
        } else {
            return std::make_pair(map_equal_range_result.first->second, 
                                  list_.end());
        }
    }

private:
    list list_;
    map map_;
    size_type capacity_;
};

} // namespace utility

#endif // UTILITY_LRU_CACHE_HPP_
