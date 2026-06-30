/* -----------------------------------------------------------------------------
 * std_array.i
 *
 * std::array<T, N> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <array>
#include <stdexcept>
%}

%include <std_except.i>

namespace std {

template<class T, size_t N> class array {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;

  array();
  array(const array &other);

  size_type size() const;
  bool empty() const;
  void fill(const T &value);

  %extend {
    T getitemcopy(int index) SWIG_RUST_THROW_OUT_OF_RANGE {
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
