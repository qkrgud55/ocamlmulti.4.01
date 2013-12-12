#include <stdio.h>

int test_fun(void *ctx, void *first_arg){
  int ret_val = 7;
  printf("test_fun %p %p\n", ctx, first_arg);
  return ret_val*2+1;
}
