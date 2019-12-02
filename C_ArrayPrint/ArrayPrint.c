#include "stdio.h"

typedef float                 FLOAT32;
typedef double                FLOAT64;

typedef char                  CHAR;
typedef signed char           SINT8;

typedef char                  INT8;
typedef unsigned char         UINT8;
typedef short                 INT16;
typedef unsigned short        UINT16;
typedef long                  INT32;
typedef unsigned long         UINT32;
typedef long long             INT64;
typedef unsigned long long    UINT64;

typedef UINT8                 BOOL;


void main(int argc, char *argv)
{
    int i;
    int AA[]={2, 4, 6, 12, 23 };
    int BB[]={4, 3, 8, 11, 23 };
    int *nextPtr = BB;

    for(i=0;i<5;i++)
    {
        if (AA[i] == *nextPtr)
        {
            nextPtr++;
        }
        else
        {
            printf("%d , ", AA[i]);
        }
    }


    system("pause");
}
