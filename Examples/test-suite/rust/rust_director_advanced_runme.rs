mod rust_director_advanced;

use rust_director_advanced::{AdvancedDirector, AdvancedDirectorHandle};
use std::os::raw::c_int;

struct MyAdvanced {
    total: c_int,
}

impl AdvancedDirector for MyAdvanced {
    unsafe fn transform(&mut self, arg0: c_int) -> c_int {
        self.total += arg0;
        arg0 * 2
    }

    unsafe fn overloaded__SWIG_0(&mut self, arg0: c_int) -> c_int {
        arg0 + 100
    }

    unsafe fn overloaded__SWIG_1(&mut self, arg0: c_int, arg1: c_int) -> c_int {
        arg0 * 10 + arg1
    }

    unsafe fn pure(&mut self, arg0: c_int) -> c_int {
        arg0 + 7
    }

    unsafe fn record(&mut self, arg0: c_int) {
        self.total += arg0;
    }
}

fn main() {
    let advanced = rust_director_advanced::Advanced::new();
    {
        let mut handle = unsafe { AdvancedDirectorHandle::connect(&advanced, MyAdvanced { total: 0 }) };

        if rust_director_advanced::AdvancedDirectorMethodTypes0 != ["c_int"] {
            panic!("unexpected director method metadata");
        }
        if advanced.call_transform(4) != 12 {
            panic!("expected rustdirectorin/rustdirectorout conversion for transform");
        }
        if handle.director().total != 5 {
            panic!("expected transformed input in Rust director");
        }
        if advanced.call_overloaded_one(2) != 105 {
            panic!("expected one-argument overload dispatch");
        }
        if advanced.call_overloaded_two(1, 2) != 25 {
            panic!("expected two-argument overload dispatch");
        }
        if advanced.call_pure(3) != 13 {
            panic!("expected pure virtual director dispatch");
        }

        advanced.call_record(10);
        if handle.director_mut().total != 16 {
            panic!("expected void director callback to update state");
        }
    }

    if advanced.call_transform(4) != 4 {
        panic!("expected handle drop to disconnect the director");
    }
}
