#include <lpfgall.h>

Start: {
  UseView(View1);
  UseView(View2);
  UseView(View3);

}
derivation length: 1;

module A();

axiom: A;

group 0:

interpretation:

vgroup View1:

A(): {
  static char text_str[64];
  sprintf (text_str, "In view 1");
  produce SetColor(1) Label(text_str) f(2.0) Sphere(1.0);
}

vgroup View2:

A(): {
  static char text_str[64];
  sprintf (text_str, "In view 2");
  produce SetColor(2) Label(text_str) f(2.0) Sphere(1.0);
}
vgroup View3:

A(): {
  static char text_str[64];
  sprintf (text_str, "In view 3");
  produce SetColor(3) Label(text_str) f(2.0) Sphere(1.0);
}

