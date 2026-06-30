/* -----------------------------------------------------------------------------
 * std_list.i
 *
 * std::list<T> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <list>
%}

namespace std {

template<class T> class list {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;

  list();
  list(const list &other);

  void clear();
  void push_back(const T &x);
  void push_front(const T &x);
  void pop_back();
  void pop_front();
  void remove(T x);
  void reverse();
  size_type size() const;
  bool empty() const;
};

}
