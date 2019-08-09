#include <stdio.h>

void BubbleSort(int *a_pdNumArray, unsigned int a_udArrSize);

void BubbleSort(int *a_pdNumArray, unsigned int a_udArrSize)
{
    unsigned int udArrSize = a_udArrSize;
    unsigned int *pdNumArr = a_pdNumArray;
    unsigned int i=0, j=0, temp=0;

    printf("a_udArrSize = %d \n", a_udArrSize);

    for( i = 0; i < udArrSize; i++) {
        for( j = i; j < a_udArrSize; j++) {
            if( pdNumArr[j] < pdNumArr[i] ) {
                temp = pdNumArr[j];
                pdNumArr[j] = pdNumArr[i];
                pdNumArr[i] = temp;
            }
        }
    }
}


int main() {

    int number[] = {9,2,3,1,5,6,4,8,7,10,6,55,91,67};
    int i=0;

    BubbleSort((int *)&number, sizeof(number)/4 );

    for( i = 0; i < sizeof(number)/4; i++ ) {
        printf("%d ", number[i]);
    }

return 0;
}
