#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
int main(int argc, char **argv) {
  void *handle;
  double (*sum)(double, double);
  char *error;

  // �ʺA�}�Ҧ@�ɨ禡�w
  handle = dlopen ("libsum.so.1", RTLD_LAZY);
  if (!handle) {
    fputs (dlerror(), stderr);
    exit(1);
  }

  // ���o sum ��ƪ���}
  sum = dlsym(handle, "sum");
  if ((error = dlerror()) != NULL)  {
    fputs(error, stderr);
    exit(1);
  }

  // �ϥ� sum ���
  double a = 2.6, b = 4.2, c;
  c = sum(a, b);
  printf("%.1f + %.1f = %.1f\n", a, b, c);

  // �����@�ɨ禡�w
  dlclose(handle);
  return 0;
}