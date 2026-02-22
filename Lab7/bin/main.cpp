#include <iostream>
#include <vector>
#include <unrolled_list.h>

template <typename Iterator>
class sorted_list {
public:

    using value_type = typename std::iterator_traits<Iterator>::value_type;
    sorted_list(Iterator first, Iterator last)
        : first_(first), last_(last) { }
    
   
    class const_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = typename sorted_list::value_type;
        using difference_type   = ptrdiff_t;
        using pointer           = const value_type*;
        using reference         = const value_type&;
        
        const_iterator(const sorted_list* parent)
            : parent_(parent), finished_(false)
        {
            if (parent_->first_ == parent_->last_) {
                finished_ = true;
                return;
            }
            bool firstFound = false;
            for (Iterator it = parent_->first_; it != parent_->last_; ++it) {
                if (!firstFound) {
                    current_ = *it;
                    firstFound = true;
                } else if (*it < current_) {
                    current_ = *it;
                }
            }
        
            freq_remaining_ = 0;
            for (Iterator it = parent_->first_; it != parent_->last_; ++it) {
                if (!(current_ < *it) && !(*it < current_))
                    ++freq_remaining_;
            }
        }
        
        const_iterator() : parent_(nullptr), finished_(true), freq_remaining_(0) { }
        
        reference operator*() const { return current_; }
        pointer operator->() const { return &current_; }
        
        const_iterator& operator++() {
            if (finished_ || parent_ == nullptr)
                return *this;
            if (freq_remaining_ > 1) {
                --freq_remaining_;
                return *this;
            }
            bool found = false;
            value_type next;
            for (Iterator it = parent_->first_; it != parent_->last_; ++it) {
                if (current_ < *it) {
                    if (!found) {
                        next = *it;
                        found = true;
                    } else if (*it < next) {
                        next = *it;
                    }
                }
            }
            if (!found) {
                finished_ = true;
                return *this;
            }
           
            current_ = next;
            freq_remaining_ = 0;
            for (Iterator it = parent_->first_; it != parent_->last_; ++it) {
                if (!(next < *it) && !(*it < next))
                    ++freq_remaining_;
            }
            --freq_remaining_;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const const_iterator& other) const {
            if (finished_ && other.finished_)
                return true;
            return finished_ == other.finished_ &&
                   current_ == other.current_ &&
                   freq_remaining_ == other.freq_remaining_ &&
                   parent_ == other.parent_;
        }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
        
    private:
        const sorted_list* parent_ = nullptr; 
        bool finished_ = false;
        value_type current_{};      
        size_t freq_remaining_ = 0; 
    };
    
    
    const_iterator begin() const { return const_iterator(this); }
    const_iterator end() const { return const_iterator(); }
    
private:
    Iterator first_;
    Iterator last_;
};




int main(int argc, char** argv) {
    std::cout << "Hello, world!" << std::endl;
    unrolled_list<int> list = {3,2,4,1,5};
    sorted_list a(list.begin(), list.end());
    for (auto i : a){
        std::cout << i << std::endl; // 1 2 3 4 5
    }
    for (auto i = a.begin(); i != a.end();i++){
        std::cout << *i << std::endl; // 3 2 4 1 5
    }
    for (auto i : list){
        std::cout << i << std::endl; // 3 2 4 1 5
    }
    return 0;
}//cmake .. -G "MinGW Makefiles"

