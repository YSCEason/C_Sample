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


#define N (9)

void Print_Triangle_left(void);
void Print_Triangle_right(void);
void Print_Triangle(void);
void Print_Inv_Triangle(void);
void Print_Diamond(void);
void Print_Triangle_Digital(void);

void main(int argc, char *argv)
{
    Print_Triangle_left();
    printf("------------------\n");
    Print_Triangle_right();
    printf("------------------\n");
    Print_Triangle();
    printf("------------------\n");
    Print_Inv_Triangle();
    printf("------------------\n");
    Print_Diamond();
    printf("------------------\n");
    Print_Triangle_Digital();

    system("pause");
}


void Print_Triangle_left(void)
{
    UINT8 ii, jj, Level;

    Level= N;

    for(ii=1; ii <= Level ;ii++)
    {
        for(jj=0; jj<ii ; jj++)
            printf("*");
        printf("\n");
    }
}


void Print_Triangle_right(void)
{
    UINT8 ii, jj, Level;
    UINT8 space;

    Level= N;
    space=(N-1);

    for(ii=1; ii<=Level ;ii++)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj<ii ; jj++)
            printf("*");
        printf("\n");
        space--;
    }
}


void Print_Triangle(void)
{
    UINT8 ii, jj, Level;
    UINT8 space;

    Level= N*2;
    space=(N-1);

    for(ii=1; ii <= Level ;ii+=2)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj<ii ; jj++)
            printf("*");
        printf("\n");
        space--;
    }
}

void Print_Inv_Triangle(void)
{
    UINT8 ii, jj, Level;
    UINT8 space;

    Level= N*2;
    space=0;

    for(ii=1; ii <= Level ;ii+=2)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj < (Level-ii) ; jj++)
            printf("*");
        printf("\n");
        space++;
    }
}

void Print_Diamond(void)
{
    UINT8 ii, jj, Level;
    UINT8 space;

    Level= N+1;
    space=(N/2);

    for(ii=1; ii <= Level ;ii+=2)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj<ii ; jj++)
            printf("*");
        printf("\n");
        space--;
    }

    space=1;
    for(ii=1+2; ii < Level ;ii+=2)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj < (Level-ii) ; jj++)
            printf("*");
        printf("\n");
        space++;
    }
}


void Print_Triangle_Digital(void)
{
    UINT8 ii, jj, Level;
    UINT8 space;

    Level= N*2;
    space=(N-1);

    for(ii=1; ii <= Level ;ii+=2)
    {
        for(jj=0; jj<space ; jj++)
            printf(" ");
        for(jj=0; jj<ii ; jj++)
            printf("%d", N-space);
        printf("\n");
        space--;
    }
}
