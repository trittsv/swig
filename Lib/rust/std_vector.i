/* -----------------------------------------------------------------------------
 * std_vector.i
 *
 * Minimal std::vector<T> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <stdexcept>
#include <vector>
%}

#if __cplusplus >= 201703L
#define SWIG_RUST_THROW_OUT_OF_RANGE noexcept(false)
#else
#define SWIG_RUST_THROW_OUT_OF_RANGE throw (std::out_of_range)
#endif

%include <std_except.i>

namespace std {

template<class T> class vector {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;

  vector();
  vector(const vector &other);

  void clear();
  void push_back(const T &x);
  size_type size() const;
  bool empty() const;
  size_type capacity() const;
  void reserve(size_type n);

  %extend {
    T getitemcopy(int index) SWIG_RUST_THROW_OUT_OF_RANGE {
      if (index >= 0 && index < (int)$self->size())
        return (*$self)[index];
      throw std::out_of_range("index");
    }

    const T &getitem(int index) SWIG_RUST_THROW_OUT_OF_RANGE {
      if (index >= 0 && index < (int)$self->size())
        return (*$self)[index];
      throw std::out_of_range("index");
    }

    void setitem(int index, const T &value) SWIG_RUST_THROW_OUT_OF_RANGE {
      if (index >= 0 && index < (int)$self->size()) {
        (*$self)[index] = value;
        return;
      }
      throw std::out_of_range("index");
    }
  }
};

}
