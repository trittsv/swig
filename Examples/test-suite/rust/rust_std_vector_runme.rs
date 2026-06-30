mod rust_std_vector;

fn main() {
    let values = rust_std_vector::IntVector::new();
    values.push_back(10);
    values.push_back(20);
    values.push_back(12);

    if values.size() != 3 {
        panic!("expected size 3, got {}", values.size());
    }
    unsafe {
        if rust_std_vector::vector_sum(values.as_ptr()) != 42 {
            panic!("expected vector sum 42");
        }
    }
}
