/* -----------------------------------------------------------------------------
 * boost_shared_ptr.i
 *
 * Shared pointer typemaps for Rust. Define SWIG_SHARED_PTR_NAMESPACE to choose
 * std or boost, matching the C# library layout.
 * ----------------------------------------------------------------------------- */

#ifndef SWIG_SHARED_PTR_NAMESPACE
#define SWIG_SHARED_PTR_NAMESPACE boost
#endif

#ifndef SWIG_SHARED_PTR_TYPEMAPS
#define SWIG_SHARED_PTR_TYPEMAPS(CONST, TYPE...) SWIG_RUST_SHARED_PTR_TYPEMAPS(CONST, TYPE)
#endif

%define SWIG_RUST_SHARED_PTR_TYPEMAPS(CONST, TYPE...)
%naturalvar TYPE;
%naturalvar SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >;

%feature("unref") TYPE "(void)arg1; delete smartarg1;"

%typemap(in) CONST TYPE ($&1_type argp = 0) %{
  argp = ((SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *)$input) ? ((SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *)$input)->get() : 0;
  $1 = *argp;
%}
%typemap(out) CONST TYPE %{ $result = new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >(new $1_ltype($1)); %}

%typemap(in) CONST TYPE * (SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *smartarg = 0) %{
  smartarg = (SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *)$input;
  $1 = (TYPE *)(smartarg ? smartarg->get() : 0);
%}
%typemap(out, fragment="SWIG_null_deleter") CONST TYPE * %{
  $result = $1 ? new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner) : 0;
%}

%typemap(in) CONST TYPE & %{
  $1 = ($1_ltype)(((SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *)$input) ? ((SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *)$input)->get() : 0);
%}
%typemap(out, fragment="SWIG_null_deleter") CONST TYPE & %{
  $result = new SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner);
%}

%typemap(in) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > %{ if ($input) $1 = *($&1_ltype)$input; %}
%typemap(out) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > %{ $result = new $1_ltype($1); %}

%typemap(ctype) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "void *"
%typemap(imtype, out="*mut c_void") SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                                    SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                                    SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                                    SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "*mut c_void"
%typemap(rusttype) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "TYPE"
%typemap(rustin) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                 SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                 SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                 SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& "$rustinput.as_ptr()"
%typemap(rustout) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE >,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > &,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *,
                  SWIG_SHARED_PTR_QNAMESPACE::shared_ptr< CONST TYPE > *& {
  let ptr = $imcall;
  rust_check_exception();
  TYPE::from_raw_owned_unchecked(ptr, true)
}
%enddef

%include <shared_ptr.i>
