#include <vector>

template<typename T>
class Deque {
 public:
  // Constructors, operator= & destructor
  Deque();
  Deque(const Deque&);
  explicit Deque(size_t);
  Deque(size_t, const T&);
  Deque& operator=(const Deque&);
  ~Deque();

  // Access to elements
  size_t size() const;
  T& operator[](size_t);
  const T& operator[](size_t) const;
  T& at(size_t);
  const T& at(size_t) const;

  // Changing deque
  void push_back(const T&);
  void pop_back();
  void push_front(const T&);
  void pop_front();

 private:
  // Class members
  static const size_t kBlockSize_ = 16;
  std::vector<T*> data_;
  size_t data_sz_ = 2;
  size_t sz_ = 0;
  size_t ia_front = kBlockSize_;

  // Additional functions
  size_t ia_from_id(size_t) const; // ia - array index
  size_t ib_from_id(size_t) const; // ib - block index
  size_t ic_from_id(size_t) const; // ic - cell index
  void reallocate(size_t);

  // Iterators
  template<bool is_const>
  class base_iterator {
   public:
    using value_type = T;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::random_access_iterator_tag;

    // Constructors
    base_iterator() = default;
    base_iterator(std::conditional_t<is_const, const Deque<T>&, Deque<T>&> deque,
                  size_t id) : it_(deque.data_.begin() + deque.ib_from_id(id)), cell_(deque.ic_from_id(id)) {}

    // Comparison operators
    bool operator==(const base_iterator& other) const { return (it_ == other.it_) && (cell_ == other.cell_); }
    bool operator<(const base_iterator& other) const { return it_ < other.it_ || (it_ == other.it_ && cell_ < other.cell_); }
    bool operator!=(const base_iterator& other) const { return !operator==(other); }
    bool operator<=(const base_iterator& other) const { return !operator>(other); }
    bool operator>=(const base_iterator& other) const { return !operator<(other); }
    bool operator>(const base_iterator& other) const { return other.operator<(*this); }

    // operators * and ->
    reference operator*() const {
      reference ref = (*it_)[cell_];
      return ref;
    }

    pointer operator->() const {
      pointer ptr = &((*it_)[cell_]);
      return ptr;
    }

    // Arithmetic operators
    int operator-(const base_iterator& other) const { return (it_ - other.it_) * kBlockSize_ + cell_ - other.cell_; }

    base_iterator& operator+=(int x) {
      it_ += (x + cell_) / kBlockSize_;
      cell_ = (x + cell_) % kBlockSize_;
      return *this;
    }

    base_iterator& operator-=(int x) {
      int diff = (x - cell_ + kBlockSize_ - 1) / kBlockSize_;
      int shift = kBlockSize_ - 1 - (x - cell_ + kBlockSize_ - 1) % kBlockSize_;
      it_ -= diff;
      cell_ = shift;
      return *this;
    }

    base_iterator& operator++() {
      if (cell_ == kBlockSize_ - 1) {
        ++it_;
        cell_ = 0;
      } else {
        ++cell_;
      }
      return *this;
    }

    base_iterator& operator--() {
      if (cell_ == 0) {
        --it_;
        cell_ = kBlockSize_ - 1;
      } else {
        --cell_;
      }
      return *this;
    }

    base_iterator operator++(int) {
      auto copy = *this;
      ++*this;
      return copy;
    }

    base_iterator operator--(int) {
      auto copy = *this;
      --*this;
      return copy;
    }

    base_iterator operator+(int num) const {
      base_iterator copy = *this;
      copy += num;
      return copy;
    }

    base_iterator operator-(int num) const {
      base_iterator copy = *this;
      copy -= num;
      return copy;
    }

    // Conversion operator
    operator base_iterator<true>() {
      base_iterator<true> it = *this;
      return it;
    }

   private:
    // Class members
    std::conditional_t<is_const, typename std::vector<T*>::const_iterator, typename std::vector<T*>::iterator> it_; // iterator of the vector, where element stored
    size_t cell_ = 0; // index of the cell, where element stored

  };
 public:
  // Definition of iterator and const_iterator
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;

  // Access to iterators
  Deque::iterator begin();
  Deque::const_iterator begin() const;
  Deque::const_iterator cbegin() const;
  std::reverse_iterator<Deque::iterator> rbegin();
  std::reverse_iterator<Deque::const_iterator> rbegin() const;
  std::reverse_iterator<Deque::const_iterator> crbegin() const;
  Deque::iterator end();
  Deque::const_iterator end() const;
  Deque::const_iterator cend() const;
  std::reverse_iterator<Deque::iterator> rend();
  std::reverse_iterator<Deque::const_iterator> rend() const;
  std::reverse_iterator<Deque::const_iterator> crend() const;

  // Changing deque
  void insert(iterator, const T&);
  void erase(iterator);
};

// Additional functions

template<typename T>
size_t Deque<T>::ia_from_id(size_t id) const { return id + ia_front; }

template<typename T>
size_t Deque<T>::ib_from_id(size_t id) const {
  size_t ia = ia_from_id(id);
  return ia / kBlockSize_;
}

template<typename T>
size_t Deque<T>::ic_from_id(size_t id) const {
  size_t ia = ia_from_id(id);
  return ia % kBlockSize_;
}

template<typename T>
void Deque<T>::reallocate(size_t num_of_blocks) {
  std::vector<T*> new_data(num_of_blocks);
  size_t front_gap = (num_of_blocks - data_sz_) / 2;
  for (size_t i = 0; i < front_gap; ++i) {
    new_data[i] = nullptr;
  }
  for (size_t i = front_gap; i < front_gap + data_sz_; ++i) {
    new_data[i] = data_[i - front_gap];
  }
  for (size_t i = data_sz_ + front_gap; i < num_of_blocks; ++i) {
    new_data[i] = nullptr;
  }
  data_ = new_data;
  data_sz_ = num_of_blocks;
  ia_front += front_gap * kBlockSize_;
}

// Constructors

template<typename T>
Deque<T>::Deque() {
  data_.resize(data_sz_);
  for (size_t i = 0; i < data_sz_; ++i) {
    data_[i] = nullptr;
  }
  data_[1] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
}

template<typename T>
Deque<T>::Deque(const Deque<T>& other) : data_sz_(other.data_sz_), sz_(other.sz_), ia_front(other.ia_front) {
  data_.resize(data_sz_);
  size_t i = 0;
  size_t j = 0;
  try {
    for (; i < data_sz_; ++i) {
      if (other.data_[i] == nullptr) {
        data_[i] = nullptr;
      } else {
        data_[i] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
        j = 0;
        for (; j < kBlockSize_; ++j) {
          if (i * kBlockSize_ + j <= ia_from_id(sz_ - 1) && i * kBlockSize_ + j >= ia_front) {
            new (data_[i] + j) T(other.data_[i][j]);
          }
        }
      }
    }
  } catch (...) {
    for (int l = 0; l < j; ++l) {
      (data_[i] + l)->~T();
    }
    delete[] reinterpret_cast<char*>(data_[i]);
    for (size_t k = 0; k < i - 1; ++k) {
      if (data_[k] != nullptr) {
        for (int l = 0; l < kBlockSize_; ++l) {
          (data_[k] + l)->~T();
        }
        delete[] reinterpret_cast<char*>(data_[k]);
      }
    }
    throw;
  }
}

template<typename T>
Deque<T>::Deque(size_t size, const T& el) : Deque() {
  size_t i = 0;
  try {
    for (; i < size; ++i) {
      push_back(el);
    }
  } catch (...) {
    size_t size = sz_;
    for (ssize_t i = 0; i < size; ++i) {
      pop_back();
    }
    delete[] reinterpret_cast<char*>(data_[0]);
    throw;
  }
}

template<typename T>
Deque<T>::Deque(size_t size) : Deque(size, T()) {}

template<typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& other) {
  Deque<T> copy(other);
  std::swap(data_, copy.data_);
  std::swap(sz_, copy.sz_);
  std::swap(data_sz_, copy.data_sz_);
  std::swap(ia_front, copy.ia_front);
  return *this;
}

template<typename T>
Deque<T>::~Deque() {
  size_t size = sz_;
  for (size_t i = 0; i < size; ++i) {
    pop_back();
  }
}

// Access to elements;

template<typename T>
size_t Deque<T>::size() const { return sz_; }

template<typename T>
T& Deque<T>::operator[](size_t id) { return data_[ib_from_id(id)][ic_from_id(id)]; }

template<typename T>
const T& Deque<T>::operator[](size_t id) const { return data_[ib_from_id(id)][ic_from_id(id)]; }

template<typename T>
T& Deque<T>::at(size_t id) {
  if (id >= sz_) {
    throw std::out_of_range("index >= this->size()");
  } else {
    return operator[](id);
  }
}

template<typename T>
const T& Deque<T>::at(size_t id) const {
  if (id >= sz_) {
    throw std::out_of_range("index >= this->size()");
  } else {
    return operator[](id);
  }
}

// Changing deque

template<typename T>
void Deque<T>::push_back(const T& el) {
  if (ic_from_id(sz_) == kBlockSize_ - 1) {
    if (ib_from_id(sz_) == data_sz_ - 1) {
      reallocate(data_sz_ == 0 ? 1 : data_sz_ * 2);
    }
    data_[ib_from_id(sz_) + 1] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
  }
  new (data_[ib_from_id(sz_)] + ic_from_id(sz_)) T(el);
  ++sz_;
}

template<typename T>
void Deque<T>::pop_back() {
  size_t ib = ib_from_id(sz_ - 1);
  size_t ic = ic_from_id(sz_ - 1);
  (data_[ib] + ic)->~T();
  if (ic == 0) {
    delete[] reinterpret_cast<char*>(data_[ib]);
  }
  --sz_;
  if (sz_ == 0) {
    ia_front = 0;
  }
}

template<typename T>
void Deque<T>::push_front(const T& el) {
  if (ic_from_id(0) == 0) {
    if (ib_from_id(0) == 0 && sz_ > 0) {
      reallocate(data_sz_ * 2);
    }
    data_[(ib_from_id(0) - 1 + data_sz_) % data_sz_] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    new (data_[(ib_from_id(0) - 1 + data_sz_) % data_sz_] + kBlockSize_ - 1) T(el);
  } else {
    new (data_[ib_from_id(0)] + ic_from_id(0) - 1) T(el);
  }
  ia_front -= 1;
  ++sz_;
}

template<typename T>
void Deque<T>::pop_front() {
  size_t ib = ib_from_id(0);
  size_t ic = ic_from_id(0);
  (data_[ib] + ic)->~T();
  if (ic == kBlockSize_ - 1) {
    delete[] reinterpret_cast<char*>(data_[ib]);
  }
  ia_front += 1;
  --sz_;
  if (sz_ == 0) {
    ia_front = 0;
  }
}

// Iterators

template<typename T>
Deque<T>::iterator Deque<T>::begin() {
  return iterator(*this, 0);
}

template<typename T>
Deque<T>::const_iterator Deque<T>::begin() const {
  return const_iterator(*this, 0);
}

template<typename T>
Deque<T>::const_iterator Deque<T>::cbegin() const {
  return const_iterator(*this, 0);
}

template<typename T>
Deque<T>::iterator Deque<T>::end() {
  return iterator(*this, sz_);
}

template<typename T>
Deque<T>::const_iterator Deque<T>::end() const {
  return const_iterator(*this, sz_);
}

template<typename T>
Deque<T>::const_iterator Deque<T>::cend() const {
  return const_iterator(*this, sz_);
}

template<typename T>
std::reverse_iterator<typename Deque<T>::iterator> Deque<T>::rbegin() {
  return std::reverse_iterator(end());
}

template<typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::rbegin() const {
  return std::reverse_iterator(end());
}

template<typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::crbegin() const {
  return std::reverse_iterator(cbegin());
}

template<typename T>
std::reverse_iterator<typename Deque<T>::iterator> Deque<T>::rend() {
  return std::reverse_iterator(begin());
}

template<typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::rend() const {
  return std::reverse_iterator(begin());
}

template<typename T>
std::reverse_iterator<typename Deque<T>::const_iterator> Deque<T>::crend() const {
  return std::reverse_iterator(cbegin());
}

// erase & insert

template<typename T>
void Deque<T>::erase(Deque::iterator it) {
  for (; it < end() - 1; ++it) {
    *it = *(it + 1);
  }
  it->~T();
  --sz_;
}

template<typename T>
void Deque<T>::insert(Deque::iterator it, const T& el) {
  for (auto i_end = end(); it < i_end; --i_end) {
    *i_end = *(i_end - 1);
  }
  *it = el;
  push_back(operator[](sz_));
}
