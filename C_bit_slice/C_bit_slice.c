#include <stdio.h>
// #include <stdint.h>

/*
// https://stackoverflow.com/questions/25293157/reading-n-bit-elements-from-a-data-stream-in-c

uint64_t bit_slice(const uint8_t ds[], int start, int end){
    //index start, end {x | 1 <= x <= 64 }
    uint64_t s = 0;//memcpy(&s, ds, 8);
    int i, n = (end - 1) / 8;
    for(i = 0; i <= n; ++i)
        s = (s << 8) + ds[i];
    s >>= (n+1) * 8 - end;
    uint64_t mask = (((uint64_t)1) << (end - start + 1))-1;//len = end - start + 1
    s &= mask;
    return s;
}

int main(void){
    uint8_t data[8] = {
        0b01101010, 0b11010101, 0b11111111, 0b00000010, 0b00000000, 0b10000000 //0b... GCC extention
    };
    unsigned x = bit_slice(data, 21, 30);
    printf("%X\n", x);//3C0 : 11 1100 0000
    return 0;
}
*/


unsigned long bit_slice(unsigned char *ds, int start, int end){
    //index start, end {x | 1 <= x <= 64 }
    unsigned long Num_2byte = 0;//memcpy(&s, ds, 8);
    int i;
    int n = (end - 1) / 8;
    int s_w = (start - 1) % 8;


    printf("n = %d\n", s_h);
    printf("s_w = %d\n", s_w);


    // for(i = 0; i <= n; ++i)
    // {
    //     s = s + (ds[i]<<(i*8));
    //     printf("s=0x%x\n", s);
    // }

    // // s >>= (n+1) * 8 - end;
    // unsigned long mask = (((unsigned long)1) << (end - start + 1))-1;//len = end - start + 1
    // s &= mask;


    for(i = 0; i <= n; i+=2)
    {
        Num_2byte = (ds[i+1]<<8) + ds[i];

        printf("0x%x\n", Num_2byte);
        printf("(Num_2byte >> s_w) = 0x%x\n", (Num_2byte >> s_w));



    }


    return Num_2byte;
}

void main(void)
{
    unsigned int x=0;
    // unsigned char data[8] = {
    //     0b01101010, 0b11010101, 0b11111111, 0b00000010, 0b00000000, 0b10000000 //0b... GCC extention
    // };
    unsigned int data = 0xdeadbeef;


    // x = bit_slice((unsigned char *)&data, 21, 30);
    // printf("%X\n", x);//3C0 : 11 1100 0000

    x = bit_slice((unsigned char *)&data, 8, 18);
    // printf("0x%X\n", x);//1AB: 0110 1010 11

    // x = bit_slice((unsigned char *)&data, 5, 15);
    // printf("%X\n", x);//56A: 1010 1101 010

    // printf("%d\n", sizeof(unsigned char *));

}
