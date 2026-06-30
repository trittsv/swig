mod rust_std_list;

fn main() {
    let values = rust_std_list::IntList::new();
    values.push_back(3);
    values.push_front(2);
    values.push_back(5);

    if values.size() != 3 {
        panic!("expected size 3, got {}", values.size());
    }
    if rust_std_list::rust_std_list_sum(&values) != 10 {
        panic!("expected list sum 10");
    }
    values.reverse();
    values.remove(3);
    if values.size() != 2 {
        panic!("expected size 2 after remove");
    }
}
