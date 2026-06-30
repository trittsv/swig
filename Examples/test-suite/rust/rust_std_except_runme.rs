mod rust_std_except;

fn main() {
    if rust_std_except::rust_no_throw() != 7 {
        panic!("rust_no_throw failed");
    }

    let old_hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(|_| {}));
    let thrown = std::panic::catch_unwind(|| {
        rust_std_except::rust_throw_out_of_range();
    });
    std::panic::set_hook(old_hook);

    if thrown.is_ok() {
        panic!("std::out_of_range was not translated into a Rust panic");
    }
}
