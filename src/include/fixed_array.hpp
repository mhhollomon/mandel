#if not defined(MANDEL_FIXED_ARRAY_HPP_)
#define MANDEL_FIXED_ARRAY_HPP_

#include <memory>

template<class T>
class fixed_array {
    std::unique_ptr<T[]> data_;
    int count_;

  public:
    fixed_array(int count) : count_(count) {
        data_ = std::unique_ptr<T[]>(new T[count_]);
    }

    fixed_array(fixed_array<T> const& o) {
        data_.reset(new T[o.count_]);
        count_ = o.count_;
        for (int i = 0; i < count_; ++i) {
            data_[i] = o.data_[i];
        }
    }

    T &operator[](int i) {
        if (i > count_ or i < 0)
            throw std::runtime_error("fixed array index out of bounds");

        return data_[i];
    }

    T const *begin() const {
        return data_.get();
    }
    T *begin() {
        return data_.get();
    }

    T const * end() const {
        return data_.get() + count_;
    }
    T* end() {
        return data_.get() + count_;
    }

    int size() const { return count_; }
};

#endif
