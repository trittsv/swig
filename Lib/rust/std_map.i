/* -----------------------------------------------------------------------------
 * std_map.i
 *
 * std::map<K, T> wrapper support for Rust.
 * ----------------------------------------------------------------------------- */

%{
#include <map>
#include <stdexcept>
%}

%include <std_except.i>
%include <std_pair.i>

namespace std {

template<class K, class T> class map {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef K key_type;
  typedef T mapped_type;
  typedef std::pair<const K, T> value_type;

  map();
  map(const map &other);

  void clear();
  size_type size() const;
  bool empty() const;
  size_type count(const K &key) const;
  size_type erase(const K &key);

  %extend {
    T getitem(const K &key) SWIG_RUST_THROW_OUT_OF_RANGE {
      typename std::map<K, T>::const_iterator it = $self->find(key);
      if (it != $self->end())
        return it->second;
      throw std::out_of_range("key");
    }

    void setitem(const K &key, const T &value) {
      (*$self)[key] = value;
    }
  }
};

}
