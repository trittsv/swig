mod rust_std_set;

fn main() {
    unsafe {
        let values = rust_std_set::IntSet::new___SWIG_0();
        values.insert_value(7);
        values.insert_value(2);
        values.insert_value(7);

        if values.size() != 2 {
            panic!("expected unique set size 2, got {}", values.size());
        }
        if values.count(7) != 1 {
            panic!("expected set to contain 7");
        }
        if rust_std_set::rust_std_set_sum(values.as_ptr()) != 9 {
            panic!("expected set sum 9");
        }
        values.erase(2);
        if values.count(2) != 0 {
            panic!("expected erased value to be absent");
        }
    }
}
