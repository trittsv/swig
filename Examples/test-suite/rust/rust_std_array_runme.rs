mod rust_std_array;

fn main() {
    unsafe {
        let values = rust_std_array::IntArray3::new___SWIG_0();
        values.fill(4);
        values.setitem(1, 9);

        if values.size() != 3 {
            panic!("expected size 3, got {}", values.size());
        }
        if values.getitemcopy(1) != 9 {
            panic!("expected updated array item");
        }
        if rust_std_array::rust_std_array_sum(values.as_ptr()) != 17 {
            panic!("expected array sum 17");
        }
    }
}
