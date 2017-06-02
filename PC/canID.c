/* ... */

#include <stdio.h>

int main(){
    unsigned char r1=0x01;
    unsigned char r2=0x0f;
    unsigned short id=2047;

    /* xxxxxxxxxxx -> xxxxxxxx + xxx..... */
    printf("r=%u - r1=%u - r2=%u \n", id, r1, r2);    
    printf("r=%x - r1=%x - r2=%x \n", id, r1, r2);    

    r1=(id>>3)&0xFF;
    r2|=(id<<5)|0xE0;

    printf("r=%u - r1=%u - r2=%u \n", id, r1, r2);    
    printf("r=%x - r1=%x - r2=%x \n", id, r1, r2);    

    return 0;
}
