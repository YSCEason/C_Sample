#include <stdio.h>

int doAdd(int, int);
int doMinus(int, int);


static int (*SensorFuncArray[])(int, int) = {
   doAdd,
   doMinus
};




int main(void) {

   // int (*my_func_ptr)(int, int);

   // my_func_ptr = doAdd;
   // printf("doAdd => %d\n", (*my_func_ptr)(5, 3));    //結果：8

   // my_func_ptr = doMinus;
   // printf("doMinus => %d\n", (*my_func_ptr)(5, 3));  //結果：2



   printf("doMinus => %d\n", SensorFuncArray[0](5, 3));


   return 0;
}





int doAdd(int a, int b) {
   return a + b;
}

int doMinus(int a, int b) {
   return a - b;
}
