mod rust_enums;

fn main() {
    if rust_enums::rust_next_color(rust_enums::RustColor::RustRed as i32) != rust_enums::RustColor::RustGreen as i32 {
        panic!("expected RustRed to advance to RustGreen");
    }
    if rust_enums::rust_next_color(rust_enums::RustColor::RustGreen as i32) != rust_enums::RustColor::RustBlue as i32 {
        panic!("expected RustGreen to advance to RustBlue");
    }
}
