#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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

typedef struct {
    UINT32 udAddr;
    UINT32 udValue;
} LOOPBACK_REG;


int main() {
    char name[20], tel[50], field[20], areaCode[20], code[20], offset[20], value[20], other[30];
    int age;
    int udoffset=0, udvalue = 0;
    FILE *fp0;
    LOOPBACK_REG *ptRegBuf;
    int ii = 0;

    ptRegBuf = (LOOPBACK_REG *)calloc(500, sizeof(LOOPBACK_REG));


    fp0 = fopen("ChecksumCrc_0.txt", "rt");

    while(!feof(fp0))
    {
        // fscanf(fp0, "ml %s %s \n", offset, value);

        // fscanf(fp0, "%s %s %*[^\n]", offset, value);
        // printf("offset >>>>>%s \n", offset);
        // printf("value  >>>>>%s \n", value);
        // printf("other  >>>>>%s \n", other);

        fscanf(fp0, "%s %s %*[\n]", offset, value);
        printf("offset >>>>>%s \n", offset);
        printf("value  >>>>>%s \n", value);
        printf("other  >>>>>%s \n", other);

        // sscanf( offset,  "%X", &ptRegBuf[ii].udAddr  );
        // sscanf( value, "%X", &ptRegBuf[ii].udValue );
        // printf("%d   udAddr=%x udValue=%x\n", ii, ptRegBuf[ii].udAddr, ptRegBuf[ii].udValue );

        ii++;
    }

    fclose(fp0);





    // char key[]="ABCDEF0102" ;
    // char hexbyte[3] = {0} ;
    // int octets[sizeof(key) / 2], d=0 ;
 
    // for( d = 0; d < strlen(key); d += 2 )
    // {
    //     // Assemble a digit pair into the hexbyte string
    //     hexbyte[0] = key[d] ;
    //     hexbyte[1] = key[d+1] ;
 
    //     // Convert the hex pair to an integer
    //     sscanf( hexbyte, "%X", &octets[d/2] ) ;
 
    //     // Show the integer has a hex pair with prefix
    //     printf( "0x%2.2X\n", octets[d/2] ) ;
    // }


    printf("------------------------------\n\n");


    sscanf("ml 0xFFED0100 0x0006D0F0", "ml %s %s", offset, value);
    printf("[%s] [%s]\n", offset, value);



    sscanf("0xFFED0104 0x00010000  // MTX_0 skew_clb_done [16]\n", "%s %s *\n", offset, value);
    printf("[%s] [%s]\n", offset, value);




    sscanf("name:john age:40 tel:082-313530", "%s", name);
    printf("%s\n", name);


    sscanf("name:john age:40 tel:082-313530", "%8s", name);
    printf("%s\n", name);


    sscanf("name:john age:40 tel:082-313530", "%[^:]", name);
    printf("%s\n", name);


    sscanf("name:john age:40 tel:082-313530", "%[^:]:%s", field, name);
    printf("%s %s\n", field, name);


    sscanf("name:john age:40 tel:082-313530", "name:%s age:%d tel:%s", name, &age, tel);
    printf("%s %d %s\n", name, age, tel);


    sscanf("name:john age:40 tel:082-313530", "%*[^:]:%s %*[^:]:%d %*[^:]:%s", name, &age, tel);
    printf("%s %d %s\n", name, age, tel);
    
    char protocol[10], site[50], path[50];
    sscanf("http://ccckmit.wikidot.com/cp/list/hello.txt", 
           "%[^:]:%*2[/]%[^/]/%[a-zA-Z0-9._/-]", 
           protocol, site, path);
    printf("protocol=%s site=%s path=%s\n", protocol, site, path);
    return 1;
}