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
    UINT8 a = 0x50;

    printf("a : 0x%x \n", a);
    printf("&a : 0x%x \n", &a);

    printf("---------------------\n\n");

    INT32  i[5]  = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    INT32 *ip    = i;

    printf("&i      = %p\n", &i);
    printf("ip      = %p\n", ip);

    printf("---------------------\n\n");
    printf("ip        = 0x%p\n", ip  );
    printf("ip+1      = 0x%p\n", ip+1);
    printf("ip+2      = 0x%p\n", ip+2);
    printf("ip+3      = 0x%p\n", ip+3);
    printf("ip+4      = 0x%p\n", ip+4);

    printf("---------------------\n\n");
    printf("*ip       = 0x%p\n", *ip    );
    printf("*(ip+1)   = 0x%p\n", *(ip+1));
    printf("*(ip+2)   = 0x%p\n", *(ip+2));
    printf("*(ip+3)   = 0x%p\n", *(ip+3));
    printf("*(ip+4)   = 0x%p\n", *(ip+4));

    printf("---------------------\n\n");
    printf("*ip++     = 0x%p\n", *ip++);
    printf("*ip       = 0x%p\n\n", *ip);

    printf("*++ip     = 0x%p\n", *++ip);
    printf("*ip       = 0x%p\n\n", *ip);

    printf("++*ip     = 0x%p\n", ++*ip);
    printf("*ip       = 0x%p\n\n", *ip);

    printf("(*ip)++   = 0x%p\n", (*ip)++);
    printf("*ip       = 0x%p\n\n", *ip);

    printf("---------------------\n\n");
    printf("ip[0]     = 0x%p\n", ip[0]);
    printf("ip[1]     = 0x%p\n", ip[1]);
    printf("ip[2]     = 0x%p\n", ip[2]);
    printf("ip[3]     = 0x%p\n", ip[3]);
    printf("ip[4]     = 0x%p\n", ip[4]);

    printf("---------------------\n\n");
    printf("&ip[0]    = 0x%p\n", &ip[0]);
    printf("&ip[1]    = 0x%p\n", &ip[1]);
    printf("&ip[2]    = 0x%p\n", &ip[2]);
    printf("&ip[3]    = 0x%p\n", &ip[3]);
    printf("&ip[4]    = 0x%p\n", &ip[4]);

    printf("---------------------\n\n");
    printf("&ip[1]    = 0x%p\n", &ip[1]);
    printf("&ip[1]+1  = 0x%p\n", &ip[1]+1);

    printf("ip[2]     = 0x%p\n", ip[2]);
    printf("ip[2]+1   = 0x%p\n", ip[2]+1);


    printf("---------------------\n\n");
    UINT32 j[5] = {0x22, 0x33, 0x44, 0x55, 0x66};
    UINT32 k = (UINT32)j;

    printf("j                       = 0x%p\n", j  );
    printf("j+1                     = 0x%p\n", j+1);
    printf("j+2                     = 0x%p\n", j+2);
    printf("j+3                     = 0x%p\n", j+3);
    printf("j+4                     = 0x%p\n", j+4);

    printf("\n");

    printf("*(j  )                  = 0x%p\n", *(j  ));
    printf("*(j+1)                  = 0x%p\n", *(j+1));
    printf("*(j+2)                  = 0x%p\n", *(j+2));
    printf("*(j+3)                  = 0x%p\n", *(j+3));
    printf("*(j+4)                  = 0x%p\n", *(j+4));

    printf("\n");

    printf("k                       = 0x%p\n", k  );
    printf("k+1                     = 0x%p\n", k+1);
    printf("k+2                     = 0x%p\n", k+2);
    printf("k+3                     = 0x%p\n", k+3);
    printf("k+4                     = 0x%p\n", k+4);

    printf("\n");

    printf("(UINT32 *)k             = 0x%p\n", (UINT32 *)k  );
    printf("(UINT32 *)k+1           = 0x%p\n", (UINT32 *)k+1);
    printf("(UINT32 *)k+2           = 0x%p\n", (UINT32 *)k+2);
    printf("(UINT32 *)k+3           = 0x%p\n", (UINT32 *)k+3);
    printf("(UINT32 *)k+4           = 0x%p\n", (UINT32 *)k+4);

    printf("\n");

    printf("(UINT32 *)k             = 0x%p\n", (UINT32 *)k);
    printf("*(UINT32 *)k            = 0x%p\n", *(UINT32 *)k);

    printf("\n");

    printf("(UINT32 *)k++           = 0x%p\n", (UINT32 *)k++);
    printf("(UINT32 *)k             = 0x%p\n", (UINT32 *)k);

    printf("\n");

    printf("(UINT32 *)++k           = 0x%p\n", (UINT32 *)++k);
    printf("(UINT32 *)k             = 0x%p\n", (UINT32 *)k);




    printf("---------------------\n\n");
    UINT32 pp[5] = {0x11, 0x33, 0x55, 0x77, 0x99};
    UINT32 b=0x10;
    UINT32 *qq;
    qq = pp;

    printf("*qq = %p\n"    , *qq);
    printf("*qq+++b = %p\n", *qq+++b);
    printf("*qq = %p\n"    , *qq);



    printf("---------------------\n\n");

    UINT8 *ucq;
    b = 0x34567890;
    ucq = (UINT8 *)&b;
    printf("*b      = 0x%p\n"     , b);
    printf("*qq     = %p\n"    , *ucq);
    printf("*(qq+1) = %p\n"    , *(ucq+1));
    printf("*(qq+2) = %p\n"    , *(ucq+2));
    printf("*(qq+3) = %p\n"    , *(ucq+3));


    system("pause");
}
