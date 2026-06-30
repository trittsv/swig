/* -----------------------------------------------------------------------------
 * std_unordered_set.i
 *
 * std::unordered_set<T, H> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <unordered_set>
%}

namespace std {

template<class T, class H = std::hash<T> > class unordered_set {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T key_type;
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;

  unordered_set();
  unordered_set(const unordered_set &other);
  void clear();
  size_type size() const;
  bool empty() const;
  size_type count(const T &x) const;
  size_type erase(const T &x);

  %extend {
    void insert_value(const T &x) {
      $self->insert(x);
    }
  }
};

}
