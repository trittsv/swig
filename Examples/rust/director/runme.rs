mod messages;

use messages::ReceiverDirector;
use std::ffi::c_void;
use std::os::raw::c_int;

struct MyReceiver {
    total: c_int,
}

impl messages::ReceiverDirector for MyReceiver {
    unsafe fn message(&mut self, value: c_int) {
        self.total += value;
    }
}

extern "C" fn receiver_message(obj: *mut c_void, value: c_int) {
    unsafe {
        (*(obj as *mut MyReceiver)).message(value);
    }
}

fn main() {
    unsafe {
        let mut rust_receiver = MyReceiver { total: 0 };

        let receiver = messages::Receiver::new();
        receiver.connect_director(
            &mut rust_receiver as *mut MyReceiver as *mut c_void,
            receiver_message,
        );
        messages::deliver(receiver.as_ptr(), 42);
        if rust_receiver.total != 42 {
            panic!("expected 42, got {}", rust_receiver.total);
        }
    }
}
