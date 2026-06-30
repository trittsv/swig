mod rust_std_except;

fn main() {
    if rust_std_except::rust_no_throw() != 7 {
      panic!("rust_no_throw failed");
    }

    let thrown = std::panic::catch_unwind(|| {
      rust_std_except::rust_throw_out_of_range();
    });
    if thrown.is_ok() {
      panic!("std::out_of_range was not translated into a Rust panic");
    }
}
