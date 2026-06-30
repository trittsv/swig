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
    let mut handle = MessageReceiverDirectorHandle::connect(&receiver, RustReceiver { total: 0 });

    let bus = rust_director_create_callback::MessageBus::new();
    bus.create(&receiver);

    if bus.dispatch(4) != 40 {
        panic!("expected dispatch through Rust director callback");
    }
    if bus.dispatch(3) != 70 {
        panic!("expected C++ to keep calling the Rust callback object");
    }
    if bus.current().is_none() {
        panic!("expected stored callback pointer");
    }
    if bus.current_ref().receive(2) != 90 {
        panic!("expected reference return to call the Rust callback object");
    }
    if handle.director_mut().total != 9 {
        panic!("expected Rust callback state to record messages");
    }

    handle.disconnect();
    if bus.dispatch(5) != 5 {
        panic!("expected disconnected director to use base callback");
    }

    let handle = MessageReceiverDirectorHandle::connect(&receiver, RustReceiver { total: 9 });
    if bus.dispatch(1) != 100 {
        panic!("expected reconnected director callback");
    }

    bus.clear();
    if bus.dispatch(5) != -1 {
        panic!("expected cleared callback slot");
    }
    if bus.current().is_some() {
        panic!("expected empty callback pointer");
    }

    let director = handle.into_director();
    if director.total != 10 {
        panic!("expected recovered Rust callback state");
    }

    let handle = MessageReceiverDirectorHandle::connect(&receiver, RustReceiver { total: 20 });
    bus.create(&receiver);
    let disowned = unsafe { handle.disown() };
    if bus.dispatch(2) != 220 {
        panic!("expected disowned director to stay connected");
    }
    let director = unsafe { MessageReceiverDirectorHandle::reclaim_disowned(&receiver, disowned) };
    if director.total != 22 {
        panic!("expected reclaimed disowned callback state");
    }
    if bus.dispatch(2) != 2 {
        panic!("expected reclaimed director to disconnect");
    }
}
