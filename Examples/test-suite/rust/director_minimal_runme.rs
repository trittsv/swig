mod director_minimal;

use director_minimal::MinimalDirector;
use std::ffi::c_void;

struct MyMinimal {
    calls: usize,
}

impl director_minimal::MinimalDirector for MyMinimal {
    unsafe fn run(&mut self) -> bool {
        self.calls += 1;
        true
    }
}

extern "C" fn minimal_run(obj: *mut c_void) -> bool {
    unsafe { (*(obj as *mut MyMinimal)).run() }
}

fn main() {
    unsafe {
        let mut rust_minimal = MyMinimal { calls: 0 };
        let minimal = director_minimal::Minimal::new();

        minimal.connect_director(
            &mut rust_minimal as *mut MyMinimal as *mut c_void,
            minimal_run,
        );
        if !minimal.get() {
            panic!("expected director override to return true");
        }
        if rust_minimal.calls != 1 {
            panic!("expected one director call, got {}", rust_minimal.calls);
        }
    }
}
