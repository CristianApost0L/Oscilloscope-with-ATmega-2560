#include <stdio.h>

typedef struct {
  int a_int;
  float a_float;
} A;

typedef struct {
  A parent;
  int b_int;
} B;

void A_print(const A* a_ptr) {
  printf("A this: %lx { a_int: %d, a_float: %f }",
         a_ptr ,
         a_ptr->a_int,
         a_ptr->a_float);
}

void B_print(const B* b_ptr) {
  printf("B this: %lx, { parent:", b_ptr);
  A_print(&(b_ptr->parent));
  printf(", b_int: %d}", b_ptr->b_int);
}

int main() {
  A a={1, 0.7};
  A_print(&a);

  B b={{2, 0.4}, 5};
  B_print(&b);

  a=b.parent;
  A_print((const A*)(&b));
  
}
