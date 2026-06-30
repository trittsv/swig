mod rust_std_deque;

fn main() {
    let values = rust_std_deque::IntDeque::new();
    values.push_back(3);
    values.push_front(2);
    values.push_back(5);
    values.setitem(1, 7);

    if values.size() != 3 {
        panic!("expected size 3, got {}", values.size());
    }
    if values.getitemcopy(1) != 7 {
        panic!("expected updated deque item");
    }
    unsafe {
        if rust_std_deque::rust_std_deque_sum(values.as_ptr()) != 14 {
            panic!("expected deque sum 14");
        }
    }
}
