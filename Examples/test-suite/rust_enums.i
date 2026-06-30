%module rust_enums

%include <enums.swg>
%include <enumsimple.swg>
%include <enumtypesafe.swg>

%inline %{
enum RustColor {
  RustRed = 1,
  RustGreen = 2,
  RustBlue = 3
};

RustColor rust_next_color(RustColor color) {
  return color == RustRed ? RustGreen : RustBlue;
}
%}
