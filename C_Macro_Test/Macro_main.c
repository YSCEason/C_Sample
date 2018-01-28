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



//-------------- Macro test ----------------

#define Macro_01(exp) printf("%s\n", #exp);

#define Macro_02(x,y) x##y

#define Macro_03(x) #x

// Common Func

#define MEM_B( x )             ( *( (INT8 *) (x) ) )                     // 得到指定 address 上的一個byte
#define MEM_W( x )             ( *( (UINT16 *) (x) ) )                   // 得到指定 address 上的一個word
#define MEM_L( x )             ( *( (UINT32 *) (x) ) )                   // 得到指定 address 上的一個UINT32

#define MAX( x, y )            ( ((x) > (y)) ? (x) : (y) )
#define MIN( x, y )            ( ((x) < (y)) ? (x) : (y) )

#define SWAP(a, b)             do { a ^= b; b ^= a; a ^= b; } while(0)

#define FPOS( type, field )    ( (UINT16) &(( type *) 0)-> field )        // 得到一個field在結構體(struct)中的偏移量

#define FSIZ( type, field )    sizeof( ((type *) 0)->field )              // 得到一個結構體中field所佔用的位元組數

#define FLIPW( ray )           ( (((UINT32) (ray)[0]) * 256) + (ray)[1] ) // 按照LSB格式把兩個位元組轉化為一個Word

#define IO_RD(port)            (*((volatile UINT32 *)(port)))
#define IO_WT(port, val)       (*((volatile UINT32 *)(port))) = ((UINT32) (val)))

//------------------------------------------

void main(int argc, char *argv)
{
    int asdfsadf=0;
    int var123=100;
    int t01=5, t02=9;

    Macro_01(asdfsadf);

    printf("Macro_02(var, 123) = %d\n", Macro_02(var, 123));

    printf("Macro_03(2+2+8) = %s", Macro_03(2+2+8) );

    SWAP(t01, t02);
    printf("SWAP(5, 9) = %d   %d\n", t01, t02);

}