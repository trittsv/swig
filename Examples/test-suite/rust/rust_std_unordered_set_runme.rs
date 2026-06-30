mod rust_std_unordered_set;

fn main() {
    let values = rust_std_unordered_set::IntUnorderedSet::new();
    values.insert_value(11);
    values.insert_value(13);

    if values.size() != 2 {
        panic!("expected unordered_set size 2, got {}", values.size());
    }
    if values.count(13) != 1 {
        panic!("expected unordered_set value 13");
    }
    if rust_std_unordered_set::rust_std_unordered_set_sum_values(&values) != 24 {
        panic!("expected unordered_set value sum 24");
    }
    values.erase(11);
    if values.count(11) != 0 {
        panic!("expected erased unordered_set value to be absent");
    }
}
