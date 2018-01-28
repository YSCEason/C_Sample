#include "stdio.h"

typedef float                 FLOAT32;
typedef double                FLOAT64;

typedef char                  CHAR;
typedef signed char           SINT8;

typedef char                   INT8;
typedef unsigned char         UINT8;
typedef short                  INT16;
typedef unsigned short        UINT16;
typedef long                   INT32;
typedef unsigned long         UINT32;
typedef long long              INT64;
typedef unsigned long long    UINT64;

typedef UINT8                 BOOL;


void main(int argc, char *argv)
{

    printf("---------- enum -----------\n");
    typedef enum{
        All,
        January,
        February,
        March,
        April,
        May
    }month;

    printf("All      : %d \n", All);
    printf("January  : %d \n", January);
    printf("February : %d \n", February);
    printf("March    : %d \n", March);
    printf("April    : %d \n", April);
    printf("May      : %d \n", May);

    printf("---------------------\n");
    enum{
        stop=6,
        stand,
        run,
        go
    };
    printf("stop     : %d \n", stop);
    printf("stand    : %d \n", stand);
    printf("run      : %d \n", run);
    printf("go       : %d \n", go);

    printf("---------------------\n");
    enum{
        stop1=6,
        stand1,
        run1=2,
        go1
    };
    printf("stop     : %d \n", stop1);
    printf("stand    : %d \n", stand1);
    printf("run      : %d \n", run1);
    printf("go       : %d \n", go1);

}