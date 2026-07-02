#pragma once

#include "utility.hpp"
#include "functional.hpp"
#include "allocator.hpp"
#include "memory.hpp"
#include <cstddef>
#include <stdexcept>

namespace mystl 
{

    template <
        typename Key, 
        typename Hash      = mystl::hash<Key>, 
        typename KeyEqual  = mystl::equal_to,
        typename Allocator = mystl::Allocator<Key>
    >
    class UnorderedSet 
    {
    public:
        using key_type        = Key;
        using value_type      = Key; 
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher          = Hash;
        using key_equal       = KeyEqual;
        using allocator_type  = Allocator;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using pointer         = typename mystl::allocator_traits<Allocator>::pointer;
        using const_pointer   = typename mystl::allocator_traits<Allocator>::const_pointer;

    private:
        struct Node 
        {
            value_type value;
            Node* next;

            template <typename... Args>
            Node(Node* n, Args&&... args) 
                : value(mystl::forward<Args>(args)...)
                , next(n) 
            {
            }
        };

        using node_allocator_type   = typename mystl::allocator_traits<Allocator>::template rebind_alloc<Node>;
        using node_traits           = mystl::allocator_traits<node_allocator_type>;
        using bucket_allocator_type = typename mystl::allocator_traits<Allocator>::template rebind_alloc<Node*>;
        using bucket_traits         = mystl::allocator_traits<bucket_allocator_type>;

        Node** buckets_         = nullptr;
        size_type bucket_count_    = 0;
        size_type size_            = 0;
        float     max_load_factor_ = 1.0f;

        [[no_unique_address]] node_allocator_type   node_alloc_;
        [[no_unique_address]] bucket_allocator_type bucket_alloc_;
        [[no_unique_address]] hasher                hash_func_;
        [[no_unique_address]] key_equal             equal_func_;

        // ========================================================================
        // PRIVATE ALLOCATION HELPERS
        // ========================================================================
        size_type get_bucket_index(const Key& key, size_type b_count) const 
        {
            return hash_func_(key) % b_count;
        }

        Node** allocate_buckets(size_type n)
        {
            Node** p = bucket_traits::allocate(bucket_alloc_, n);
            for (size_type i = 0; i < n; ++i) p[i] = nullptr;
            return p;
        }

        void deallocate_buckets(Node** p, size_type n) noexcept
        {
            bucket_traits::deallocate(bucket_alloc_, p, n);
        }

        template <typename... Args>
        Node* create_node(Node* next, Args&&... args)
        {
            Node* p = node_traits::allocate(node_alloc_, 1);
            try
            {
                node_traits::construct(node_alloc_, p, next, mystl::forward<Args>(args)...);
            }
            catch (...)
            {
                node_traits::deallocate(node_alloc_, p, 1);
                throw;
            }
            return p;
        }

        void destroy_node(Node* p) noexcept
        {
            node_traits::destroy(node_alloc_, p);
            node_traits::deallocate(node_alloc_, p, 1);
        }

        // Allocator-extended move helper: take over the source's storage when our
        // allocator can free it (always-equal, or compares equal), otherwise move
        // the elements into storage owned by our allocator so the source's memory
        // is never released through the wrong allocator.
        void adopt_or_move(UnorderedSet& other)
        {
            bool take_ownership;
            if constexpr (node_traits::is_always_equal::value)
                take_ownership = true;
            else
                take_ownership = (node_alloc_ == other.node_alloc_);

            if (take_ownership)
            {
                buckets_ = other.buckets_;
                size_    = other.size_;
                other.buckets_      = nullptr;
                other.bucket_count_ = 0;
                other.size_         = 0;
                return;
            }

            buckets_ = allocate_buckets(bucket_count_);
            try
            {
                for (size_type i = 0; i < other.bucket_count_; ++i)
                {
                    Node* curr = other.buckets_[i];
                    Node* prev = nullptr;
                    while (curr)
                    {
                        Node* new_node = create_node(nullptr, mystl::move(curr->value));
                        if (!prev) buckets_[i] = new_node;
                        else       prev->next  = new_node;
                        prev = new_node;
                        curr = curr->next;
                        ++size_;
                    }
                }
            }
            catch (...)
            {
                clear();
                deallocate_buckets(buckets_, bucket_count_);
                buckets_ = nullptr;
                throw;
            }
            other.clear();
        }

    public:
        // ========================================================================
        // ITERATORS
        // ========================================================================
        class ConstIterator 
        {
            friend class UnorderedSet;
        public:
            using iterator_category = mystl::forward_iterator_tag;
            using value_type        = UnorderedSet::value_type;
            using difference_type   = UnorderedSet::difference_type;
            using pointer           = UnorderedSet::const_pointer;
            using reference         = UnorderedSet::const_reference;

        private:
            const Node* node_;
            size_type           bucket_idx_;
            const UnorderedSet* set_;

            ConstIterator(const Node* node, size_type b_idx, const UnorderedSet* set)
                : node_(node)
                , bucket_idx_(b_idx)
                , set_(set) 
            {
            }

        public:
            ConstIterator() : node_(nullptr), bucket_idx_(0), set_(nullptr) {}

            const_reference   operator*()  const { return node_->value; }
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

        // For std::unordered_set, iterator is a constant iterator.
        using iterator       = ConstIterator;
        using const_iterator = ConstIterator;

        iterator       begin()  const noexcept { return cbegin(); }
        iterator       end()    const noexcept { return cend(); }
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
        // CONSTRUCTORS, DESTRUCTOR, RULE OF FIVE
        // ========================================================================
        UnorderedSet() : UnorderedSet(8) {}
        
        explicit UnorderedSet(size_type bucket_count, 
                              const hasher&    hf  = hasher(), 
                              const key_equal& eql = key_equal())
            : bucket_count_(bucket_count)
            , hash_func_(hf)
            , equal_func_(eql)
        {
            buckets_ = allocate_buckets(bucket_count_);
        }

        ~UnorderedSet() 
        {
            clear();
            if (buckets_)
                deallocate_buckets(buckets_, bucket_count_);
        }

        UnorderedSet(const UnorderedSet& other)
            : bucket_count_(other.bucket_count_)
            , max_load_factor_(other.max_load_factor_)
            , node_alloc_(node_traits::select_on_container_copy_construction(other.node_alloc_))
            , bucket_alloc_(bucket_traits::select_on_container_copy_construction(other.bucket_alloc_))
            , hash_func_(other.hash_func_)
            , equal_func_(other.equal_func_)
        {
            buckets_ = allocate_buckets(bucket_count_);
            for (size_type i = 0; i < other.bucket_count_; ++i) 
            {
                Node* curr = other.buckets_[i];
                Node* prev = nullptr;
                while (curr) 
                {
                    Node* new_node = create_node(nullptr, curr->value);
                    if (!prev) buckets_[i] = new_node;
                    else prev->next = new_node;
                    prev = new_node;
                    curr = curr->next;
                    ++size_;
                }
            }
        }

        UnorderedSet(const UnorderedSet& other, const allocator_type& alloc)
            : bucket_count_(other.bucket_count_)
            , max_load_factor_(other.max_load_factor_)
            , node_alloc_(alloc)
            , hash_func_(other.hash_func_)
            , equal_func_(other.equal_func_)
        {
            buckets_ = allocate_buckets(bucket_count_);
            for (size_type i = 0; i < other.bucket_count_; ++i) 
            {
                Node* curr = other.buckets_[i];
                Node* prev = nullptr;
                while (curr) 
                {
                    Node* new_node = create_node(nullptr, curr->value);
                    if (!prev) buckets_[i] = new_node;
                    else prev->next = new_node;
                    prev = new_node;
                    curr = curr->next;
                    ++size_;
                }
            }
        }

        UnorderedSet(UnorderedSet&& other) noexcept
            : buckets_(other.buckets_)
            , bucket_count_(other.bucket_count_)
            , size_(other.size_)
            , max_load_factor_(other.max_load_factor_)
            , node_alloc_(mystl::move(other.node_alloc_))
            , bucket_alloc_(mystl::move(other.bucket_alloc_))
            , hash_func_(mystl::move(other.hash_func_))
            , equal_func_(mystl::move(other.equal_func_))
        {
            other.buckets_      = nullptr;
            other.bucket_count_ = 0;
            other.size_         = 0;
        }

        UnorderedSet(UnorderedSet&& other, const allocator_type& alloc)
            : bucket_count_(other.bucket_count_)
            , max_load_factor_(other.max_load_factor_)
            , node_alloc_(alloc)
            , bucket_alloc_(alloc)
            , hash_func_(mystl::move(other.hash_func_))
            , equal_func_(mystl::move(other.equal_func_))
        {
            adopt_or_move(other);
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
                if (buckets_) deallocate_buckets(buckets_, bucket_count_);

                buckets_         = other.buckets_;
                bucket_count_    = other.bucket_count_;
                size_            = other.size_;
                max_load_factor_ = other.max_load_factor_;
                node_alloc_      = mystl::move(other.node_alloc_);
                bucket_alloc_    = mystl::move(other.bucket_alloc_);
                hash_func_       = mystl::move(other.hash_func_);
                equal_func_      = mystl::move(other.equal_func_);

                other.buckets_      = nullptr;
                other.bucket_count_ = 0;
                other.size_         = 0;
            }
            return *this;
        }

        // ========================================================================
        // SIZE AND STATE
        // ========================================================================
        [[nodiscard]] bool      empty()        const noexcept { return size_ == 0; }
        [[nodiscard]] size_type size()         const noexcept { return size_; }
        [[nodiscard]] size_type bucket_count() const noexcept { return bucket_count_; }
        
        [[nodiscard]] float load_factor() const noexcept 
        { 
            return bucket_count_ == 0 ? 0.0f : static_cast<float>(size_) / bucket_count_; 
        }
        
        [[nodiscard]] float          max_load_factor() const noexcept { return max_load_factor_; }
        [[nodiscard]] allocator_type get_allocator()   const noexcept { return allocator_type(node_alloc_); }

        void clear() noexcept 
        {
            for (size_type i = 0; i < bucket_count_; ++i) 
            {
                Node* curr = buckets_[i];
                while (curr) 
                {
                    Node* next = curr->next;
                    destroy_node(curr);
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
            size_type   idx  = get_bucket_index(key, bucket_count_);
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
            
            Node** new_buckets = allocate_buckets(new_count);
            
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
            
            if (buckets_) deallocate_buckets(buckets_, bucket_count_);
            buckets_ = new_buckets;
            bucket_count_ = new_count;
        }

        template <typename... Args>
        mystl::Pair<iterator, bool> emplace(Args&&... args) 
        {
            Node* new_node = create_node(nullptr, mystl::forward<Args>(args)...);
            const Key& key = new_node->value;

            iterator it = find(key);
            if (it != end()) 
            {
                destroy_node(new_node); 
                return { it, false };
            }

            if (load_factor() + 1.0f / (bucket_count_ == 0 ? 1 : bucket_count_) > max_load_factor_) 
            {
                rehash(bucket_count_ == 0 ? 8 : bucket_count_ * 2);
            }

            size_type idx = get_bucket_index(key, bucket_count_);
            new_node->next = buckets_[idx];
            buckets_[idx] = new_node;
            ++size_;

            return {iterator(new_node, idx, this), true};
        }

        mystl::Pair<iterator, bool> insert(const value_type& value) { return emplace(value); }
        mystl::Pair<iterator, bool> insert(value_type&& value)      { return emplace(mystl::move(value)); }

        size_type erase(const Key& key) 
        {
            if (bucket_count_ == 0) return 0;
            size_type idx  = get_bucket_index(key, bucket_count_);
            Node* curr = buckets_[idx];
            Node* prev = nullptr;

            while (curr) 
            {
                if (equal_func_(curr->value, key)) 
                {
                    if (!prev) buckets_[idx] = curr->next;
                    else prev->next = curr->next;
                    destroy_node(curr);
                    --size_;
                    return 1;
                }
                prev = curr;
                curr = curr->next;
            }
            return 0;
        }

        void swap(UnorderedSet& other) noexcept 
        {
            mystl::swap(buckets_,         other.buckets_);
            mystl::swap(bucket_count_,    other.bucket_count_);
            mystl::swap(size_,            other.size_);
            mystl::swap(max_load_factor_, other.max_load_factor_);
            mystl::swap(node_alloc_,      other.node_alloc_);
            mystl::swap(bucket_alloc_,    other.bucket_alloc_);
            mystl::swap(hash_func_,       other.hash_func_);
            mystl::swap(equal_func_,      other.equal_func_);
        }
    };

} // namespace mystl