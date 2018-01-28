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


struct Student_Detail {
    int age;
    char *name;
    char *address;
};

struct Student_Data {
        int stuid;
        struct Student_Detail detail;
};

void main(int argc, char *argv)
{
    struct Student_Data x;
    x.stuid          = 100;
    x.detail.age     = 20;
    x.detail.name    = "Johnson Lee";
    x.detail.address = "Nation Chi Nan University";

    printf("%d\n"    , x.stuid         );
    printf("%d\n"    , x.detail.age    );
    printf("%s\n"    , x.detail.name   );
    printf("%s\n"    , x.detail.address);
}