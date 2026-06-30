/* -----------------------------------------------------------------------------
 * This file is part of SWIG, which is licensed as a whole under version 3
 * (or any later version) of the GNU General Public License. Some additional
 * terms also apply to certain portions of SWIG. The full details of the SWIG
 * license and copyrights can be found in the LICENSE and COPYRIGHT files
 * included with the SWIG source code as distributed by the SWIG developers
 * and at https://www.swig.org/legal.html.
 *
 * rust.cxx
 *
 * Rust language module for SWIG.
 * ----------------------------------------------------------------------------- */

#include "swigmod.h"

typedef DOH UpcallData;

static const char *usage = "\
Rust Options (available with -rust)\n\
     -crate <name>  - Set the Rust module/crate name used in generated comments\n\
\n";

class RUST : public Language {
  File *f_begin;
  File *f_runtime;
  File *f_runtime_h;
  File *f_header;
  File *f_wrappers;
  File *f_init;
  File *f_directors;
  File *f_directors_h;
  File *f_rust;
  String *module_name;
  String *crate_name;
  String *rust_extern_code;
  String *rust_proxy_code;
  String *proxy_class_name;
  String *proxy_class_code;
  String *destructor_call;
  String *variable_name;
  String *director_callback_typedefs;
  String *director_callbacks;
  String *director_connect_parms;
  String *director_trait_code;
  String *director_callback_shims;
  String *director_method_metadata;
  List *dmethods_seq;
  Hash *dmethods_table;
  int n_dmethods;
  int first_class_dmethod;
  int curr_class_dmethod;
  bool global_variable_flag;
  bool member_variable_flag;
  bool static_member_variable_flag;

public:
  RUST() :
    f_begin(NULL),
    f_runtime(NULL),
    f_runtime_h(NULL),
    f_header(NULL),
    f_wrappers(NULL),
    f_init(NULL),
    f_directors(NULL),
    f_directors_h(NULL),
    f_rust(NULL),
    module_name(NULL),
    crate_name(NULL),
    rust_extern_code(NULL),
    rust_proxy_code(NULL),
    proxy_class_name(NULL),
    proxy_class_code(NULL),
    destructor_call(NULL),
    variable_name(NULL),
    director_callback_typedefs(NULL),
    director_callbacks(NULL),
    director_connect_parms(NULL),
    director_trait_code(NULL),
    director_callback_shims(NULL),
    director_method_metadata(NULL),
    dmethods_seq(NULL),
    dmethods_table(NULL),
    n_dmethods(0),
    first_class_dmethod(0),
    curr_class_dmethod(0),
    global_variable_flag(false),
    member_variable_flag(false),
    static_member_variable_flag(false) {
    director_multiple_inheritance = 0;
    directorLanguage();
    allow_overloading();
    Swig_interface_feature_enable();
  }

  virtual void main(int argc, char *argv[]) {
    SWIG_library_directory("rust");

    for (int i = 1; i < argc; i++) {
      if (argv[i]) {
        if (strcmp(argv[i], "-help") == 0) {
          fputs(usage, stdout);
        } else if (strcmp(argv[i], "-crate") == 0) {
          if (argv[i + 1]) {
            crate_name = NewString(argv[i + 1]);
            Swig_mark_arg(i);
            Swig_mark_arg(i + 1);
            i++;
          } else {
            Swig_arg_error();
          }
        }
      }
    }

    Preprocessor_define("SWIGRUST 1", 0);
    SWIG_config_file("rust.swg");
  }

  virtual int top(Node *n) {
    module_name = Getattr(n, "name");
    if (!crate_name)
      crate_name = Copy(module_name);

    Node *module = Getattr(n, "module");
    Node *optionsnode = Getattr(module, "options");
    if (optionsnode) {
      if (Getattr(optionsnode, "directors"))
        allow_directors();
      if (Getattr(optionsnode, "dirprot"))
        allow_dirprot();
      allow_allprotected(GetFlag(optionsnode, "allprotected"));
    }

    String *outfile = Getattr(n, "outfile");
    String *outfile_h = Getattr(n, "outfile_h");
    f_begin = NewFile(outfile, "w", SWIG_output_files());
    if (!f_begin) {
      FileErrorDisplay(outfile);
      Exit(EXIT_FAILURE);
    }

    if (Swig_directors_enabled()) {
      if (!outfile_h) {
        Printf(stderr, "Unable to determine outfile_h\n");
        Exit(EXIT_FAILURE);
      }
      f_runtime_h = NewFile(outfile_h, "w", SWIG_output_files());
      if (!f_runtime_h) {
        FileErrorDisplay(outfile_h);
        Exit(EXIT_FAILURE);
      }
    }

    String *rustfile = NewStringf("%s.rs", module_name);
    f_rust = NewFile(rustfile, "w", SWIG_output_files());
    if (!f_rust) {
      FileErrorDisplay(rustfile);
      Exit(EXIT_FAILURE);
    }
    Delete(rustfile);

    f_runtime = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");
    f_init = NewString("");
    f_directors = NewString("");
    f_directors_h = NewString("");
    rust_extern_code = NewString("");
    rust_proxy_code = NewString("");
    dmethods_seq = NewList();
    dmethods_table = NewHash();
    n_dmethods = 0;

    Swig_register_filebyname("begin", f_begin);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("init", f_init);
    Swig_register_filebyname("director", f_directors);
    Swig_register_filebyname("director_h", f_directors_h);

    Swig_banner(f_begin);
    Swig_obligatory_macros(f_runtime, "RUST");
    Printf(f_runtime, "#include <stdlib.h>\n");
    Printf(f_runtime, "#include <string.h>\n\n");
    Printf(f_runtime, "#if defined(__cplusplus) && __cplusplus >= 201103L\n");
    Printf(f_runtime, "# define SWIGRUST_THREAD_LOCAL thread_local\n");
    Printf(f_runtime, "#elif defined(_MSC_VER)\n");
    Printf(f_runtime, "# define SWIGRUST_THREAD_LOCAL __declspec(thread)\n");
    Printf(f_runtime, "#elif defined(__GNUC__)\n");
    Printf(f_runtime, "# define SWIGRUST_THREAD_LOCAL __thread\n");
    Printf(f_runtime, "#else\n");
    Printf(f_runtime, "# define SWIGRUST_THREAD_LOCAL\n");
    Printf(f_runtime, "#endif\n\n");
    Printf(f_runtime, "static char *SWIG_RustStringCopy(const char *s) {\n");
    Printf(f_runtime, "  size_t len;\n");
    Printf(f_runtime, "  char *copy;\n");
    Printf(f_runtime, "  if (!s)\n");
    Printf(f_runtime, "    return 0;\n");
    Printf(f_runtime, "  len = strlen(s) + 1;\n");
    Printf(f_runtime, "  copy = (char *)malloc(len);\n");
    Printf(f_runtime, "  if (copy)\n");
    Printf(f_runtime, "    memcpy(copy, s, len);\n");
    Printf(f_runtime, "  return copy;\n");
    Printf(f_runtime, "}\n\n");
    Printf(f_runtime, "static SWIGRUST_THREAD_LOCAL char *SWIG_RustPendingExceptionType = 0;\n");
    Printf(f_runtime, "static SWIGRUST_THREAD_LOCAL char *SWIG_RustPendingException = 0;\n\n");
    Printf(f_runtime, "static void SWIG_RustSetPendingException(const char *kind, const char *message) {\n");
    Printf(f_runtime, "  if (SWIG_RustPendingExceptionType)\n");
    Printf(f_runtime, "    free(SWIG_RustPendingExceptionType);\n");
    Printf(f_runtime, "  if (SWIG_RustPendingException)\n");
    Printf(f_runtime, "    free(SWIG_RustPendingException);\n");
    Printf(f_runtime, "  SWIG_RustPendingExceptionType = SWIG_RustStringCopy(kind ? kind : \"std::exception\");\n");
    Printf(f_runtime, "  SWIG_RustPendingException = SWIG_RustStringCopy(message ? message : \"C++ exception\");\n");
    Printf(f_runtime, "}\n\n");

    if (Swig_directors_enabled()) {
      Printf(f_runtime, "#define SWIG_DIRECTORS\n");
      Swig_banner(f_directors_h);
      Printf(f_directors_h, "\n");
      Printf(f_directors_h, "#ifndef SWIG_%s_WRAP_H_\n", module_name);
      Printf(f_directors_h, "#define SWIG_%s_WRAP_H_\n\n", module_name);
      Printf(f_directors, "\n\n");
      Printf(f_directors, "/* ---------------------------------------------------\n");
      Printf(f_directors, " * C++ director class methods\n");
      Printf(f_directors, " * --------------------------------------------------- */\n\n");
      String *filename = Swig_file_filename(outfile_h);
      Printf(f_directors, "#include \"%s\"\n\n", filename);
      Delete(filename);
    }

    Swig_name_register("wrapper", "Rust_%f");
    Swig_name_register("construct", "new_%c");
    Swig_name_register("destroy", "delete_%c");

    Printf(f_wrappers, "\n#ifdef __cplusplus\n");
    Printf(f_wrappers, "extern \"C\" {\n");
    Printf(f_wrappers, "#endif\n\n");
    Printf(f_wrappers, "SWIGEXPORT void Rust_string_free(char *s) {\n");
    Printf(f_wrappers, "  free(s);\n");
    Printf(f_wrappers, "}\n\n");
    Printf(f_wrappers, "SWIGEXPORT char *Rust_take_exception(void) {\n");
    Printf(f_wrappers, "  char *message = SWIG_RustPendingException;\n");
    Printf(f_wrappers, "  SWIG_RustPendingException = 0;\n");
    Printf(f_wrappers, "  return message;\n");
    Printf(f_wrappers, "}\n\n");
    Printf(f_wrappers, "SWIGEXPORT char *Rust_take_exception_type(void) {\n");
    Printf(f_wrappers, "  char *kind = SWIG_RustPendingExceptionType;\n");
    Printf(f_wrappers, "  SWIG_RustPendingExceptionType = 0;\n");
    Printf(f_wrappers, "  return kind;\n");
    Printf(f_wrappers, "}\n\n");

    Swig_banner_target_lang(f_rust, "//");
    Printf(f_rust, "\n");
    Printf(f_rust, "#![allow(dead_code)]\n");
    Printf(f_rust, "#![allow(non_camel_case_types)]\n");
    Printf(f_rust, "#![allow(non_snake_case)]\n");
    Printf(f_rust, "#![allow(non_upper_case_globals)]\n");
    Printf(f_rust, "#![allow(path_statements)]\n");
    Printf(f_rust, "#![allow(unused_imports)]\n\n");
    Printf(f_rust,
           "use std::os::raw::{c_char, c_double, c_float, c_int, c_long, c_longlong, c_schar, c_short, c_uchar, c_uint, c_ulong, "
           "c_ulonglong, c_ushort, c_void};\n\n");
    Printf(f_rust, "// Rust bindings for the '%s' SWIG module.\n", crate_name);
    Printf(f_rust, "// Link the generated wrapper library from your Rust build, for example with\n");
    Printf(f_rust, "// 'cargo:rustc-link-lib=dylib=%s'.\n\n", module_name);
    Printf(rust_extern_code, "  #[link_name = \"Rust_string_free\"]\n");
    Printf(rust_extern_code, "  pub fn Rust_string_free_raw(s: *mut c_char);\n");
    Printf(rust_extern_code, "  #[link_name = \"Rust_take_exception\"]\n");
    Printf(rust_extern_code, "  pub fn Rust_take_exception_raw() -> *mut c_char;\n");
    Printf(rust_extern_code, "  #[link_name = \"Rust_take_exception_type\"]\n");
    Printf(rust_extern_code, "  pub fn Rust_take_exception_type_raw() -> *mut c_char;\n");

    Language::top(n);

    if (Swig_directors_enabled()) {
      Swig_insert_file("director_common.swg", f_runtime);
      Swig_insert_file("director.swg", f_runtime);
    }

    Printf(f_wrappers, "\n#ifdef __cplusplus\n");
    Printf(f_wrappers, "}\n");
    Printf(f_wrappers, "#endif\n");

    if (Swig_directors_enabled()) {
      Printf(f_runtime_h, "\n");
      Dump(f_directors_h, f_runtime_h);
      Printf(f_runtime_h, "\n#endif\n");
    }

    Printf(f_rust, "extern \"C\" {\n");
    Dump(rust_extern_code, f_rust);
    Printf(f_rust, "}\n");
    Printf(f_rust, "\n#[derive(Clone, Debug, Eq, PartialEq)]\n");
    Printf(f_rust, "pub struct RustException {\n");
    Printf(f_rust, "  pub kind: String,\n");
    Printf(f_rust, "  pub message: String,\n");
    Printf(f_rust, "}\n\n");
    Printf(f_rust, "impl std::fmt::Display for RustException {\n");
    Printf(f_rust, "  fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {\n");
    Printf(f_rust, "    write!(f, \"{}: {}\", self.kind, self.message)\n");
    Printf(f_rust, "  }\n");
    Printf(f_rust, "}\n\n");
    Printf(f_rust, "impl std::error::Error for RustException {}\n\n");
    Printf(f_rust, "fn rust_take_exception() -> Option<RustException> {\n");
    Printf(f_rust, "  unsafe {\n");
    Printf(f_rust, "    let kind = Rust_take_exception_type_raw();\n");
    Printf(f_rust, "    let message = Rust_take_exception_raw();\n");
    Printf(f_rust, "    if kind.is_null() && message.is_null() {\n");
    Printf(f_rust, "      return None;\n");
    Printf(f_rust, "    }\n");
    Printf(f_rust, "    let kind_text = if kind.is_null() {\n");
    Printf(f_rust, "      \"std::exception\".to_string()\n");
    Printf(f_rust, "    } else {\n");
    Printf(f_rust, "      let text = std::ffi::CStr::from_ptr(kind).to_string_lossy().into_owned();\n");
    Printf(f_rust, "      Rust_string_free_raw(kind);\n");
    Printf(f_rust, "      text\n");
    Printf(f_rust, "    };\n");
    Printf(f_rust, "    let message_text = if message.is_null() {\n");
    Printf(f_rust, "      \"C++ exception\".to_string()\n");
    Printf(f_rust, "    } else {\n");
    Printf(f_rust, "      let text = std::ffi::CStr::from_ptr(message).to_string_lossy().into_owned();\n");
    Printf(f_rust, "      Rust_string_free_raw(message);\n");
    Printf(f_rust, "      text\n");
    Printf(f_rust, "    };\n");
    Printf(f_rust, "    Some(RustException { kind: kind_text, message: message_text })\n");
    Printf(f_rust, "  }\n");
    Printf(f_rust, "}\n\n");
    Printf(f_rust, "fn rust_check_exception() {\n");
    Printf(f_rust, "  if let Some(error) = rust_take_exception() {\n");
    Printf(f_rust, "    panic!(\"{}\", error);\n");
    Printf(f_rust, "  }\n");
    Printf(f_rust, "}\n\n");
    Dump(rust_proxy_code, f_rust);

    Dump(f_runtime, f_begin);
    Dump(f_header, f_begin);
    if (Swig_directors_enabled())
      Dump(f_directors, f_begin);
    Dump(f_wrappers, f_begin);
    Wrapper_pretty_print(f_init, f_begin);

    Delete(f_init);
    Delete(f_wrappers);
    Delete(f_header);
    Delete(f_runtime);
    Delete(f_directors);
    Delete(f_directors_h);
    if (f_runtime_h)
      Delete(f_runtime_h);
    Delete(f_rust);
    Delete(f_begin);
    Delete(crate_name);
    Delete(rust_extern_code);
    Delete(rust_proxy_code);
    Delete(proxy_class_name);
    Delete(proxy_class_code);
    Delete(destructor_call);
    Delete(variable_name);
    Delete(dmethods_seq);
    Delete(dmethods_table);
    f_init = NULL;
    f_wrappers = NULL;
    f_header = NULL;
    f_runtime = NULL;
    f_directors = NULL;
    f_directors_h = NULL;
    f_runtime_h = NULL;
    f_rust = NULL;
    f_begin = NULL;
    crate_name = NULL;
    rust_extern_code = NULL;
    rust_proxy_code = NULL;
    proxy_class_name = NULL;
    proxy_class_code = NULL;
    destructor_call = NULL;
    variable_name = NULL;
    dmethods_seq = NULL;
    dmethods_table = NULL;
    return SWIG_OK;
  }

  virtual int functionWrapper(Node *n) {
    String *symname = Getattr(n, "sym:name");
    SwigType *returntype = Getattr(n, "type");
    ParmList *parms = Getattr(n, "parms");
    String *tm = NULL;

    if (!Getattr(n, "sym:overloaded")) {
      if (!addSymbol(symname, n))
        return SWIG_ERROR;
    }

    String *overloaded_name = Copy(symname);
    if (Getattr(n, "sym:overloaded"))
      Append(overloaded_name, Getattr(n, "sym:overname"));

    String *wname = Swig_name_wrapper(overloaded_name);
    Setattr(n, "wrap:name", wname);

    Wrapper *f = NewWrapper();
    String *return_ctype = NewString("");
    String *return_imtype = NewString("");
    String *cleanup = NewString("");
    String *outarg = NewString("");

    Swig_typemap_attach_parms("ctype", parms, f);
    Swig_typemap_attach_parms("imtype", parms, f);
    Swig_typemap_attach_parms("rusttype", parms, f);
    Swig_typemap_attach_parms("rustin", parms, f);

    if ((tm = Swig_typemap_lookup("ctype", n, "", 0))) {
      String *ctypeout = Getattr(n, "tmap:ctype:out");
      Printv(return_ctype, ctypeout ? ctypeout : tm, NIL);
    } else {
      Swig_warning(WARN_TYPEMAP_UNDEF, input_file, line_number, "No ctype typemap defined for %s\n", SwigType_str(returntype, 0));
      Printv(return_ctype, SwigType_str(returntype, 0), NIL);
    }

    if ((tm = Swig_typemap_lookup("imtype", n, "", 0))) {
      String *imtypeout = Getattr(n, "tmap:imtype:out");
      Printv(return_imtype, imtypeout ? imtypeout : tm, NIL);
    } else {
      String *fallback = rustType(returntype);
      Printv(return_imtype, fallback, NIL);
      Delete(fallback);
    }

    const bool is_void_return = Cmp(return_ctype, "void") == 0;

    if (!is_void_return)
      Wrapper_add_localv(f, "jresult", return_ctype, "jresult = 0", NIL);

    Printv(f->def, "SWIGEXPORT ", return_ctype, " ", wname, "(", NIL);
    emit_parameter_variables(parms, f);
    emit_attach_parmmaps(parms, f);
    Setattr(n, "wrap:parms", parms);

    String *rust_im_params = NewString("");
    String *rust_params = NewString("");
    String *rust_pre_code = NewString("");
    String *rust_args = NewString("");
    int gencomma = 0;
    int num_arguments = emit_num_arguments(parms);
    Parm *p = parms;
    for (int index = 0; index < num_arguments; index++) {
      while (p && checkAttribute(p, "tmap:in:numinputs", "0"))
        p = Getattr(p, "tmap:in:next");
      if (!p)
        break;

      SwigType *pt = Getattr(p, "type");
      String *lname = Getattr(p, "lname");
      String *arg = NewStringf("arg%d", index);
      String *c_arg_name = NewStringf("j%s", lname);
      String *c_param_type = Getattr(p, "tmap:ctype");
      String *im_param_type = Getattr(p, "tmap:imtype");
      String *rust_param_type = Getattr(p, "tmap:rusttype");
      String *rust_arg = Getattr(p, "tmap:rustin") ? Copy(Getattr(p, "tmap:rustin")) : Copy(arg);
      String *pre = Getattr(p, "tmap:rustin:pre");
      bool rust_param_type_owned = false;

      if (!c_param_type) {
        Swig_warning(WARN_TYPEMAP_UNDEF, input_file, line_number, "No ctype typemap defined for %s\n", SwigType_str(pt, 0));
        c_param_type = SwigType_str(pt, 0);
      }
      if (!im_param_type) {
        im_param_type = rustType(pt);
      } else {
        String *imtypeout = Getattr(p, "tmap:imtype:out");
        if (imtypeout)
          im_param_type = imtypeout;
      }
      if (!rust_param_type) {
        rust_param_type = rustType(pt);
        rust_param_type_owned = true;
      } else {
        String *rusttypeout = Getattr(p, "tmap:rusttype:out");
        if (rusttypeout)
          rust_param_type = rusttypeout;
      }
      applyRustProxyInput(pt, arg, &rust_param_type, &rust_arg, &rust_param_type_owned);

      Replaceall(rust_arg, "$rustinput", arg);
      Replaceall(rust_arg, "$iminput", arg);
      if (pre) {
        String *pre_code = Copy(pre);
        Replaceall(pre_code, "$rustinput", arg);
        Replaceall(pre_code, "$iminput", arg);
        Printf(rust_pre_code, "  %s\n", pre_code);
        Delete(pre_code);
      }

      Printv(f->def, gencomma ? ", " : "", c_param_type, " ", c_arg_name, NIL);
      Printv(rust_im_params, gencomma ? ", " : "", arg, ": ", im_param_type, NIL);
      Printv(rust_params, gencomma ? ", " : "", arg, ": ", rust_param_type, NIL);
      Printv(rust_args, gencomma ? ", " : "", rust_arg, NIL);
      gencomma = 1;

      if ((tm = Getattr(p, "tmap:in"))) {
        Replaceall(tm, "$arg", c_arg_name);
        Replaceall(tm, "$input", c_arg_name);
        Setattr(p, "emit:input", c_arg_name);
        Printf(f->code, "%s\n", tm);
        p = Getattr(p, "tmap:in:next");
      } else {
        Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
        p = nextSibling(p);
      }

      if (rust_param_type_owned)
        Delete(rust_param_type);
      Delete(rust_arg);
      Delete(c_arg_name);
      Delete(arg);
    }

    Printf(f->def, ") {");

    for (p = parms; p;) {
      if ((tm = Getattr(p, "tmap:freearg"))) {
        Replaceall(tm, "$arg", Getattr(p, "emit:input"));
        Replaceall(tm, "$input", Getattr(p, "emit:input"));
        Printv(cleanup, tm, "\n", NIL);
        p = Getattr(p, "tmap:freearg:next");
      } else {
        p = nextSibling(p);
      }
    }

    for (p = parms; p;) {
      if ((tm = Getattr(p, "tmap:argout"))) {
        Replaceall(tm, "$arg", Getattr(p, "emit:input"));
        Replaceall(tm, "$result", "jresult");
        Replaceall(tm, "$input", Getattr(p, "emit:input"));
        Printv(outarg, tm, "\n", NIL);
        p = Getattr(p, "tmap:argout:next");
      } else {
        p = nextSibling(p);
      }
    }

    String *actioncode = emit_action(n);
    Replaceall(actioncode, "$null", is_void_return ? "" : "0");
    if ((tm = Swig_typemap_lookup_out("out", n, Swig_cresult_name(), f, actioncode))) {
      Replaceall(tm, "$result", "jresult");
      Replaceall(tm, "$owner", GetFlag(n, "feature:new") ? "1" : "0");
      Printf(f->code, "%s\n", tm);
      emit_return_variable(n, returntype, f);
    } else {
      Swig_warning(
        WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(returntype, 0), Getattr(n, "name"));
    }
    Printv(f->code, outarg, NIL);
    Printv(f->code, cleanup, NIL);
    if (!is_void_return)
      Printf(f->code, "return jresult;\n");
    Printf(f->code, "}\n");

    Wrapper_print(f, f_wrappers);
    writeRustFunction(n, overloaded_name, wname, return_imtype, rust_im_params, rust_params, rust_pre_code, rust_args);
    emitRustVariableAccessor(n, overloaded_name);

    Delete(outarg);
    Delete(cleanup);
    Delete(rust_pre_code);
    Delete(rust_params);
    Delete(rust_im_params);
    Delete(rust_args);
    Delete(return_imtype);
    Delete(return_ctype);
    Delete(wname);
    Delete(overloaded_name);
    DelWrapper(f);

    return SWIG_OK;
  }

  virtual int constantWrapper(Node *n) {
    String *symname = Getattr(n, "sym:name");
    String *value = Getattr(n, "value");
    SwigType *type = Getattr(n, "type");
    String *rust_type = rustType(type);

    if (!addSymbol(symname, n)) {
      Delete(rust_type);
      return SWIG_ERROR;
    }

    if (Getattr(n, "wrappedasconstant") && Getattr(n, "staticmembervariableHandler:value"))
      value = Getattr(n, "staticmembervariableHandler:value");

    if (value && rustTypeIsPrimitive(type)) {
      if (Getattr(n, "wrappedasconstant") && Getattr(n, "staticmembervariableHandler:value") && proxy_class_code && proxy_class_name) {
        String *prefix = NewStringf("%s_", proxy_class_name);
        String *member_name = Copy(symname);
        if (Strncmp(member_name, prefix, Len(prefix)) == 0) {
          String *stripped = NewString(Char(member_name) + Len(prefix));
          Delete(member_name);
          member_name = stripped;
        }
        String *rust_name = rustAssociatedConstantName(member_name);
        Printf(proxy_class_code, "  pub const %s: %s = %s;\n", rust_name, rust_type, value);
        Delete(rust_name);
        Delete(member_name);
        Delete(prefix);
      } else {
        String *rust_name = rustIdentifier(symname);
        Printf(f_rust, "pub const %s: %s = %s;\n", rust_name, rust_type, value);
        Delete(rust_name);
      }
    } else {
      Swig_warning(WARN_LANG_NATIVE_UNIMPL, input_file, line_number, "Rust constant '%s' has unsupported type %s\n", symname, SwigType_str(type, 0));
    }

    Delete(rust_type);
    return SWIG_OK;
  }

  virtual int variableWrapper(Node *n) {
    return Language::variableWrapper(n);
  }

  virtual int globalvariableHandler(Node *n) {
    Delete(variable_name);
    variable_name = Copy(Getattr(n, "sym:name"));
    global_variable_flag = true;
    int result = Language::globalvariableHandler(n);
    global_variable_flag = false;
    return result;
  }

  virtual int membervariableHandler(Node *n) {
    Delete(variable_name);
    variable_name = Copy(Getattr(n, "sym:name"));
    member_variable_flag = true;
    int result = Language::membervariableHandler(n);
    member_variable_flag = false;
    return result;
  }

  virtual int staticmembervariableHandler(Node *n) {
    Delete(variable_name);
    variable_name = Copy(Getattr(n, "sym:name"));
    static_member_variable_flag = true;
    int result = Language::staticmembervariableHandler(n);
    static_member_variable_flag = false;
    return result;
  }

  virtual int classHandler(Node *n) {
    String *old_proxy_class_name = proxy_class_name;
    String *old_proxy_class_code = proxy_class_code;
    String *old_destructor_call = destructor_call;
    String *classname = Getattr(n, "sym:name");

    proxy_class_name = classname ? Copy(classname) : NULL;
    proxy_class_code = NewString("");
    destructor_call = NewString("");

    int result = Language::classHandler(n);

    if (classname) {
      Printf(rust_proxy_code, "pub struct %s {\n", classname);
      Printf(rust_proxy_code, "  ptr: *mut c_void,\n");
      Printf(rust_proxy_code, "  owned: bool,\n");
      Printf(rust_proxy_code, "}\n\n");
      Printf(rust_proxy_code, "impl %s {\n", classname);
      Printf(rust_proxy_code, "  pub unsafe fn from_raw(ptr: *mut c_void) -> Self {\n");
      Printf(rust_proxy_code, "    Self::from_raw_owned(ptr, true)\n");
      Printf(rust_proxy_code, "  }\n");
      Printf(rust_proxy_code, "  pub unsafe fn borrowed(ptr: *mut c_void) -> Self {\n");
      Printf(rust_proxy_code, "    Self::from_raw_owned(ptr, false)\n");
      Printf(rust_proxy_code, "  }\n");
      Printf(rust_proxy_code, "  pub unsafe fn from_raw_owned(ptr: *mut c_void, owned: bool) -> Self {\n");
      Printf(rust_proxy_code, "    Self { ptr, owned }\n");
      Printf(rust_proxy_code, "  }\n");
      Printf(rust_proxy_code, "  pub fn as_ptr(&self) -> *mut c_void {\n");
      Printf(rust_proxy_code, "    self.ptr\n");
      Printf(rust_proxy_code, "  }\n");
      Printf(rust_proxy_code, "  pub fn disown(&mut self) {\n");
      Printf(rust_proxy_code, "    self.owned = false;\n");
      Printf(rust_proxy_code, "  }\n");
      Printf(rust_proxy_code, "  pub fn into_raw(mut self) -> *mut c_void {\n");
      Printf(rust_proxy_code, "    let ptr = self.ptr;\n");
      Printf(rust_proxy_code, "    self.ptr = std::ptr::null_mut();\n");
      Printf(rust_proxy_code, "    self.owned = false;\n");
      Printf(rust_proxy_code, "    std::mem::forget(self);\n");
      Printf(rust_proxy_code, "    ptr\n");
      Printf(rust_proxy_code, "  }\n");
      if (Len(destructor_call) > 0) {
        Printf(rust_proxy_code, "  unsafe fn delete_owned(&mut self) {\n");
        Printf(rust_proxy_code, "    if self.owned && !self.ptr.is_null() {\n");
        Printf(rust_proxy_code, "      %s;\n", destructor_call);
        Printf(rust_proxy_code, "      self.ptr = std::ptr::null_mut();\n");
        Printf(rust_proxy_code, "      self.owned = false;\n");
        Printf(rust_proxy_code, "    }\n");
        Printf(rust_proxy_code, "  }\n");
        Printf(rust_proxy_code, "  pub unsafe fn delete(mut self) {\n");
        Printf(rust_proxy_code, "    self.delete_owned();\n");
        Printf(rust_proxy_code, "    std::mem::forget(self);\n");
        Printf(rust_proxy_code, "  }\n");
      }
      Printv(rust_proxy_code, proxy_class_code, NIL);
      Printf(rust_proxy_code, "}\n\n");
      if (Len(destructor_call) > 0) {
        Printf(rust_proxy_code, "impl Drop for %s {\n", classname);
        Printf(rust_proxy_code, "  fn drop(&mut self) {\n");
        Printf(rust_proxy_code, "    unsafe { self.delete_owned(); }\n");
        Printf(rust_proxy_code, "  }\n");
        Printf(rust_proxy_code, "}\n\n");
      }
    }

    if (proxy_class_name && proxy_class_name != old_proxy_class_name)
      Delete(proxy_class_name);
    proxy_class_name = old_proxy_class_name;
    if (proxy_class_code && proxy_class_code != old_proxy_class_code)
      Delete(proxy_class_code);
    proxy_class_code = old_proxy_class_code;
    if (destructor_call && destructor_call != old_destructor_call)
      Delete(destructor_call);
    destructor_call = old_destructor_call;

    return result;
  }

  virtual int memberfunctionHandler(Node *n) {
    Language::memberfunctionHandler(n);
    emitProxyFunction(n, false);
    emitProxyFunction(n, false, true);
    return SWIG_OK;
  }

  virtual int staticmemberfunctionHandler(Node *n) {
    Language::staticmemberfunctionHandler(n);
    emitProxyFunction(n, true);
    emitProxyFunction(n, true, true);
    return SWIG_OK;
  }

  virtual int constructorHandler(Node *n) {
    Language::constructorHandler(n);
    emitProxyConstructor(n);
    emitProxyConstructor(n, true);
    return SWIG_OK;
  }

  virtual int destructorHandler(Node *n) {
    Language::destructorHandler(n);
    String *symname = Getattr(n, "sym:name");
    if (proxy_class_name && destructor_call) {
      Clear(destructor_call);
      Printf(destructor_call, "%s_raw(self.ptr)", Swig_name_destroy(getNSpace(), symname));
    }
    return SWIG_OK;
  }

  virtual int enumDeclaration(Node *n) {
    if (ImportMode)
      return SWIG_OK;

    String *symname = Getattr(n, "sym:name");
    String *old_enum_code = proxy_class_code;
    String *enum_code = NewString("");
    String *rust_enum_name = symname ? rustIdentifier(symname) : NULL;
    String *scope = NULL;
    proxy_class_code = enum_code;

    if (symname && !Getattr(n, "unnamedinstance")) {
      if (getCurrentClass()) {
        Delete(rust_enum_name);
        rust_enum_name = Swig_name_member(getNSpace(), getClassPrefix(), symname);
        scope = Copy(getClassPrefix());
      }
      if (!addSymbol(symname, n, scope))
        return SWIG_ERROR;
      Setattr(n, "rust:enumname", rust_enum_name);
      Printf(enum_code, "#[repr(C)]\n");
      Printf(enum_code, "#[derive(Copy, Clone, Debug, PartialEq, Eq)]\n");
      Printf(enum_code, "pub enum %s {\n", rust_enum_name);
      Language::enumDeclaration(n);
      Printf(enum_code, "}\n\n");
      Printv(rust_proxy_code, enum_code, NIL);
    } else {
      Language::enumDeclaration(n);
    }

    proxy_class_code = old_enum_code;
    Delete(scope);
    Delete(rust_enum_name);
    Delete(enum_code);
    return SWIG_OK;
  }

  virtual int enumvalueDeclaration(Node *n) {
    Swig_require("enumvalueDeclaration", n, "*name", "?value", NIL);
    String *symname = Getattr(n, "sym:name");
    String *value = Getattr(n, "enumvalue") ? Getattr(n, "enumvalue") : Getattr(n, "value");
    Node *parent = parentNode(n);
    String *parent_name = Getattr(parent, "sym:name");

    if (parent_name && proxy_class_code) {
      String *scope = Getattr(parent, "rust:enumname") ? Copy(Getattr(parent, "rust:enumname")) : Copy(parent_name);
      String *rust_name = rustIdentifier(symname);
      if (!addSymbol(symname, n, scope)) {
        Delete(rust_name);
        Delete(scope);
        return SWIG_ERROR;
      }
      Printf(proxy_class_code, "  %s", rust_name);
      if (value)
        Printf(proxy_class_code, " = %s", value);
      Printf(proxy_class_code, ",\n");
      Delete(rust_name);
      Delete(scope);
    } else if (value) {
      String *rust_name = rustIdentifier(symname);
      if (!addSymbol(symname, n)) {
        Delete(rust_name);
        return SWIG_ERROR;
      }
      Printf(rust_proxy_code, "pub const %s: c_int = %s;\n", rust_name, value);
      Delete(rust_name);
    }

    Swig_restore(n);
    return SWIG_OK;
  }

  virtual int classDirectorInit(Node *n) {
    Delete(director_ctor_code);
    director_ctor_code = NewString("$director_new");

    directorDeclaration(n);

    Printf(f_directors_h, "%s {\n", Getattr(n, "director:decl"));
    Printf(f_directors_h, "\npublic:\n");

    first_class_dmethod = curr_class_dmethod = n_dmethods;
    director_callback_typedefs = NewString("");
    director_callbacks = NewString("");
    director_connect_parms = NewString("");
    director_trait_code = NewStringf("pub trait %sDirector {\n", Getattr(n, "sym:name"));
    director_callback_shims = NewString("");
    director_method_metadata = NewString("");

    return Language::classDirectorInit(n);
  }

  virtual int classDirectorConstructor(Node *n) {
    Node *parent = parentNode(n);
    String *decl = Getattr(n, "decl");
    String *dirclassname = directorClassName(parent);
    ParmList *superparms = Getattr(n, "parms");
    ParmList *parms = CopyParmList(superparms);
    int argidx = 0;

    for (Parm *p = superparms; p; p = nextSibling(p)) {
      String *pname = Getattr(p, "name");
      if (!pname) {
        pname = NewStringf("arg%d", argidx++);
        Setattr(p, "name", pname);
      }
    }

    if (!Getattr(n, "defaultargs")) {
      String *basetype = Getattr(parent, "classtype");
      String *target = Swig_method_decl(0, decl, dirclassname, parms, 0);
      String *call = Swig_csuperclass_call(0, basetype, superparms);

      Printf(f_directors, "%s::%s : %s, %s {\n", dirclassname, target, call, Getattr(parent, "director:ctor"));
      Printf(f_directors, "  swig_init_callbacks();\n");
      Printf(f_directors, "}\n\n");

      Delete(target);
      Delete(call);

      target = Swig_method_decl(0, decl, dirclassname, parms, 1);
      Printf(f_directors_h, "    %s;\n", target);
      Delete(target);
    }

    Delete(parms);
    Delete(dirclassname);
    return Language::classDirectorConstructor(n);
  }

  virtual int classDirectorDefaultConstructor(Node *n) {
    String *dirclassname = directorClassName(n);
    Wrapper *w = NewWrapper();

    Printf(w->def, "%s::%s() : %s {", dirclassname, dirclassname, Getattr(n, "director:ctor"));
    Printf(w->code, "  swig_init_callbacks();\n");
    Printf(w->code, "}\n");
    Wrapper_print(w, f_directors);

    Printf(f_directors_h, "    %s();\n", dirclassname);
    DelWrapper(w);
    Delete(dirclassname);
    return Language::classDirectorDefaultConstructor(n);
  }

  virtual int classDirectorMethod(Node *n, Node *parent, String *super) {
    String *classname = Getattr(parent, "sym:name");
    String *c_classname = Getattr(parent, "name");
    String *name = Getattr(n, "name");
    String *symname = Getattr(n, "sym:name");
    SwigType *returntype = Getattr(n, "type");
    String *storage = Getattr(n, "storage");
    String *value = Getattr(n, "value");
    String *decl = Getattr(n, "decl");
    ParmList *l = Getattr(n, "parms");
    bool is_void = Cmp(returntype, "void") == 0;
    bool pure_virtual = Cmp(storage, "virtual") == 0 && Cmp(value, "0") == 0;
    bool ignored_method = GetFlag(n, "feature:ignore") ? true : false;
    bool output_director = true;
    String *overloaded_name = ignored_method ? NULL : getOverloadedName(n);
    String *dirclassname = directorClassName(parent);
    String *qualified_name = NewStringf("%s::%s", dirclassname, name);
    String *qualified_return = SwigType_rcaststr(returntype, "c_result");
    String *c_ret_type = NULL;
    String *callback_typedef_parms = NewString("");
    String *callback_args = NewString("");
    String *rust_callback_type = NewString("");
    String *rust_trait_params = NewString("");
    String *rust_shim_params = NewString("");
    String *rust_shim_args = NewString("");
    String *rust_shim_pre_code = NewString("");
    String *rust_shim_post_code = NewString("");
    String *rust_shim_terminator_code = NewString("");
    String *rust_method_types = NewString("");
    String *declaration = NewString("");
    Wrapper *w = NewWrapper();
    int status = SWIG_OK;

    if (!is_void && (!ignored_method || pure_virtual)) {
      if (SwigType_ispointer(returntype) || SwigType_isreference(returntype)) {
        Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), "= 0", NIL);
      } else {
        String *construct_result = NewStringf("= SwigValueInit< %s >()", SwigType_lstr(returntype, 0));
        Wrapper_add_localv(w, "c_result", SwigType_lstr(returntype, "c_result"), construct_result, NIL);
        Delete(construct_result);
      }
    }

    c_ret_type = Swig_typemap_lookup("ctype", n, "", 0);
    if (c_ret_type) {
      String *ctypeout = Getattr(n, "tmap:ctype:out");
      if (ctypeout)
        c_ret_type = ctypeout;
      if (!is_void && !ignored_method) {
        String *jretval_decl = NewStringf("%s jresult", c_ret_type);
        Wrapper_add_localv(w, "jresult", jretval_decl, "= 0", NIL);
        Delete(jretval_decl);
      }
    } else {
      Swig_warning(WARN_TYPEMAP_UNDEF,
                   input_file,
                   line_number,
                   "No ctype typemap defined for %s for use in %s::%s (skipping director method)\n",
                   SwigType_str(returntype, 0),
                   SwigType_namestr(c_classname),
                   SwigType_namestr(name));
      output_director = false;
    }

    Swig_director_parms_fixup(l);
    Swig_typemap_attach_parms("out", l, 0);
    Swig_typemap_attach_parms("ctype", l, 0);
    Swig_typemap_attach_parms("imtype", l, 0);
    Swig_typemap_attach_parms("rusttype", l, 0);
    Swig_typemap_attach_parms("directorin", l, w);
    Swig_typemap_attach_parms("rustdirectorin", l, 0);
    Swig_typemap_attach_parms("directorargout", l, w);

    if (!ignored_method)
      Printf(w->code, "if (!swig_callback%s) {\n", overloaded_name);

    if (!pure_virtual) {
      String *super_call = Swig_method_call(super, l);
      if (is_void) {
        Printf(w->code, "%s;\n", super_call);
        if (!ignored_method)
          Printf(w->code, "return;\n");
      } else {
        Printf(w->code, "return %s;\n", super_call);
      }
      Delete(super_call);
    } else {
      Printf(w->code, "Swig::DirectorPureVirtualException::raise(\"%s::%s\");\n", SwigType_namestr(c_classname), SwigType_namestr(name));
      if (!is_void)
        Printf(w->code, "return %s;\n", qualified_return);
      else if (!ignored_method)
        Printf(w->code, "return;\n");
    }

    if (!ignored_method)
      Printf(w->code, "} else {\n");

    int i = 0;
    for (Parm *p = l; p;) {
      while (p && checkAttribute(p, "tmap:directorin:numinputs", "0"))
        p = Getattr(p, "tmap:directorin:next");
      if (!p)
        break;

      SwigType *pt = Getattr(p, "type");
      if (SwigType_type(pt) == T_VOID) {
        p = nextSibling(p);
        i++;
        continue;
      }

      String *ln = makeParameterName(n, p, i, false);
      String *arg = NewStringf("j%s", ln);
      String *rust_arg = NewStringf("arg%d", i);
      String *c_param_type = Getattr(p, "tmap:ctype");
      String *c_decl = NewString("");
      String *im_type = Getattr(p, "tmap:imtype");
      String *rust_type = Getattr(p, "tmap:rusttype");
      String *owned_im_type = NULL;
      String *owned_rust_type = NULL;

      if (c_param_type) {
        String *ctypeout = Getattr(p, "tmap:ctype:out");
        if (ctypeout)
          c_param_type = ctypeout;
        if (im_type) {
          String *imtypeout = Getattr(p, "tmap:imtype:out");
          if (imtypeout)
            im_type = imtypeout;
        } else {
          owned_im_type = rustType(pt);
          im_type = owned_im_type;
        }
        if (rust_type) {
          String *rusttypeout = Getattr(p, "tmap:rusttype:out");
          if (rusttypeout)
            rust_type = rusttypeout;
        } else {
          owned_rust_type = rustType(pt);
          rust_type = owned_rust_type;
        }

        Printf(c_decl, "%s %s", c_param_type, arg);
        if (!ignored_method)
          Wrapper_add_localv(w, arg, c_decl, SwigType_ispointer(pt) || SwigType_isreference(pt) ? "= 0" : "", NIL);

        String *tm = Getattr(p, "tmap:directorin");
        if (tm) {
          Setattr(p, "emit:directorinput", arg);
          Replaceall(tm, "$input", arg);
          Replaceall(tm, "$owner", "0");
          if (Len(tm) && !ignored_method)
            Printf(w->code, "%s\n", tm);

          if (Len(callback_typedef_parms))
            Printf(callback_typedef_parms, ", ");
          Printf(callback_typedef_parms, "%s", c_param_type);

          Printf(callback_args, ", %s", arg);

          if (Len(rust_callback_type))
            Printf(rust_callback_type, ", ");
          Printf(rust_callback_type, "%s", im_type);

          if (Len(rust_trait_params))
            Printf(rust_trait_params, ", ");
          Printf(rust_trait_params, "arg%d: %s", i, rust_type);

          if (Len(rust_shim_params))
            Printf(rust_shim_params, ", ");
          Printf(rust_shim_params, "arg%d: %s", i, im_type);

          String *din = Copy(Getattr(p, "tmap:rustdirectorin"));
          if (din) {
            Replaceall(din, "$iminput", rust_arg);
            Replaceall(din, "$rustinput", rust_arg);
            Replaceall(din, "$input", rust_arg);
            if (Len(rust_shim_args))
              Printf(rust_shim_args, ", ");
            Printv(rust_shim_args, din, NIL);

            String *pre = Getattr(p, "tmap:rustdirectorin:pre");
            if (pre) {
              String *pre_code = Copy(pre);
              Replaceall(pre_code, "$iminput", rust_arg);
              Replaceall(pre_code, "$rustinput", rust_arg);
              Replaceall(pre_code, "$input", rust_arg);
              Printf(rust_shim_pre_code, "    %s\n", pre_code);
              Delete(pre_code);
            }
            String *post = Getattr(p, "tmap:rustdirectorin:post");
            if (post) {
              String *post_code = Copy(post);
              Replaceall(post_code, "$iminput", rust_arg);
              Replaceall(post_code, "$rustinput", rust_arg);
              Replaceall(post_code, "$input", rust_arg);
              Printf(rust_shim_post_code, "    %s\n", post_code);
              Delete(post_code);
            }
            String *terminator = Getattr(p, "tmap:rustdirectorin:terminator");
            if (terminator) {
              String *terminator_code = Copy(terminator);
              Replaceall(terminator_code, "$iminput", rust_arg);
              Replaceall(terminator_code, "$rustinput", rust_arg);
              Replaceall(terminator_code, "$input", rust_arg);
              String *indented_terminator_code = NewStringf("    %s\n", terminator_code);
              Insert(rust_shim_terminator_code, 0, indented_terminator_code);
              Delete(indented_terminator_code);
              Delete(terminator_code);
            }
            Delete(din);
          } else {
            Swig_warning(WARN_TYPEMAP_DIRECTORIN_UNDEF,
                         input_file,
                         line_number,
                         "No rustdirectorin typemap defined for argument %s for use in %s::%s (skipping director method)\n",
                         SwigType_str(pt, 0),
                         SwigType_namestr(c_classname),
                         SwigType_namestr(name));
            output_director = false;
          }

          if (Len(rust_method_types))
            Printf(rust_method_types, ", ");
          Printf(rust_method_types, "\"%s\"", rust_type);

          Parm *next = Getattr(p, "tmap:directorin:next");
          p = next ? next : nextSibling(p);
        } else {
          Swig_warning(WARN_TYPEMAP_DIRECTORIN_UNDEF,
                       input_file,
                       line_number,
                       "No directorin typemap defined for argument %s for use in %s::%s (skipping director method)\n",
                       SwigType_str(pt, 0),
                       SwigType_namestr(c_classname),
                       SwigType_namestr(name));
          output_director = false;
          p = nextSibling(p);
        }
      } else {
        Swig_warning(WARN_TYPEMAP_UNDEF,
                     input_file,
                     line_number,
                     "No ctype typemap defined for argument %s for use in %s::%s (skipping director method)\n",
                     SwigType_str(pt, 0),
                     SwigType_namestr(c_classname),
                     SwigType_namestr(name));
        output_director = false;
        p = nextSibling(p);
      }

      Delete(owned_im_type);
      Delete(owned_rust_type);
      Delete(c_decl);
      Delete(arg);
      Delete(rust_arg);
      Delete(ln);
      i++;
    }

    String *target = Swig_method_decl(Getattr(n, "conversion_operator") ? 0 : Getattr(n, "classDirectorMethods:type"), decl, qualified_name, l, 0);
    Printf(w->def, "%s", target);
    Delete(target);
    target = Swig_method_decl(Getattr(n, "conversion_operator") ? 0 : Getattr(n, "classDirectorMethods:type"), decl, name, l, 1);
    Printf(declaration, "    virtual %s", target);
    Delete(target);

    if (Getattr(n, "noexcept")) {
      Append(w->def, " noexcept");
      Append(declaration, " noexcept");
    }

    ParmList *throw_parm_list = NULL;
    if ((throw_parm_list = Getattr(n, "throws")) || Getattr(n, "throw")) {
      int gencomma = 0;

      Append(w->def, " throw(");
      Append(declaration, " throw(");

      if (throw_parm_list)
        Swig_typemap_attach_parms("throws", throw_parm_list, 0);
      for (Parm *p = throw_parm_list; p; p = nextSibling(p)) {
        if (Getattr(p, "tmap:throws")) {
          if (gencomma++) {
            Append(w->def, ", ");
            Append(declaration, ", ");
          }
          Printf(w->def, "%s", SwigType_str(Getattr(p, "type"), 0));
          Printf(declaration, "%s", SwigType_str(Getattr(p, "type"), 0));
        }
      }

      Append(w->def, ")");
      Append(declaration, ")");
    }

    Append(w->def, " {");
    Append(declaration, ";\n");

    if (!ignored_method) {
      if (!is_void) {
        Printf(w->code, "jresult = ");
      }
      Printf(w->code, "swig_callback%s(swig_rust_object%s);\n", overloaded_name, callback_args);
      if (!is_void) {
        String *jresult_str = NewString("jresult");
        String *result_str = NewString("c_result");
        String *tm = Swig_typemap_lookup("directorout", n, result_str, w);
        if (tm) {
          Replaceall(tm, "$input", jresult_str);
          Replaceall(tm, "$result", result_str);
          Printf(w->code, "%s\n", tm);
        } else {
          Swig_warning(WARN_TYPEMAP_DIRECTOROUT_UNDEF,
                       input_file,
                       line_number,
                       "Unable to use return type %s used in %s::%s (skipping director method)\n",
                       SwigType_str(returntype, 0),
                       SwigType_namestr(c_classname),
                       SwigType_namestr(name));
          output_director = false;
        }
        Delete(jresult_str);
        Delete(result_str);
      }

      for (Parm *p = l; p;) {
        String *tm = Getattr(p, "tmap:directorargout");
        if (tm) {
          Replaceall(tm, "$result", "jresult");
          Replaceall(tm, "$input", Getattr(p, "emit:directorinput"));
          Printv(w->code, tm, "\n", NIL);
          p = Getattr(p, "tmap:directorargout:next");
        } else {
          p = nextSibling(p);
        }
      }

      Printf(w->code, "}\n");
      if (!is_void)
        Printf(w->code, "return %s;\n", qualified_return);
    }

    Printf(w->code, "}");

    String *inline_extra_method = NewString("");
    if (dirprot_mode() && !is_public(n) && !pure_virtual) {
      Printv(inline_extra_method, declaration, NIL);
      String *extra_method_name = NewStringf("%sSwigPublic", name);
      Replaceall(inline_extra_method, name, extra_method_name);
      Replaceall(inline_extra_method, ";\n", " {\n      ");
      if (!is_void)
        Printf(inline_extra_method, "return ");
      String *methodcall = Swig_method_call(super, l);
      Printv(inline_extra_method, methodcall, ";\n    }\n", NIL);
      Delete(methodcall);
      Delete(extra_method_name);
    }

    if (!ignored_method && output_director) {
      String *member_name = Swig_name_member(getNSpace(), getClassPrefix(), overloaded_name);
      String *rust_dmethod = NewStringf("SwigDirector_%s", member_name);
      UpcallData *udata = addUpcallMethod(rust_dmethod, symname, decl, overloaded_name);
      String *methid = Getattr(udata, "class_methodidx");
      String *rust_return = rustType(returntype);
      String *im_return = Swig_typemap_lookup("imtype", n, "", 0);
      if (im_return) {
        String *imtypeout = Getattr(n, "tmap:imtype:out");
        if (imtypeout)
          im_return = imtypeout;
      }
      String *rust_callback_shim = NewStringf("%s_director_callback_%s", classname, methid);
      String *rust_trait_name = rustIdentifier(overloaded_name);

      Setattr(n, "upcalldata", udata);
      Setattr(udata, "rust_callback_shim", rust_callback_shim);
      Printf(director_callback_typedefs, "    typedef %s (* SWIG_Callback%s_t)(void *rustobj", c_ret_type, methid);
      if (Len(callback_typedef_parms))
        Printf(director_callback_typedefs, ", %s", callback_typedef_parms);
      Printf(director_callback_typedefs, ");\n");
      Printf(director_callbacks, "    SWIG_Callback%s_t swig_callback%s;\n", methid, overloaded_name);
      if (Len(director_connect_parms))
        Printf(director_connect_parms, ", ");
      Printf(director_connect_parms, "SWIG_Callback%s_t callback%s", methid, methid);

      Printf(rust_proxy_code, "pub type %sCallback%s = extern \"C\" fn(*mut c_void", classname, methid);
      if (Len(rust_callback_type))
        Printf(rust_proxy_code, ", %s", rust_callback_type);
      Printf(rust_proxy_code, ")");
      if (Cmp(rust_return, "()") != 0)
        Printf(rust_proxy_code, " -> %s", rust_return);
      Printf(rust_proxy_code, ";\n");

      Printf(director_trait_code, "  unsafe fn %s(&mut self%s%s)", rust_trait_name, Len(rust_trait_params) ? ", " : "", rust_trait_params);
      if (Cmp(rust_return, "()") != 0)
        Printf(director_trait_code, " -> %s", rust_return);
      Printf(director_trait_code, ";\n");

      Printf(director_method_metadata, "pub const %sDirectorMethodTypes%s: &[&str] = &[%s];\n", classname, methid, rust_method_types);

      Printf(director_callback_shims, "extern \"C\" fn %s<T: %sDirector>(rustobj: *mut c_void", rust_callback_shim, classname);
      if (Len(rust_shim_params))
        Printf(director_callback_shims, ", %s", rust_shim_params);
      Printf(director_callback_shims, ")");
      if (im_return && Cmp(im_return, "()") != 0)
        Printf(director_callback_shims, " -> %s", im_return);
      Printf(director_callback_shims, " {\n");
      Printf(director_callback_shims, "  unsafe {\n");
      Printf(director_callback_shims, "    let director = &mut *(rustobj as *mut T);\n");
      Printv(director_callback_shims, rust_shim_pre_code, NIL);
      if (Cmp(rust_return, "()") == 0) {
        Printf(director_callback_shims, "    director.%s(%s);\n", rust_trait_name, rust_shim_args);
      } else {
        Printf(director_callback_shims, "    let swig_result = director.%s(%s);\n", rust_trait_name, rust_shim_args);
        String *tm = Swig_typemap_lookup("rustdirectorout", n, "", 0);
        if (tm) {
          String *converted = Copy(tm);
          Replaceall(converted, "$rustcall", "swig_result");
          Replaceall(converted, "$imcall", "swig_result");
          Printf(director_callback_shims, "    let swig_result = %s;\n", converted);
          Delete(converted);
        } else {
          Swig_warning(WARN_TYPEMAP_DIRECTOROUT_UNDEF,
                       input_file,
                       line_number,
                       "Unable to use Rust return type %s used in %s::%s (skipping director method)\n",
                       SwigType_str(returntype, 0),
                       SwigType_namestr(c_classname),
                       SwigType_namestr(name));
          output_director = false;
        }
      }
      Printv(director_callback_shims, rust_shim_post_code, NIL);
      Printv(director_callback_shims, rust_shim_terminator_code, NIL);
      if (Cmp(rust_return, "()") != 0)
        Printf(director_callback_shims, "    swig_result\n");
      Printf(director_callback_shims, "  }\n");
      Printf(director_callback_shims, "}\n\n");

      Delete(rust_trait_name);
      Delete(rust_return);
      Delete(rust_callback_shim);
      Delete(rust_dmethod);
      Delete(member_name);
    }

    if (status == SWIG_OK && output_director) {
      Replaceall(w->code, "$null", is_void ? "" : qualified_return);
      Replaceall(w->code, "$isvoid", is_void ? "1" : "0");
      Replaceall(w->code, "$symname", symname);
      if (!Getattr(n, "defaultargs")) {
        Wrapper_print(w, f_directors);
        Printv(f_directors_h, declaration, NIL);
        Printv(f_directors_h, inline_extra_method, NIL);
      }
    }

    Delete(inline_extra_method);
    Delete(qualified_return);
    Delete(qualified_name);
    Delete(dirclassname);
    Delete(callback_typedef_parms);
    Delete(callback_args);
    Delete(rust_callback_type);
    Delete(rust_trait_params);
    Delete(rust_shim_params);
    Delete(rust_shim_args);
    Delete(rust_shim_pre_code);
    Delete(rust_shim_post_code);
    Delete(rust_shim_terminator_code);
    Delete(rust_method_types);
    Delete(declaration);
    Delete(overloaded_name);
    DelWrapper(w);
    return status;
  }

  virtual int classDirectorDestructor(Node *n) {
    Node *current_class = getCurrentClass();
    String *dirclassname = directorClassName(current_class);
    Wrapper *w = NewWrapper();

    if (Getattr(n, "noexcept")) {
      Printf(f_directors_h, "    virtual ~%s() noexcept;\n", dirclassname);
      Printf(w->def, "%s::~%s() noexcept {\n", dirclassname, dirclassname);
    } else if (Getattr(n, "throw")) {
      Printf(f_directors_h, "    virtual ~%s() throw();\n", dirclassname);
      Printf(w->def, "%s::~%s() throw() {\n", dirclassname, dirclassname);
    } else {
      Printf(f_directors_h, "    virtual ~%s();\n", dirclassname);
      Printf(w->def, "%s::~%s() {\n", dirclassname, dirclassname);
    }

    Printf(w->code, "}\n");
    Wrapper_print(w, f_directors);

    DelWrapper(w);
    Delete(dirclassname);
    return SWIG_OK;
  }

  virtual int classDirectorEnd(Node *n) {
    String *dirclassname = directorClassName(n);
    Wrapper *w = NewWrapper();

    if (Len(director_callback_typedefs) > 0)
      Printf(f_directors_h, "\n%s", director_callback_typedefs);

    Printf(f_directors_h, "    void swig_connect_director(void *rustobj");
    Printf(w->def, "void %s::swig_connect_director(void *rustobj", dirclassname);
    Printf(w->code, "swig_rust_object = rustobj;\n");

    for (int i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *methid = Getattr(udata, "class_methodidx");
      String *overname = Getattr(udata, "overname");

      Printf(f_directors_h, ", SWIG_Callback%s_t callback%s", methid, overname);
      Printf(w->def, ", SWIG_Callback%s_t callback%s", methid, overname);
      Printf(w->code, "swig_callback%s = callback%s;\n", overname, overname);
    }

    Printf(f_directors_h, ");\n");
    Printf(w->def, ") {");
    Printf(f_directors_h, "\nprivate:\n");
    if (Len(director_callbacks) > 0)
      Printf(f_directors_h, "%s", director_callbacks);
    Printf(f_directors_h, "    void swig_init_callbacks();\n");
    Printf(f_directors_h, "    void *swig_rust_object;\n");
    Printf(f_directors_h, "};\n\n");
    Printf(w->code, "}\n\n");

    Printf(w->code, "void %s::swig_init_callbacks() {\n", dirclassname);
    Printf(w->code, "swig_rust_object = 0;\n");
    for (int i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *overname = Getattr(udata, "overname");
      Printf(w->code, "swig_callback%s = 0;\n", overname);
    }
    Printf(w->code, "}");

    Wrapper_print(w, f_directors);
    emitDirectorConnect(n);
    if (director_trait_code) {
      Printf(director_trait_code, "}\n");
      Printv(rust_proxy_code, director_trait_code, NIL);
    }
    if (director_method_metadata)
      Printv(rust_proxy_code, director_method_metadata, NIL);
    if (director_callback_shims)
      Printv(rust_proxy_code, director_callback_shims, NIL);

    Delete(director_callback_typedefs);
    Delete(director_callbacks);
    Delete(director_connect_parms);
    Delete(director_trait_code);
    Delete(director_callback_shims);
    Delete(director_method_metadata);
    director_callback_typedefs = NULL;
    director_callbacks = NULL;
    director_connect_parms = NULL;
    director_trait_code = NULL;
    director_callback_shims = NULL;
    director_method_metadata = NULL;

    DelWrapper(w);
    Delete(dirclassname);
    return Language::classDirectorEnd(n);
  }

  virtual int classDirectorDisown(Node *n) {
    (void)n;
    return SWIG_OK;
  }

  virtual bool extraDirectorProtectedCPPMethodsRequired() const {
    return false;
  }

private:
  void buildRustProxyParameters(Node *n, ParmList *parms, String *rust_params, String *rust_pre_code, String *rust_args, int skip_inputs = 0,
                                bool variable_setter = false) {
    Swig_typemap_attach_parms("rusttype", parms, 0);
    Swig_typemap_attach_parms("rustin", parms, 0);
    int gencomma = 0;
    int index = 0;
    int skipped = 0;
    for (Parm *p = parms; p; p = nextSibling(p), index++) {
      if (checkAttribute(p, "varargs:ignore", "1"))
        continue;
      if (checkAttribute(p, "tmap:in:numinputs", "0"))
        continue;

      SwigType *pt = Getattr(p, "type");
      if (SwigType_type(pt) == T_VOID)
        continue;
      if (skipped < skip_inputs) {
        skipped++;
        continue;
      }

      String *arg = variable_setter && !gencomma ? NewString("value") : makeParameterName(n, p, index, false);
      String *rust_type = Getattr(p, "tmap:rusttype");
      String *rust_arg = Getattr(p, "tmap:rustin") ? Copy(Getattr(p, "tmap:rustin")) : Copy(arg);
      String *pre = Getattr(p, "tmap:rustin:pre");
      bool rust_type_owned = false;

      if (!rust_type) {
        rust_type = rustType(pt);
        rust_type_owned = true;
      } else if (Getattr(p, "tmap:rusttype:out")) {
        rust_type = Getattr(p, "tmap:rusttype:out");
      }
      applyRustProxyInput(pt, arg, &rust_type, &rust_arg, &rust_type_owned);

      Replaceall(rust_arg, "$rustinput", arg);
      Replaceall(rust_arg, "$iminput", arg);
      if (pre) {
        String *pre_code = Copy(pre);
        Replaceall(pre_code, "$rustinput", arg);
        Replaceall(pre_code, "$iminput", arg);
        Printf(rust_pre_code, "    %s\n", pre_code);
        Delete(pre_code);
      }

      Printv(rust_params, gencomma ? ", " : "", arg, ": ", rust_type, NIL);
      Printv(rust_args, gencomma ? ", " : "", rust_arg, NIL);
      gencomma = 1;

      if (rust_type_owned)
        Delete(rust_type);
      Delete(rust_arg);
      Delete(arg);
    }
  }

  String *rustProxyMethodName(Node *n) {
    String *method_name = rustIdentifier(Getattr(n, "sym:name"));
    if (Getattr(n, "sym:overloaded"))
      Printf(method_name, "_%s", Getattr(n, "sym:overname"));
    return method_name;
  }

  String *rustConstructorName(Node *n) {
    String *constructor_name = NewString("new");
    if (Getattr(n, "sym:overloaded") && ParmList_len(Getattr(n, "parms")) > 0) {
      String *overname = Getattr(n, "sym:overname");
      if (overname && Strncmp(overname, "__SWIG_", 7) == 0)
        Printf(constructor_name, "_%s", Char(overname) + 7);
      else
        Printf(constructor_name, "_%s", overname);
    }
    return constructor_name;
  }

  bool rustSignatureNeedsUnsafe(String *rust_params, String *rust_return) const {
    return (rust_params && (Strstr(rust_params, "*mut ") || Strstr(rust_params, "*const "))) ||
           (rust_return && (Strstr(rust_return, "*mut ") || Strstr(rust_return, "*const ")));
  }

  void emitProxyFunction(Node *n, bool is_static, bool result_mode = false) {
    if (!proxy_class_code || Getattr(n, "overload:ignore") || GetFlag(n, "explicitcall"))
      return;

    SwigType *returntype = Getattr(n, "type");
    ParmList *parms = Getattr(n, "parms");
    String *overloaded_name = getOverloadedName(n);
    String *wrapper_name = Swig_name_member(getNSpace(), getClassPrefix(), overloaded_name);
    String *method_name = rustProxyMethodName(n);
    String *rust_return = rustReturnType(n, returntype);
    String *rust_params = NewString("");
    String *rust_pre_code = NewString("");
    String *rust_args = NewString("");
    String *rust_call = NewString("");
    String *rust_body = NewString("");

    buildRustProxyParameters(n, parms, rust_params, rust_pre_code, rust_args);

    bool needs_unsafe = rustSignatureNeedsUnsafe(rust_params, rust_return);
    String *public_method_name = result_mode ? rustTryIdentifier(method_name) : Copy(method_name);
    Printf(proxy_class_code, "  pub %sfn %s(", needs_unsafe ? "unsafe " : "", public_method_name);
    if (!is_static)
      Printf(proxy_class_code, "&self");
    if (Len(rust_params)) {
      if (!is_static)
        Printf(proxy_class_code, ", ");
      Printv(proxy_class_code, rust_params, NIL);
    }
    Printf(proxy_class_code, ")");
    if (result_mode) {
      String *result_type = rustResultType(rust_return);
      Printf(proxy_class_code, " -> %s", result_type);
      Delete(result_type);
    } else if (Cmp(rust_return, "()") != 0) {
      Printf(proxy_class_code, " -> %s", rust_return);
    }
    Printf(proxy_class_code, " {\n");
    if (!needs_unsafe)
      Printf(proxy_class_code, "    unsafe {\n");
    Printv(proxy_class_code, rust_pre_code, NIL);
    Printf(rust_call, "%s_raw(", wrapper_name);
    if (!is_static)
      Printf(rust_call, "self.as_ptr()");
    if (Len(rust_args)) {
      if (!is_static)
        Printf(rust_call, ", ");
      Printv(rust_call, rust_args, NIL);
    }
    Printf(rust_call, ")");
    rust_body = rustOutCode(n, rust_call, needs_unsafe ? "    " : "      ", result_mode);
    Printv(proxy_class_code, rust_body, NIL);
    if (!needs_unsafe)
      Printf(proxy_class_code, "    }\n");
    Printf(proxy_class_code, "  }\n");

    Delete(rust_body);
    Delete(rust_call);
    Delete(rust_args);
    Delete(rust_pre_code);
    Delete(rust_params);
    Delete(rust_return);
    Delete(public_method_name);
    Delete(method_name);
    Delete(wrapper_name);
    Delete(overloaded_name);
  }

  void emitProxyConstructor(Node *n, bool result_mode = false) {
    if (!proxy_class_code || Getattr(n, "overload:ignore"))
      return;

    ParmList *parms = Getattr(n, "parms");
    String *overloaded_name = getOverloadedName(n);
    String *wrapper_name = Swig_name_construct(getNSpace(), overloaded_name);
    String *constructor_name = rustConstructorName(n);
    String *rust_params = NewString("");
    String *rust_pre_code = NewString("");
    String *rust_args = NewString("");

    buildRustProxyParameters(n, parms, rust_params, rust_pre_code, rust_args);

    bool needs_unsafe = rustSignatureNeedsUnsafe(rust_params, 0);
    const char *rust_return = result_mode ? "Result<Self, RustException>" : "Self";
    Printf(
      proxy_class_code, "  pub %sfn %s%s(%s) -> %s {\n", needs_unsafe ? "unsafe " : "", result_mode ? "try_" : "", constructor_name, rust_params, rust_return);
    if (!needs_unsafe)
      Printf(proxy_class_code, "    unsafe {\n");
    Printv(proxy_class_code, rust_pre_code, NIL);
    Printf(proxy_class_code, "%slet swig_result = %s_raw(%s);\n", needs_unsafe ? "    " : "      ", wrapper_name, rust_args);
    if (result_mode) {
      Printf(proxy_class_code, "%sif let Some(error) = rust_take_exception() {\n", needs_unsafe ? "    " : "      ");
      Printf(proxy_class_code, "%s  return Err(error);\n", needs_unsafe ? "    " : "      ");
      Printf(proxy_class_code, "%s}\n", needs_unsafe ? "    " : "      ");
      Printf(proxy_class_code, "%sOk(Self::from_raw_owned(swig_result, true))\n", needs_unsafe ? "    " : "      ");
    } else {
      Printf(proxy_class_code, "%srust_check_exception();\n", needs_unsafe ? "    " : "      ");
      Printf(proxy_class_code, "%sSelf::from_raw_owned(swig_result, true)\n", needs_unsafe ? "    " : "      ");
    }
    if (!needs_unsafe)
      Printf(proxy_class_code, "    }\n");
    Printf(proxy_class_code, "  }\n");

    Delete(rust_args);
    Delete(rust_pre_code);
    Delete(rust_params);
    Delete(constructor_name);
    Delete(wrapper_name);
    Delete(overloaded_name);
  }

  String *rustSetterName(String *name) {
    String *setter = NewStringf("set_%s", name);
    String *rust_name = rustIdentifier(setter);
    Delete(setter);
    return rust_name;
  }

  String *rustGetterName(String *name) {
    String *getter = NewStringf("get_%s", name);
    String *rust_name = rustIdentifier(getter);
    Delete(getter);
    return rust_name;
  }

  void emitRustVariableAccessor(Node *n, String *wrapper_name) {
    if (!variable_name || (!global_variable_flag && !member_variable_flag && !static_member_variable_flag))
      return;

    const bool setter = Getattr(n, "varset") || Getattr(n, "memberset");
    const bool getter = Getattr(n, "varget") || Getattr(n, "memberget");
    if (!setter && !getter)
      return;

    String *rust_name = NULL;
    if (setter)
      rust_name = rustSetterName(variable_name);
    else if (global_variable_flag)
      rust_name = rustGetterName(variable_name);
    else
      rust_name = rustIdentifier(variable_name);
    String *rust_return = rustReturnType(n, Getattr(n, "type"));
    String *rust_params = NewString("");
    String *rust_pre_code = NewString("");
    String *rust_args = NewString("");
    String *raw_args = NewString("");
    String *call = NewString("");
    String *target = (member_variable_flag || static_member_variable_flag) && proxy_class_code ? proxy_class_code : rust_proxy_code;

    if (setter)
      buildRustProxyParameters(n, Getattr(n, "parms"), rust_params, rust_pre_code, rust_args, member_variable_flag ? 1 : 0, true);

    if (member_variable_flag) {
      Printf(raw_args, "self.as_ptr()");
      if (Len(rust_args))
        Printf(raw_args, ", %s", rust_args);
    } else {
      Printv(raw_args, rust_args, NIL);
    }

    Printf(call, "%s_raw(%s)", wrapper_name, raw_args);

    bool needs_unsafe = rustSignatureNeedsUnsafe(rust_params, rust_return);
    if (target == proxy_class_code) {
      Printf(target, "  pub %sfn %s(", needs_unsafe ? "unsafe " : "", rust_name);
      if (member_variable_flag)
        Printf(target, "&self");
      if (Len(rust_params)) {
        if (member_variable_flag)
          Printf(target, ", ");
        Printv(target, rust_params, NIL);
      }
      Printf(target, ")");
    } else {
      Printf(target, "pub %sfn %s(", needs_unsafe ? "unsafe " : "", rust_name);
      Printv(target, rust_params, NIL);
      Printf(target, ")");
    }
    if (Cmp(rust_return, "()") != 0)
      Printf(target, " -> %s", rust_return);
    Printf(target, " {\n");
    if (!needs_unsafe)
      Printf(target, "%sunsafe {\n", target == proxy_class_code ? "    " : "  ");
    Printv(target, rust_pre_code, NIL);
    String *body = rustOutCode(n, call, needs_unsafe ? (target == proxy_class_code ? "    " : "  ") : (target == proxy_class_code ? "      " : "    "));
    Printv(target, body, NIL);
    if (!needs_unsafe)
      Printf(target, "%s}\n", target == proxy_class_code ? "    " : "  ");
    Printf(target, "%s}\n", target == proxy_class_code ? "  " : "");
    if (target == rust_proxy_code)
      Printf(target, "\n");

    Delete(body);
    Delete(call);
    Delete(raw_args);
    Delete(rust_args);
    Delete(rust_pre_code);
    Delete(rust_params);
    Delete(rust_return);
    Delete(rust_name);
  }

  String *getOverloadedName(Node *n) const {
    String *overloaded_name = Copy(Getattr(n, "sym:name"));
    if (Getattr(n, "sym:overloaded"))
      Append(overloaded_name, Getattr(n, "sym:overname"));
    return overloaded_name;
  }

  UpcallData *addUpcallMethod(String *rust_method, String *class_method, String *decl, String *overloaded_name) {
    String *key = NewStringf("%s|%s", class_method, decl);

    curr_class_dmethod++;
    UpcallData *udata = NewHash();
    String *methid = NewStringf("%d", n_dmethods - first_class_dmethod);
    n_dmethods++;
    Append(dmethods_seq, udata);
    Setattr(dmethods_table, key, udata);
    Setattr(udata, "method", Copy(class_method));
    Setattr(udata, "rust_method", Copy(rust_method));
    Setattr(udata, "class_methodidx", methid);
    Setattr(udata, "overname", Copy(overloaded_name));
    Setattr(udata, "decl", Copy(decl));
    Delete(methid);
    Delete(key);
    return udata;
  }

  void directorDeclaration(Node *n) {
    String *base = Getattr(n, "classtype");
    String *class_ctor = NewString("Swig::Director()");
    String *dirclassname = directorClassName(n);
    String *declaration = Swig_class_declaration(n, dirclassname);

    Printf(declaration, " : public %s, public Swig::Director", base);
    Setattr(n, "director:decl", declaration);
    Setattr(n, "director:ctor", class_ctor);

    Delete(dirclassname);
  }

  String *cAbiType(SwigType *t) {
    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    String *result = NULL;

    if (SwigType_isreference(stripped)) {
      SwigType *base = SwigType_base(stripped);
      SwigType_add_pointer(base);
      result = SwigType_str(base, 0);
      Delete(base);
    } else {
      result = SwigType_str(stripped, 0);
    }

    Delete(stripped);
    Delete(resolved);
    return result;
  }

  void emitDirectorInput(String *code, SwigType *t, String *name) {
    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    if (SwigType_isreference(stripped))
      Printf(code, "&%s", name);
    else
      Printf(code, "%s", name);
    Delete(stripped);
    Delete(resolved);
  }

  void emitDirectorOutput(String *code, SwigType *t, const_String_or_char_ptr name) {
    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    if (SwigType_isreference(stripped))
      Printf(code, "*%s", name);
    else
      Printf(code, "%s", name);
    Delete(stripped);
    Delete(resolved);
  }

  void emitDirectorConnect(Node *n) {
    if (!Swig_directorclass(n))
      return;

    String *norm_name = SwigType_namestr(Getattr(n, "name"));
    String *dirclassname = directorClassName(n);
    String *swig_director_connect = Swig_name_member(getNSpace(), getClassPrefix(), "director_connect");
    String *swig_director_disconnect = Swig_name_member(getNSpace(), getClassPrefix(), "director_disconnect");
    String *wname = Swig_name_wrapper(swig_director_connect);
    String *disconnect_wname = Swig_name_wrapper(swig_director_disconnect);
    String *sym_name = Getattr(n, "sym:name");
    Wrapper *code_wrap = NewWrapper();
    Wrapper *disconnect_wrap = NewWrapper();

    Printf(code_wrap->def, "SWIGEXPORT void %s(void *objarg, void *rustobj", wname);
    Printf(code_wrap->code, "  %s *obj = (%s *)objarg;\n", norm_name, norm_name);
    Printf(code_wrap->code, "  %s *director = static_cast<%s *>(obj);\n", dirclassname, dirclassname);
    Printf(code_wrap->code, "  director->swig_connect_director(rustobj");

    Printf(rust_extern_code, "  #[link_name = \"%s\"]\n", wname);
    Printf(rust_extern_code, "  pub fn %s_raw(obj: *mut c_void, rustobj: *mut c_void", swig_director_connect);
    String *rust_connect_params = NewString("");
    String *rust_connect_args = NewString("");
    String *rust_trait_args = NewString("");

    for (int i = first_class_dmethod; i < curr_class_dmethod; ++i) {
      UpcallData *udata = Getitem(dmethods_seq, i);
      String *methid = Getattr(udata, "class_methodidx");
      String *rust_callback_shim = Getattr(udata, "rust_callback_shim");

      Printf(code_wrap->def, ", %s::SWIG_Callback%s_t callback%s", dirclassname, methid, methid);
      Printf(code_wrap->code, ", callback%s", methid);
      Printf(rust_extern_code, ", callback%s: %sCallback%s", methid, sym_name, methid);
      if (Len(rust_connect_params)) {
        Printf(rust_connect_params, ", ");
        Printf(rust_connect_args, ", ");
        Printf(rust_trait_args, ", ");
      }
      Printf(rust_connect_params, "callback%s: %sCallback%s", methid, sym_name, methid);
      Printf(rust_connect_args, "callback%s", methid);
      Printf(rust_trait_args, "%s::<T>", rust_callback_shim);
    }

    Printf(code_wrap->def, ") {\n");
    Printf(code_wrap->code, ");\n");
    Printf(code_wrap->code, "}\n");
    Printf(rust_extern_code, ");\n");
    Printf(disconnect_wrap->def, "SWIGEXPORT void %s(void *objarg) {\n", disconnect_wname);
    Printf(disconnect_wrap->code, "  %s *obj = (%s *)objarg;\n", norm_name, norm_name);
    Printf(disconnect_wrap->code, "  %s *director = static_cast<%s *>(obj);\n", dirclassname, dirclassname);
    Printf(disconnect_wrap->code, "  director->swig_connect_director(0");
    for (int i = first_class_dmethod; i < curr_class_dmethod; ++i)
      Printf(disconnect_wrap->code, ", 0");
    Printf(disconnect_wrap->code, ");\n");
    Printf(disconnect_wrap->code, "}\n");
    Printf(rust_extern_code, "  #[link_name = \"%s\"]\n", disconnect_wname);
    Printf(rust_extern_code, "  pub fn %s_raw(obj: *mut c_void);\n", swig_director_disconnect);

    Printf(rust_proxy_code, "impl %s {\n", sym_name);
    Printf(rust_proxy_code, "  unsafe fn connect_director(&self, rustobj: *mut c_void");
    if (Len(rust_connect_params))
      Printf(rust_proxy_code, ", %s", rust_connect_params);
    Printf(rust_proxy_code, ") {\n");
    Printf(rust_proxy_code, "    %s_raw(self.as_ptr(), rustobj", swig_director_connect);
    if (Len(rust_connect_args))
      Printf(rust_proxy_code, ", %s", rust_connect_args);
    Printf(rust_proxy_code, ");\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  unsafe fn connect_director_trait<T: %sDirector>(&self, director: &mut T) {\n", sym_name);
    Printf(rust_proxy_code, "    self.connect_director(director as *mut T as *mut c_void");
    if (Len(rust_trait_args))
      Printf(rust_proxy_code, ", %s", rust_trait_args);
    Printf(rust_proxy_code, ");\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  fn disconnect_director(&self) {\n");
    Printf(rust_proxy_code, "    unsafe {\n");
    Printf(rust_proxy_code, "    %s_raw(self.as_ptr());\n", swig_director_disconnect);
    Printf(rust_proxy_code, "    }\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "}\n\n");
    Printf(rust_proxy_code, "pub struct %sDirectorHandle<T: %sDirector> {\n", sym_name, sym_name);
    Printf(rust_proxy_code, "  object: *mut c_void,\n");
    Printf(rust_proxy_code, "  director: Option<Box<T>>,\n");
    Printf(rust_proxy_code, "  connected: bool,\n");
    Printf(rust_proxy_code, "}\n");
    Printf(rust_proxy_code, "impl<T: %sDirector> %sDirectorHandle<T> {\n", sym_name, sym_name);
    Printf(rust_proxy_code, "  pub fn connect(object: &%s, director: T) -> Self {\n", sym_name);
    Printf(rust_proxy_code, "    let mut handle = Self { object: object.as_ptr(), director: Some(Box::new(director)), connected: false };\n");
    Printf(rust_proxy_code, "    unsafe {\n");
    Printf(rust_proxy_code, "      object.connect_director_trait(&mut **handle.director.as_mut().expect(\"director handle has no director\"));\n");
    Printf(rust_proxy_code, "    }\n");
    Printf(rust_proxy_code, "    handle.connected = true;\n");
    Printf(rust_proxy_code, "    handle\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub fn is_connected(&self) -> bool {\n");
    Printf(rust_proxy_code, "    self.connected\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub fn disconnect(&mut self) {\n");
    Printf(rust_proxy_code, "    if self.connected && !self.object.is_null() {\n");
    Printf(rust_proxy_code, "      unsafe { %s_raw(self.object); }\n", swig_director_disconnect);
    Printf(rust_proxy_code, "      self.connected = false;\n");
    Printf(rust_proxy_code, "    }\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub fn into_director(mut self) -> T {\n");
    Printf(rust_proxy_code, "    self.disconnect();\n");
    Printf(rust_proxy_code, "    let director = *self.director.take().expect(\"director handle has no director\");\n");
    Printf(rust_proxy_code, "    std::mem::forget(self);\n");
    Printf(rust_proxy_code, "    director\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub unsafe fn disown(mut self) -> *mut T {\n");
    Printf(rust_proxy_code, "    self.connected = false;\n");
    Printf(rust_proxy_code, "    let director = Box::into_raw(self.director.take().expect(\"director handle has no director\"));\n");
    Printf(rust_proxy_code, "    std::mem::forget(self);\n");
    Printf(rust_proxy_code, "    director\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub unsafe fn reclaim_disowned(object: &%s, director: *mut T) -> T {\n", sym_name);
    Printf(rust_proxy_code, "    %s_raw(object.as_ptr());\n", swig_director_disconnect);
    Printf(rust_proxy_code, "    *Box::from_raw(director)\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub fn director(&self) -> &T {\n");
    Printf(rust_proxy_code, "    self.director.as_ref().expect(\"director handle has no director\")\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "  pub fn director_mut(&mut self) -> &mut T {\n");
    Printf(rust_proxy_code, "    self.director.as_mut().expect(\"director handle has no director\")\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "}\n");
    Printf(rust_proxy_code, "impl<T: %sDirector> Drop for %sDirectorHandle<T> {\n", sym_name, sym_name);
    Printf(rust_proxy_code, "  fn drop(&mut self) {\n");
    Printf(rust_proxy_code, "    self.disconnect();\n");
    Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "}\n\n");

    Wrapper_print(code_wrap, f_wrappers);
    Wrapper_print(disconnect_wrap, f_wrappers);
    DelWrapper(code_wrap);
    DelWrapper(disconnect_wrap);

    Delete(rust_connect_params);
    Delete(rust_connect_args);
    Delete(rust_trait_args);
    Delete(norm_name);
    Delete(dirclassname);
    Delete(swig_director_connect);
    Delete(swig_director_disconnect);
    Delete(wname);
    Delete(disconnect_wname);
  }

  String *rustReturnType(Node *n, SwigType *returntype) {
    String *tm = Swig_typemap_lookup("rusttype", n, "", 0);
    if (tm) {
      String *rusttypeout = Getattr(n, "tmap:rusttype:out");
      String *rust_return = Copy(rusttypeout ? rusttypeout : tm);
      String *proxy_return = rustProxyReturnType(returntype);
      if (proxy_return && rustTypeIsVoidPointer(rust_return)) {
        Delete(rust_return);
        rust_return = proxy_return;
      } else {
        Delete(proxy_return);
      }
      return rust_return;
    }
    String *proxy_return = rustProxyReturnType(returntype);
    if (proxy_return)
      return proxy_return;
    return rustType(returntype);
  }

  String *rustResultType(String *rust_return) const {
    if (Cmp(rust_return, "()") == 0)
      return NewString("Result<(), RustException>");
    return NewStringf("Result<%s, RustException>", rust_return);
  }

  void rustEmitResultExceptionCheck(String *body, const_String_or_char_ptr indent) {
    Printf(body, "%sif let Some(error) = rust_take_exception() {\n", indent);
    Printf(body, "%s  return Err(error);\n", indent);
    Printf(body, "%s}\n", indent);
  }

  String *rustOutCode(Node *n, String *imcall, const_String_or_char_ptr indent, bool result_mode = false) {
    SwigType *returntype = Getattr(n, "type");
    String *rust_return = rustReturnType(n, returntype);
    String *body = NewString("");

    String *proxy_body = rustProxyOutCode(n, returntype, imcall, indent, result_mode);
    if (proxy_body) {
      Delete(rust_return);
      return proxy_body;
    }

    String *tm = Swig_typemap_lookup("rustout", n, "", 0);
    if (tm) {
      String *converted = Copy(tm);
      Replaceall(converted, "$imcall", "swig_result");
      Replaceall(converted, "$rustcall", "swig_result");
      Replaceall(converted, "\n", NewStringf("\n%s", indent));
      Printf(body, "%slet swig_result = %s;\n", indent, imcall);
      if (result_mode) {
        rustEmitResultExceptionCheck(body, indent);
        Printf(body, "%sOk({\n", indent);
        Printv(body, indent, "  ", converted, "\n", NIL);
        Printf(body, "%s})\n", indent);
      } else {
        Printf(body, "%srust_check_exception();\n", indent);
        Printv(body, indent, converted, "\n", NIL);
      }
      Delete(converted);
      Delete(rust_return);
      return body;
    }

    if (Cmp(rust_return, "()") != 0) {
      Printf(body, "%slet swig_result = %s;\n", indent, imcall);
      if (result_mode) {
        rustEmitResultExceptionCheck(body, indent);
        Printf(body, "%sOk(swig_result)\n", indent);
      } else {
        Printf(body, "%srust_check_exception();\n", indent);
        Printf(body, "%sswig_result\n", indent);
      }
    } else {
      Printf(body, "%s%s;\n", indent, imcall);
      if (result_mode) {
        rustEmitResultExceptionCheck(body, indent);
        Printf(body, "%sOk(())\n", indent);
      } else {
        Printf(body, "%srust_check_exception();\n", indent);
      }
    }
    Delete(rust_return);
    return body;
  }

  bool rustTypeIsVoidPointer(String *rust_type) const {
    return Cmp(rust_type, "*mut c_void") == 0 || Cmp(rust_type, "*const c_void") == 0;
  }

  String *rustProxyNameForType(SwigType *t, bool *is_pointer, bool *is_reference) {
    *is_pointer = false;
    *is_reference = false;

    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    SwigType *proxy_type = NULL;

    if (SwigType_ispointer(stripped)) {
      SwigType *base = SwigType_base(stripped);
      SwigType *base_stripped = SwigType_strip_qualifiers(base);
      if (!SwigType_ispointer(base_stripped) && !SwigType_isarray(base_stripped) && !SwigType_isreference(base_stripped)) {
        proxy_type = Copy(base_stripped);
        *is_pointer = true;
      }
      Delete(base_stripped);
      Delete(base);
    } else if (SwigType_isreference(stripped)) {
      SwigType *base = SwigType_base(stripped);
      SwigType *base_stripped = SwigType_strip_qualifiers(base);
      if (!SwigType_ispointer(base_stripped) && !SwigType_isarray(base_stripped) && !SwigType_isreference(base_stripped)) {
        proxy_type = Copy(base_stripped);
        *is_reference = true;
      }
      Delete(base_stripped);
      Delete(base);
    } else if (!SwigType_isarray(stripped)) {
      proxy_type = Copy(stripped);
    }

    String *proxy_name = NULL;
    if (proxy_type) {
      Node *proxy_class = classLookup(proxy_type);
      if (proxy_class)
        proxy_name = Copy(Getattr(proxy_class, "sym:name"));
      Delete(proxy_type);
    }

    Delete(stripped);
    Delete(resolved);
    return proxy_name;
  }

  void applyRustProxyInput(SwigType *pt, String *arg, String **rust_type, String **rust_arg, bool *rust_type_owned) {
    if (!rustTypeIsVoidPointer(*rust_type))
      return;

    bool is_pointer = false;
    bool is_reference = false;
    String *proxy_name = rustProxyNameForType(pt, &is_pointer, &is_reference);
    if (!proxy_name)
      return;

    if (is_pointer || is_reference) {
      if (*rust_type_owned)
        Delete(*rust_type);
      Delete(*rust_arg);
      *rust_type = NewStringf("&%s", proxy_name);
      *rust_arg = NewStringf("%s.as_ptr()", arg);
      *rust_type_owned = true;
    }
    Delete(proxy_name);
  }

  String *rustProxyReturnType(SwigType *returntype) {
    bool is_pointer = false;
    bool is_reference = false;
    String *proxy_name = rustProxyNameForType(returntype, &is_pointer, &is_reference);
    if (!proxy_name)
      return NULL;

    String *rust_return = NULL;
    if (is_pointer)
      rust_return = NewStringf("Option<%s>", proxy_name);
    else if (is_reference)
      rust_return = Copy(proxy_name);
    else
      rust_return = Copy(proxy_name);

    Delete(proxy_name);
    return rust_return;
  }

  String *rustProxyOutCode(Node *n, SwigType *returntype, String *imcall, const_String_or_char_ptr indent, bool result_mode = false) {
    bool is_pointer = false;
    bool is_reference = false;
    String *proxy_name = rustProxyNameForType(returntype, &is_pointer, &is_reference);
    if (!proxy_name)
      return NULL;

    String *body = NewString("");
    Printf(body, "%slet swig_result = %s;\n", indent, imcall);
    if (result_mode)
      rustEmitResultExceptionCheck(body, indent);
    else
      Printf(body, "%srust_check_exception();\n", indent);
    if (result_mode)
      Printf(body, "%sOk(", indent);
    if (is_pointer) {
      Printf(body, "%sif swig_result.is_null() {\n", result_mode ? "" : indent);
      Printf(body, "%s  None\n", indent);
      Printf(body, "%s} else {\n", indent);
      Printf(body, "%s  Some(%s::from_raw_owned(swig_result, %s))\n", indent, proxy_name, GetFlag(n, "feature:new") ? "true" : "false");
      Printf(body, "%s}", indent);
    } else if (is_reference) {
      Printf(body, "%s%s::from_raw_owned(swig_result, false)", result_mode ? "" : indent, proxy_name);
    } else {
      Printf(body, "%s%s::from_raw_owned(swig_result, true)", result_mode ? "" : indent, proxy_name);
    }
    if (result_mode)
      Printf(body, ")\n");
    else
      Printf(body, "\n");

    Delete(proxy_name);
    return body;
  }

  void writeRustFunction(Node *n, String *rust_name, String *c_name, String *raw_return, String *rust_im_params, String *rust_params, String *rust_pre_code,
                         String *rust_args) {
    String *public_rust_name = rustIdentifier(rust_name);
    String *public_rust_return = rustReturnType(n, Getattr(n, "type"));

    Printf(rust_extern_code, "  #[link_name = \"%s\"]\n", c_name);
    Printf(rust_extern_code, "  pub fn %s_raw(%s)", rust_name, rust_im_params);
    if (Cmp(raw_return, "()") != 0)
      Printf(rust_extern_code, " -> %s", raw_return);
    Printf(rust_extern_code, ";\n");

    bool needs_unsafe = rustSignatureNeedsUnsafe(rust_params, public_rust_return);
    Printf(rust_proxy_code, "pub %sfn %s(%s)", needs_unsafe ? "unsafe " : "", public_rust_name, rust_params);
    if (Cmp(public_rust_return, "()") != 0)
      Printf(rust_proxy_code, " -> %s", public_rust_return);
    Printf(rust_proxy_code, " {\n");
    if (!needs_unsafe)
      Printf(rust_proxy_code, "  unsafe {\n");
    Printv(rust_proxy_code, rust_pre_code, NIL);
    String *imcall = NewStringf("%s_raw(%s)", rust_name, rust_args);
    String *body = rustOutCode(n, imcall, needs_unsafe ? "  " : "    ");
    Printv(rust_proxy_code, body, NIL);
    if (!needs_unsafe)
      Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "}\n\n");
    Delete(body);

    String *result_type = rustResultType(public_rust_return);
    String *try_public_rust_name = rustTryIdentifier(public_rust_name);
    Printf(rust_proxy_code, "pub %sfn %s(%s) -> %s {\n", needs_unsafe ? "unsafe " : "", try_public_rust_name, rust_params, result_type);
    if (!needs_unsafe)
      Printf(rust_proxy_code, "  unsafe {\n");
    Printv(rust_proxy_code, rust_pre_code, NIL);
    body = rustOutCode(n, imcall, needs_unsafe ? "  " : "    ", true);
    Printv(rust_proxy_code, body, NIL);
    if (!needs_unsafe)
      Printf(rust_proxy_code, "  }\n");
    Printf(rust_proxy_code, "}\n\n");
    Delete(body);
    Delete(result_type);
    Delete(try_public_rust_name);
    Delete(imcall);
    Delete(public_rust_return);
    Delete(public_rust_name);
  }

  bool isRustKeyword(const String *name) const {
    static const char *keywords[] = {"Self",   "abstract", "as",    "async",  "await", "become", "box",    "break",  "const", "continue", "crate", "do",
                                     "dyn",    "else",     "enum",  "extern", "false", "final",  "fn",     "for",    "if",    "impl",     "in",    "let",
                                     "loop",   "macro",    "match", "mod",    "move",  "mut",    "priv",   "pub",    "ref",   "return",   "self",  "static",
                                     "struct", "super",    "trait", "true",   "try",   "type",   "typeof", "unsafe", "use",   "where",    "while", "yield"};

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
      if (Cmp(name, keywords[i]) == 0)
        return true;
    }
    return false;
  }

  String *rustIdentifier(const String *name) const {
    if (!name)
      return NewString("");
    if (isRustKeyword(name))
      return NewStringf("r#%s", name);
    return Copy(name);
  }

  String *rustAssociatedConstantName(const String *name) const {
    String *rust_name = rustIdentifier(name);
    String *constant_name = NewString("");
    const char *value = Char(rust_name);
    for (const char *c = value; c && *c; ++c) {
      char ch = *c;
      if (ch >= 'a' && ch <= 'z')
        ch = ch - 'a' + 'A';
      Printf(constant_name, "%c", ch);
    }
    Delete(rust_name);
    return constant_name;
  }

  String *rustTryIdentifier(const String *name) const {
    if (Strncmp(name, "r#", 2) == 0)
      return NewStringf("r#try_%s", Char(name) + 2);
    return NewStringf("try_%s", name);
  }

  bool rustTypeIsPrimitive(SwigType *t) {
    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    bool primitive = Cmp(stripped, "void") == 0 || Cmp(stripped, "bool") == 0 || Cmp(stripped, "char") == 0 || Cmp(stripped, "signed char") == 0 ||
                     Cmp(stripped, "unsigned char") == 0 || Cmp(stripped, "short") == 0 || Cmp(stripped, "unsigned short") == 0 || Cmp(stripped, "int") == 0 ||
                     Cmp(stripped, "unsigned int") == 0 || Cmp(stripped, "long") == 0 || Cmp(stripped, "unsigned long") == 0 ||
                     Cmp(stripped, "long long") == 0 || Cmp(stripped, "unsigned long long") == 0 || Cmp(stripped, "float") == 0 || Cmp(stripped, "double") == 0;
    Delete(stripped);
    Delete(resolved);
    return primitive;
  }

  String *rustType(SwigType *t) {
    SwigType *resolved = SwigType_typedef_resolve_all(t);
    SwigType *stripped = SwigType_strip_qualifiers(resolved);
    String *result = NULL;

    if (SwigType_ispointer(stripped) || SwigType_isarray(stripped)) {
      SwigType *base = SwigType_base(stripped);
      SwigType *base_stripped = SwigType_strip_qualifiers(base);
      if (Cmp(base_stripped, "char") == 0) {
        result = NewString("*mut c_char");
      } else if (Cmp(base_stripped, "void") == 0) {
        result = NewString("*mut c_void");
      } else {
        result = NewString("*mut c_void");
      }
      Delete(base_stripped);
      Delete(base);
    } else if (SwigType_isreference(stripped)) {
      result = NewString("*mut c_void");
    } else if (Cmp(stripped, "void") == 0) {
      result = NewString("()");
    } else if (Cmp(stripped, "bool") == 0) {
      result = NewString("bool");
    } else if (Cmp(stripped, "char") == 0) {
      result = NewString("c_char");
    } else if (Cmp(stripped, "signed char") == 0) {
      result = NewString("c_schar");
    } else if (Cmp(stripped, "unsigned char") == 0) {
      result = NewString("c_uchar");
    } else if (Cmp(stripped, "short") == 0) {
      result = NewString("c_short");
    } else if (Cmp(stripped, "unsigned short") == 0) {
      result = NewString("c_ushort");
    } else if (Cmp(stripped, "int") == 0) {
      result = NewString("c_int");
    } else if (Cmp(stripped, "unsigned int") == 0) {
      result = NewString("c_uint");
    } else if (Cmp(stripped, "long") == 0) {
      result = NewString("c_long");
    } else if (Cmp(stripped, "unsigned long") == 0) {
      result = NewString("c_ulong");
    } else if (Cmp(stripped, "long long") == 0) {
      result = NewString("c_longlong");
    } else if (Cmp(stripped, "unsigned long long") == 0) {
      result = NewString("c_ulonglong");
    } else if (Cmp(stripped, "float") == 0) {
      result = NewString("c_float");
    } else if (Cmp(stripped, "double") == 0) {
      result = NewString("c_double");
    } else {
      result = NewString("*mut c_void");
    }

    Delete(stripped);
    Delete(resolved);
    return result;
  }
};

/* -----------------------------------------------------------------------------
 * swig_rust()    - Instantiate module
 * ----------------------------------------------------------------------------- */

static Language *new_swig_rust() {
  return new RUST();
}

extern "C" Language *swig_rust(void) {
  return new_swig_rust();
}
