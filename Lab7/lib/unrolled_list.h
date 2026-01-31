#include <memory>
#include <iterator>
#include <stdexcept>
#include <cstddef>
#include <initializer_list>
#include <algorithm>
#include <new>
#include <type_traits>

template <typename T, size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;

private:

    struct Node {
        Node* prev;
        Node* next;
        size_type count;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage[NodeMaxSize];

        Node() : prev(nullptr), next(nullptr), count(0) { }


        T* ptr_at(size_type i) {
            return reinterpret_cast<T*>(&storage[i]);
        }
        const T* ptr_at(size_type i) const {
            return reinterpret_cast<const T*>(&storage[i]);
        }
    };


    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

    Node* first;
    Node* last;
    size_type total_size;
    NodeAllocator node_allocator;

    Node* create_node() {
        Node* node = NodeAllocTraits::allocate(node_allocator, 1);
        try {
            NodeAllocTraits::construct(node_allocator, node);
        } catch (...) {
            NodeAllocTraits::deallocate(node_allocator, node, 1);
            throw;
        }
        return node;
    }

    void destroy_node(Node* node) noexcept {
        for (size_type i = 0; i < node->count; ++i) {
            node->ptr_at(i)->~T();
        }
        NodeAllocTraits::destroy(node_allocator, node);
        NodeAllocTraits::deallocate(node_allocator, node, 1);
    }

    void split_node(Node* node) {
        Node* new_node = create_node();
        new_node->prev = node;
        new_node->next = node->next;
        if (new_node->next)
            new_node->next->prev = new_node;
        else
            last = new_node;
        node->next = new_node;

        size_type half = node->count / 2;
        for (size_type i = half; i < node->count; ++i) {
            new (new_node->ptr_at(new_node->count)) T(*node->ptr_at(i));
            ++new_node->count;
            node->ptr_at(i)->~T();
        }
        node->count = half;
    }

public:
    class iterator {
        friend class unrolled_list;
        Node* node;
        size_type index;
        Node* tail_ref;

        iterator(Node* n, size_type i, Node* tail) noexcept
                : node(n), index(i), tail_ref(tail) { }
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        reference operator*() const {
            return *node->ptr_at(index);
        }
        pointer operator->() const {
            return node->ptr_at(index);
        }

        iterator& operator++() {
            if (node) {
                if (++index >= node->count) {
                    node = node->next;
                    index = 0;
                }
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }
        iterator& operator--() {
            if (node) {
                if (index == 0) {
                    node = node->prev;
                    if (node)
                        index = node->count - 1;
                } else {
                    --index;
                }
            } else { 
                node = tail_ref;
                if (node)
                    index = node->count - 1;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --*this;
            return tmp;
        }
        bool operator==(const iterator& other) const {
            return node == other.node && index == other.index;
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    class const_iterator {
        friend class unrolled_list;
        const Node* node;
        size_type index;
        const Node* tail_ref;
        const_iterator(const Node* n, size_type i, const Node* tail) noexcept
                : node(n), index(i), tail_ref(tail) { }
    public:
        using value_type = const T;
        using difference_type = ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        reference operator*() const {
            return *node->ptr_at(index);
        }
        pointer operator->() const {
            return node->ptr_at(index);
        }

        const_iterator& operator++() {
            if (node) {
                if (++index >= node->count) {
                    node = node->next;
                    index = 0;
                }
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++*this;
            return tmp;
        }
        const_iterator& operator--() {
            if (node) {
                if (index == 0) {
                    node = node->prev;
                    if (node)
                        index = node->count - 1;
                } else {
                    --index;
                }
            } else {
                node = tail_ref;
                if (node)
                    index = node->count - 1;
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --*this;
            return tmp;
        }
        bool operator==(const const_iterator& other) const {
            return node == other.node && index == other.index;
        }
        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
        const_iterator(const iterator& it) noexcept
                : node(it.node), index(it.index), tail_ref(it.tail_ref) { }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <typename InputIt> unrolled_list(InputIt first, InputIt last, const Allocator& alloc = Allocator())
    : first(nullptr), last(nullptr), total_size(0), node_allocator(alloc){
    try {
        for (; first != last; ++first)
            push_back(*first);
    } catch (...) {
        clear();
        throw;
    }
}

     unrolled_list(const Allocator& alloc = Allocator()) noexcept
            : first(nullptr), last(nullptr), total_size(0), node_allocator(alloc) { }

     unrolled_list(size_type count, const T& value = T(), const Allocator& alloc = Allocator())
            : first(nullptr), last(nullptr), total_size(0), node_allocator(alloc) {
        for (size_type i = 0; i < count; ++i)
            push_back(value);
    }


    unrolled_list(std::initializer_list<T> init, const Allocator& alloc = Allocator())
            : unrolled_list(init.begin(), init.end(), alloc) { }

    unrolled_list(const unrolled_list& other)
            : first(nullptr), last(nullptr), total_size(0),
              node_allocator(std::allocator_traits<NodeAllocator>::select_on_container_copy_construction(other.node_allocator)) {
        for (const auto& elem : other)
            push_back(elem);
    }

    unrolled_list(unrolled_list&& other) noexcept
            : first(other.first), last(other.last), total_size(other.total_size),
              node_allocator(other.node_allocator) {
        other.first = nullptr;
        other.last = nullptr;
        other.total_size = 0;
    }

    
    unrolled_list(unrolled_list&& other, const Allocator& alloc)
            : first(nullptr), last(nullptr), total_size(0), node_allocator(alloc) {
        if (alloc == other.get_allocator()) {
            first = other.first;
            last = other.last;
            total_size = other.total_size;
            other.first = nullptr;
            other.last = nullptr;
            other.total_size = 0;
        } else {
            for (const auto& elem : other)
                push_back(elem);
        }
    }

    
    ~unrolled_list() { clear(); }

    unrolled_list& operator=(const unrolled_list& other) {
        if (this != &other) {
            clear();
            for (const auto& elem : other)
                push_back(elem);
        }
        return *this;
    }

    unrolled_list& operator=(unrolled_list&& other) noexcept {
        if (this != &other) {
            clear();
            first = other.first;
            last = other.last;
            total_size = other.total_size;
            node_allocator = other.node_allocator;
            other.first = nullptr;
            other.last = nullptr;
            other.total_size = 0;
        }
        return *this;
    }

    unrolled_list& operator=(std::initializer_list<T> ilist) {
        clear();
        for (const auto& item : ilist)
            push_back(item);
        return *this;
    }

    
    reference front() {
        if (empty()) throw std::out_of_range("front() on empty container");
        return *begin();
    }
    const_reference front() const {
        if (empty()) throw std::out_of_range("front() on empty container");
        return *begin();
    }
    reference back() {
        if (empty()) throw std::out_of_range("back() on empty container");
        iterator it = end();
        --it;
        return *it;
    }
    const_reference back() const {
        if (empty()) throw std::out_of_range("back() on empty container");
        const_iterator it = end();
        --it;
        return *it;
    }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
    iterator begin() noexcept { return iterator(first, 0, last); }
    const_iterator begin() const noexcept { return const_iterator(first, 0, last); }
    const_iterator cbegin() const noexcept { return begin(); }
    iterator end() noexcept { return iterator(nullptr, 0, last); }
    const_iterator end() const noexcept { return const_iterator(nullptr, 0, last); }
    const_iterator cend() const noexcept { return end(); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    bool empty() const noexcept { return total_size == 0; }
    size_type size() const noexcept { return total_size; }
    size_type max_size() const noexcept { return NodeAllocTraits::max_size(node_allocator); }
    allocator_type get_allocator() const noexcept { return allocator_type(node_allocator); }

    void swap(unrolled_list& other) noexcept {
        std::swap(first, other.first);
        std::swap(last, other.last);
        std::swap(total_size, other.total_size);
        std::swap(node_allocator, other.node_allocator);
    }

    void push_back(const T& value) {
    if (!last || last->count >= NodeMaxSize) {
        Node* new_node = create_node();
        try {
            new (new_node->ptr_at(0)) T(value);
        } catch (...) {
            destroy_node(new_node);
            throw;
        }
        new_node->count = 1;
        new_node->prev = last;
        if (last)
            last->next = new_node;
        else
            first = new_node;
        last = new_node;
    } else {

        try {
            new (last->ptr_at(last->count)) T(value);
        } catch (...) {
        
            throw;
        }
        ++last->count;
    }
    ++total_size;
}

void push_front(const T& value) {
    if (!first || first->count >= NodeMaxSize) {
        Node* new_node = create_node();
        T temp(value);
        new (new_node->ptr_at(0)) T((temp));
        new_node->count = 1;
        new_node->next = first;
        if (first)
            first->prev = new_node;
        else
            last = new_node;
        first = new_node;
    } else {
        const size_type n = first->count;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[NodeMaxSize];
        T* tmp = reinterpret_cast<T*>(buffer);
        size_type i = 0;
        try {
            new (tmp) T(value);
            for (i = 0; i < n; ++i) {
                new (tmp + i + 1) T((*first->ptr_at(i)));
            }
        } catch (...) {
            for (size_type j = 0; j < i + 1; ++j) {
                tmp[j].~T();
            }
            throw;
        }
        for (size_type i = 0; i < n; ++i) {
            first->ptr_at(i)->~T();
        }
        for (size_type i = 0; i < n + 1; ++i) {
            new (first->ptr_at(i)) T((tmp[i]));
            tmp[i].~T();
        }
        first->count = n + 1;
    }
    ++total_size;
}

    void pop_back() noexcept {
        
        --last->count;
        last->ptr_at(last->count)->~T();
        --total_size;
        if (last->count == 0) {
            Node* prev = last->prev;
            if (prev)
                prev->next = nullptr;
            else
                first = nullptr;
            destroy_node(last);
            last = prev;
        }
    }

    void pop_front() noexcept {
        
        first->ptr_at(0)->~T();
        for (size_type i = 1; i < first->count; ++i) {
            new (first->ptr_at(i - 1)) T(*first->ptr_at(i));
            first->ptr_at(i)->~T();
        }
        --first->count;
        --total_size;
        if (first->count == 0) {
            Node* next = first->next;
            if (next)
                next->prev = nullptr;
            else
                last = nullptr;
            destroy_node(first);
            first = next;
        }
    }

    iterator insert(const_iterator pos, const T& value) {
        iterator it = make_iterator(pos);
        Node* node = it.node;
        size_type index = it.index;
        if (!node) { 
            push_back(value);
            return iterator(last, last->count - 1, last);
        }
        if (node->count >= NodeMaxSize) {
            split_node(node);
            if (index > node->count) {
                index -= node->count;
                return insert(iterator(node->next, index, last), value);
            } else {
                return insert(iterator(node, index, last), value);
            }
        }
        
        typename std::aligned_storage<sizeof(T), alignof(T)>::type tmp_buffer[NodeMaxSize];
        T* tmp = reinterpret_cast<T*>(tmp_buffer);
        size_type new_count = node->count + 1;
        size_type i = 0;
        try {
        
            for (i = 0; i < index; i++) {
                new (tmp + i) T(*node->ptr_at(i));
            }
            new (tmp + index) T(value);
            for (i = index; i < node->count; i++) {
                new (tmp + i + 1) T(*node->ptr_at(i));
            }
        } catch (...) {
            for (size_type j = 0; j < (i < index ? i : i + 1); j++) {
                tmp[j].~T();
            }
            throw;
        }
    
        for (size_type j = 0; j < node->count; j++) {
            node->ptr_at(j)->~T();
        }
        for (i = 0; i < new_count; i++) {
            new (node->ptr_at(i)) T((tmp[i]));
            tmp[i].~T();
        }
        node->count = new_count;
        ++total_size;
        return iterator(node, index, last);
    }    
    
    iterator insert(const_iterator pos, size_type count, const T& value) {
        iterator it = make_iterator(pos);
        for (size_type i = 0; i < count; ++i)
            it = insert(it, value);
        return it;
    }

    iterator erase(const_iterator p) noexcept {
        iterator it = make_iterator(p);
        Node* node = it.node;
        size_type index = it.index;
        
        for (size_type i = index; i < node->count - 1; ++i) {
            node->ptr_at(i)->~T();
            new (node->ptr_at(i)) T((*node->ptr_at(i + 1)));
        }
        node->ptr_at(node->count - 1)->~T();
        --node->count;
        --total_size;

        if (node->count == 0) {
            Node* next_node = node->next;
            if (node->prev)
                node->prev->next = next_node;
            else
                first = next_node;
            if (next_node)
                next_node->prev = node->prev;
            else
                last = node->prev;
            destroy_node(node);
            return iterator(next_node, 0, last);
        }
        return iterator(node, index, last);
    }
    
    iterator erase(const_iterator first, const_iterator last) {
        iterator it = make_iterator(first);
        while (it != make_iterator(last))
            it = erase(it);
        return it;
    }

    void clear() noexcept {
        Node* current = first;
        while (current) {
            Node* next = current->next;
            destroy_node(current);
            current = next;
        }
        first = last = nullptr;
        total_size = 0;
    }

private:
    iterator make_iterator(const_iterator pos) {
        return iterator(const_cast<Node*>(pos.node), pos.index, last);
    }
};


template <typename T, size_t N, typename A>
void swap(unrolled_list<T, N, A>& lhs, unrolled_list<T, N, A>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T, size_t N, typename A>
bool operator==(const unrolled_list<T, N, A>& lhs, const unrolled_list<T, N, A>& rhs) {
    if (lhs.size() != rhs.size())
        return false;
    auto it1 = lhs.begin();
    auto it2 = rhs.begin();
    for (; it1 != lhs.end(); ++it1, ++it2) {
        if (!(*it1 == *it2))
            return false;
    }
    return true;
}

template <typename T, size_t N, typename A>
bool operator!=(const unrolled_list<T, N, A>& lhs, const unrolled_list<T, N, A>& rhs) {
    return !(lhs == rhs);
}
