mod rust_std_unordered_map;

fn main() {
    let values = rust_std_unordered_map::IntIntUnorderedMap::new();
    values.setitem(5, 50);
    values.setitem(7, 70);

    if values.size() != 2 {
        panic!("expected unordered_map size 2, got {}", values.size());
    }
    if values.getitem(7) != 70 {
        panic!("expected unordered_map value 70");
    }
    if rust_std_unordered_map::rust_std_unordered_map_sum_values(&values) != 120 {
        panic!("expected unordered_map value sum 120");
    }
    values.erase(5);
    if values.count(5) != 0 {
        panic!("expected erased unordered_map key to be absent");
    }
}
