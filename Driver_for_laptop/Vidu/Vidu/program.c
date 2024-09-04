#include <stdio.h>
#include "demo.h"

int main(void){
  int c = add(5,8);
  int d = multiply(5,8);
  printf("a+b=%d; a*b=%d \n", c, d);

  return 0;
}
