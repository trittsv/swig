%module rust_std_except

%include <std_except.i>

%inline %{
#include <stdexcept>

int rust_no_throw() throw (std::out_of_range) {
  return 7;
}

int rust_throw_out_of_range() throw (std::out_of_range) {
  throw std::out_of_range("rust out of range");
}
%}

