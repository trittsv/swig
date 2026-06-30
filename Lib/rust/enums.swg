/* -----------------------------------------------------------------------------
 * enums.swg
 *
 * Enum typemaps for Rust.
 * ----------------------------------------------------------------------------- */

%typemap(ctype) enum SWIGTYPE, const enum SWIGTYPE & "int"
%typemap(imtype) enum SWIGTYPE, const enum SWIGTYPE & "c_int"
%typemap(rusttype) enum SWIGTYPE, const enum SWIGTYPE & "c_int"

%typemap(in) enum SWIGTYPE
%{ $1 = ($1_ltype)$input; %}

%typemap(in) const enum SWIGTYPE & ($*1_ltype temp)
%{ temp = ($*1_ltype)$input;
   $1 = &temp; %}

%typemap(out) enum SWIGTYPE
%{ $result = (int)$1; %}

%typemap(out) const enum SWIGTYPE &
%{ $result = (int)*$1; %}

%typemap(rustin) enum SWIGTYPE, const enum SWIGTYPE & "$rustinput"
%typemap(rustout) enum SWIGTYPE, const enum SWIGTYPE & {
  let ret = $imcall;
  rust_check_exception();
  ret
}
