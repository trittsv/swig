mod rust_std_map;

fn main() {
    let values = rust_std_map::IntIntMap::new();
    values.setitem(2, 20);
    values.setitem(3, 30);

    if values.size() != 2 {
        panic!("expected map size 2, got {}", values.size());
    }
    if values.getitem(3) != 30 {
        panic!("expected map value 30");
    }
    unsafe {
        if rust_std_map::rust_std_map_sum_values(values.as_ptr()) != 50 {
            panic!("expected map value sum 50");
        }
    }
    values.erase(2);
    if values.count(2) != 0 {
        panic!("expected erased map key to be absent");
    }
}
