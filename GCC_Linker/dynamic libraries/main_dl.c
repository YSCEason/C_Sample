#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
int main(int argc, char **argv) {
  void *handle;
  double (*sum)(double, double);
  char *error;

  // 動態開啟共享函式庫
  handle = dlopen ("libsum.so.1", RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }

  // 取得 sum 函數的位址
  sum = dlsym(handle, "sum");
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    exit(1);
  }

  // 使用 sum 函數
  double a = 2.6, b = 4.2, c;
  c = sum(a, b);
  printf("%.1f + %.1f = %.1f\n", a, b, c);

  // 關閉共享函式庫
  dlclose(handle);
  return 0;
}