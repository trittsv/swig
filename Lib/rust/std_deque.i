/* -----------------------------------------------------------------------------
 * std_deque.i
 *
 * std::deque<T> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <deque>
#include <stdexcept>
%}

%include <std_except.i>

namespace std {

template<class T> class deque {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;

  deque();
  deque(const deque &other);

  void clear();
  void push_back(const T &x);
  void push_front(const T &x);
  void pop_back();
  void pop_front();
  size_type size() const;
  bool empty() const;

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
