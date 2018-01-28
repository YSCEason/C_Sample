#include <stdio.h>

//function宣告 
int doAdd(int, int);
int doMinus(int, int);

int main(void) {
   //宣告 function pointer
   //注意所設定的參數數量與型態
   int (*my_func_ptr)(int, int);

   //function pointer 指向 doAdd
   my_func_ptr = doAdd;
   printf("doAdd => %d\n", (*my_func_ptr)(5, 3));    //結果：8

   //function pointer 指向 doMinus
   my_func_ptr = doMinus;
   printf("doMinus => %d\n", (*my_func_ptr)(5, 3));  //結果：2 

   return 0;
}   //end main


int doAdd(int a, int b) {
   return a + b;
}   //end doAdd

int doMinus(int a, int b) {
   return a - b;
}   //end doMinus