mod rust_ownership;

use rust_ownership::{LifetimeThing, RustOwnership};

fn main() {
    let borrowed = rust_ownership::maybe_lifetime(1).expect("expected borrowed pointer result");
    if !borrowed.is_borrowed() || borrowed.ownership() != RustOwnership::Borrowed {
        panic!("expected non-newobject pointer result to be borrowed");
    }
    if borrowed.value() != 100 {
        panic!("expected borrowed pointer method dispatch");
    }
    if rust_ownership::maybe_lifetime(0).is_some() {
        panic!("expected null pointer result to map to None");
    }

    let alive_at_start = LifetimeThing::alive();

    let owned = rust_ownership::make_owned_lifetime(7).expect("expected owned factory result");
    if !owned.is_owned() || owned.ownership() != RustOwnership::Owned {
        panic!("expected factory result to be owned");
    }
    if owned.value() != 7 {
        panic!("expected owned proxy method dispatch");
    }
    if rust_ownership::lifetime_value(&owned) != 7 {
        panic!("expected borrowed proxy argument");
    }

    let borrowed_ref = rust_ownership::lifetime_ref(&owned);
    if !borrowed_ref.is_borrowed() {
        panic!("expected reference return to be borrowed");
    }
    if borrowed_ref.value() != 7 {
        panic!("expected borrowed reference method dispatch");
    }

    let alias = owned.borrow_proxy();
    if !alias.is_borrowed() || alias.value() != 7 {
        panic!("expected explicit borrowed proxy alias");
    }

    let raw = owned.into_raw();
    let owned_again = unsafe { LifetimeThing::from_raw(raw) }.expect("expected non-null raw pointer");
    if !owned_again.is_owned() || owned_again.value() != 7 {
        panic!("expected raw ownership transfer back into an owned proxy");
    }
    drop(owned_again);

    if LifetimeThing::alive() != alive_at_start {
        panic!("expected owned proxy drop to free exactly one object");
    }

    if unsafe { LifetimeThing::from_raw(std::ptr::null_mut()) }.is_some() {
        panic!("expected null raw pointer construction to fail");
    }
}
