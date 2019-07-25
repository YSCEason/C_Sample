#include <stdio.h>


typedef struct
{
   int ttt;
}struct_test;



int doAdd(int a, int b);
int doMinus(int a, int b);
void doTest_pointer(int *a);
void Return_Struct_pointer(struct_test **response_pointer);


typedef struct
{
   int (*doAdd)(int a, int b);
   int (*doMinus)(int a, int b);
   void (*doTest_pointer)(int *a);
   void (*Return_Struct_pointer)(struct_test **response_pointer);
}SensorFuncStruct;




SensorFuncStruct interface =
{
   doAdd,
   doMinus,
   doTest_pointer,
   Return_Struct_pointer
};

int main(void) {




   printf("=> %d\n", interface.doAdd(5, 6));

   // printf("doMinus => %d\n", SensorFuncArray[0](5, 3));



   // -------------------------------

   int aa = 0;
   doTest_pointer(&aa);
   printf("aa = %d\n", aa);

   // -------------------------------

   struct_test *ttt2;
   Return_Struct_pointer(&ttt2);
   printf("ttt2 = %d\n", ttt2->ttt);



   return 0;
}





int doAdd(int a, int b) {
   return a + b;
}

int doMinus(int a, int b) {
   return a - b;
}







void doTest_pointer(int *a) {
   *a = 5;
}






struct_test stst = {666};

void Return_Struct_pointer(struct_test **response_pointer) {

   *response_pointer = (struct_test*)&stst;

   printf(">>> response_pointer->ttt = %d\n", ((struct_test*)*response_pointer)->ttt);
}
