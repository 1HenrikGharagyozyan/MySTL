#pragma once

#include "utility.hpp"
#include <cstddef>
#include <functional>
#include <cassert>

namespace mystl 
{
    // We use the same default hashers as for the map (this can be moved to a shared file later)
    template <typename Key>
    struct hash_set 
    {
        size_t operator()(const Key& key) const { return std::hash<Key>{}(key); }
    };

    template <typename T>
    struct equal_to_set 
    {
        constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
    };

    template <
        typename Key, 
        typename Hash = mystl::hash_set<Key>, 
        typename KeyEqual = mystl::equal_to_set<Key>
    >
    class UnorderedSet 
    {
    public:
        using key_type        = Key;
        using value_type      = Key; // In a set, the key and value are the same
        using size_type       = std::size_t;
        using hasher          = Hash;
        using key_equal       = KeyEqual;
        using reference       = value_type&;
        using const_reference = const value_type&;

    private:
        struct Node 
        {
            value_type value; // We store only the key, saving memory!
            Node* next;

            template <typename... Args>
            Node(Node* n, Args&&... args) 
                : value(mystl::forward<Args>(args)...), next(n) {}
        };

        Node** buckets_         = nullptr;
        size_type bucket_count_    = 0;
        size_type size_            = 0;
        float     max_load_factor_ = 1.0f;
        hasher    hash_func_;
        key_equal equal_func_;

        size_type get_bucket_index(const Key& key, size_type b_count) const 
        {
            return hash_func_(key) % b_count;
        }

    public:
        // ========================================================================
        // ITERATORS (Smart Iterator)
        // ========================================================================
        class ConstIterator 
        {
            friend class UnorderedSet;
        private:
            const Node* node_;
            size_type bucket_idx_;
            const UnorderedSet* set_;

            ConstIterator(const Node* node, size_type b_idx, const UnorderedSet* set)
                : node_(node)
                , bucket_idx_(b_idx)
                , set_(set) 
            {
            }

        public:
            const_reference operator*() const { return node_->value; }
            const value_type* operator->() const { return &(node_->value); }

            ConstIterator& operator++() 
            {
                node_ = node_->next;
                if (!node_) 
                {
                    while (++bucket_idx_ < set_->bucket_count_) 
                    {
                        if (set_->buckets_[bucket_idx_]) 
                        {
                            node_ = set_->buckets_[bucket_idx_];
                            break;
                        }
                    }
                }
                return *this;
            }

            ConstIterator operator++(int) 
            {
                ConstIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
            bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }
        };

        // In std::unordered_set, iterator and const_iterator are the same, 
        // so the user cannot modify the key through a reference.
        using iterator = ConstIterator;
        using const_iterator = ConstIterator;

        iterator begin() const noexcept { return cbegin(); }
        iterator end() const noexcept { return cend(); }
        const_iterator cbegin() const noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                if (buckets_[i]) return const_iterator(buckets_[i], i, this);
            }
            return cend();
        }
        const_iterator cend() const noexcept { return const_iterator(nullptr, bucket_count_, this); }

        // ========================================================================
        // LIFETIME (Rule of Five)
        // ========================================================================
        UnorderedSet() : UnorderedSet(8) {}
        
        explicit UnorderedSet(size_type bucket_count, const hasher& hf = hasher(), const key_equal& eql = key_equal())
            : bucket_count_(bucket_count)
            , hash_func_(hf)
            , equal_func_(eql)
        {
            buckets_ = new Node*[bucket_count_](); 
        }

        ~UnorderedSet() 
        {
            clear();
            delete[] buckets_;
        }

        UnorderedSet(const UnorderedSet& other)
            : bucket_count_(other.bucket_count_)
            , max_load_factor_(other.max_load_factor_)
            , hash_func_(other.hash_func_)
            , equal_func_(other.equal_func_)
        {
            buckets_ = new Node*[bucket_count_]();
            for (size_type i = 0; i < other.bucket_count_; ++i) 
            {
                Node* curr = other.buckets_[i];
                Node* prev = nullptr;
                while (curr) 
                {
                    Node* new_node = new Node(nullptr, curr->value);
                    if (!prev) buckets_[i] = new_node;
                    else prev->next = new_node;
                    prev = new_node;
                    curr = curr->next;
                    size_++;
                }
            }
        }

        UnorderedSet(UnorderedSet&& other) noexcept
            : buckets_(other.buckets_)
            , bucket_count_(other.bucket_count_)
            , size_(other.size_)
            , max_load_factor_(other.max_load_factor_)
            , hash_func_(mystl::move(other.hash_func_))
            , equal_func_(mystl::move(other.equal_func_))
        {
            other.buckets_ = nullptr;
            other.bucket_count_ = 0;
            other.size_ = 0;
        }

        UnorderedSet& operator=(const UnorderedSet& other) 
        {
            if (this != &other) 
            {
                UnorderedSet tmp(other);
                swap(tmp);
            }
            return *this;
        }

        UnorderedSet& operator=(UnorderedSet&& other) noexcept 
        {
            if (this != &other) 
            {
                clear();
                delete[] buckets_;
                buckets_ = other.buckets_;
                bucket_count_ = other.bucket_count_;
                size_ = other.size_;
                max_load_factor_ = other.max_load_factor_;
                other.buckets_ = nullptr;
                other.bucket_count_ = 0;
                other.size_ = 0;
            }
            return *this;
        }

        // ========================================================================
        // SIZE AND STATE
        // ========================================================================
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
        [[nodiscard]] size_type size() const noexcept { return size_; }
        [[nodiscard]] float load_factor() const noexcept { return bucket_count_ == 0 ? 0.0f : static_cast<float>(size_) / bucket_count_; }
        void clear() noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                Node* curr = buckets_[i];
                while (curr) 
                {
                    Node* next = curr->next;
                    delete curr;
                    curr = next;
                }
                buckets_[i] = nullptr;
            }
            size_ = 0;
        }

        // ========================================================================
        // SEARCH
        // ========================================================================
        const_iterator find(const Key& key) const 
        {
            if (bucket_count_ == 0) return cend();
            size_type idx = get_bucket_index(key, bucket_count_);
            const Node* curr = buckets_[idx];
            while (curr) 
            {
                if (equal_func_(curr->value, key)) return const_iterator(curr, idx, this);
                curr = curr->next;
            }
            return cend();
        }

        bool contains(const Key& key) const 
        {
            return find(key) != cend();
        }

        // ========================================================================
        // MODIFIERS
        // ========================================================================
        void rehash(size_type new_count) 
        {
            if (new_count <= bucket_count_) return;
            Node** new_buckets = new Node*[new_count]();
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                Node* curr = buckets_[i];
                while (curr) 
                {
                    Node* next_node = curr->next;
                    size_type new_idx = get_bucket_index(curr->value, new_count);
                    curr->next = new_buckets[new_idx];
                    new_buckets[new_idx] = curr;
                    curr = next_node;
                }
            }
            delete[] buckets_;
            buckets_ = new_buckets;
            bucket_count_ = new_count;
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> emplace(Args&&... args) 
        {
            Node* new_node = new Node(nullptr, mystl::forward<Args>(args)...);
            const Key& key = new_node->value;

            iterator it = find(key);
            if (it != end()) 
            {
                delete new_node; 
                return { it, false };
            }

            if (load_factor() + 1.0f / (bucket_count_ == 0 ? 1 : bucket_count_) > max_load_factor_) 
            {
                rehash(bucket_count_ == 0 ? 8 : bucket_count_ * 2);
            }

            size_type idx = get_bucket_index(key, bucket_count_);
            new_node->next = buckets_[idx];
            buckets_[idx] = new_node;
            size_++;

            return {iterator(new_node, idx, this), true};
        }

        mystl::Pair<iterator, bool> insert(const value_type& value) { return emplace(value); }
        mystl::Pair<iterator, bool> insert(value_type&& value) { return emplace(mystl::move(value)); }

        size_type erase(const Key& key) 
        {
            if (bucket_count_ == 0) return 0;
            size_type idx = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];
            Node* prev = nullptr;

            while (curr) 
            {
                if (equal_func_(curr->value, key)) 
                {
                    if (!prev) buckets_[idx] = curr->next;
                    else prev->next = curr->next;
                    delete curr;
                    size_--;
                    return 1;
                }
                prev = curr;
                curr = curr->next;
            }
            return 0;
        }

        void swap(UnorderedSet& other) noexcept 
        {
            mystl::swap(buckets_, other.buckets_);
            mystl::swap(bucket_count_, other.bucket_count_);
            mystl::swap(size_, other.size_);
            mystl::swap(max_load_factor_, other.max_load_factor_);
        }
    };

} // namespace mystl