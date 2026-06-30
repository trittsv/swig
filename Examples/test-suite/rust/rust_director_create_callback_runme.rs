mod rust_director_create_callback;

use rust_director_create_callback::{MessageReceiverDirector, MessageReceiverDirectorHandle};
use std::os::raw::c_int;

struct RustReceiver {
    total: c_int,
}

impl MessageReceiverDirector for RustReceiver {
    unsafe fn receive(&mut self, arg0: c_int) -> c_int {
        self.total += arg0;
        self.total * 10
    }
}

fn main() {
    let receiver = rust_director_create_callback::MessageReceiver::new();
    let mut handle =
        unsafe { MessageReceiverDirectorHandle::connect(&receiver, RustReceiver { total: 0 }) };

    let bus = rust_director_create_callback::MessageBus::new();
    unsafe {
        bus.create(receiver.as_ptr());
    }

    if bus.dispatch(4) != 40 {
        panic!("expected dispatch through Rust director callback");
    }
    if bus.dispatch(3) != 70 {
        panic!("expected C++ to keep calling the Rust callback object");
    }
    if handle.director_mut().total != 7 {
        panic!("expected Rust callback state to record messages");
    }

    bus.clear();
    if bus.dispatch(5) != -1 {
        panic!("expected cleared callback slot");
    }
}
