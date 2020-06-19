#include "mbed.h"
#include "protocol.h"

// #include <string>
// #include <cstring>
using namespace std;
Thread thread;

#define TX D8 // transmit pin name
#define RX D2 // recieve pin name
#define EN D3 // enable pin name

const float
Y = 30,
L = 20.8,
OFFSET1 = 10,
OFFSET2 = 20.5,
PI = atan(1.f) * 4.f;

//#define comp(str) (!strcmp(mode.c_str(), str))
#define delay_us wait_us(M1.delay + M2.delay)

//void communication();
void goto_xy(float, float);

DM50 M2(0x01, TX, RX, EN);
DM50 M1(0x02, TX, RX, EN);
Serial PC(USBTX, USBRX, 115200);
//RS485 Tx(D8), RX(D2), enable(D3)

DigitalIn b1(BUTTON1);
DigitalOut led(LED1);

// + positoin means CW
int main(){
    int i = 0;
    printf("===================program on====================\n");
    //M1.read_encoder();
    //M2.read_encoder();
    M1.init_zero(19);
    M2.init_zero(-23);
    M1.PC1(0);
    M2.PC1(0);
    delay_us;
    while(b1);
    led = 1;
    
    while(true){
        goto_xy(0, i * 10);
        goto_xy(15, i * 10);
        goto_xy(30, i * 10);
        if(!b1) i = (i+1) % 2;
    }
}

void goto_xy(float x, float y){
    float d1 = sqrt((OFFSET1-x)*(OFFSET1-x) + (Y-y)*(Y-y));
    float d2 = sqrt((OFFSET2-x)*(OFFSET2-x) + (Y-y)*(Y-y));
    float angle1, angle2;
    
    if (x>OFFSET1) {
        angle1 = PI + (isnan(acos(d1/(2*L)))? 0 :acos(d1/(2*L))) - atan((x-OFFSET1)/(Y-y));       //radians
    }
    else {
        angle1 = PI + (isnan(acos(d1/(2*L)))? 0 :acos(d1/(2*L))) + atan((OFFSET1-x)/(Y-y));       //radians
    }

  // ----- calculate motor2 angle when pen at start position (0,0)
    if (x > OFFSET2) {
        angle2 = PI - (isnan(acos(d2/(2*L)))? 0 :acos(d2/(2*L))) - atan((x-OFFSET2)/(Y-y));       //radians
    }
    else {
        angle2 = PI - (isnan(acos(d2/(2*L)))? 0 :acos(d2/(2*L))) + atan((OFFSET2-x)/(Y-y));       //radians
    }
    // printf("PI = %f\n", PI);
    angle1 = angle1 / PI * 180.f - 180.f;
    angle2 = angle2 / PI * 180.f - 180.f;
    //printf("angle1 = %f\n angle2 = %f", angle1, angle2);
    M1.PC1(angle1);
    M2.PC1(angle2);
    //printf("goto (%f, %f), ang1 = %f,  ang2 = %f\n", x, y, angle1, angle2);
    delay_us;
    wait_us(1000000);
}
// void communication(){
//     string mode;
//     int32_t position, speed, direction, ID;
//     position = speed = direction = ID = 0;
//     PC.scanf("%d %s %d %d %d",&ID, &mode[0], &position, &speed, &direction);
//             PC.printf("MANUAL ORDER : MORTOR %d :: %s, %d, %d, %d\n", ID, mode.c_str(), position, speed, direction);
            
//             if(comp("OL")){
//                 if(ID == 1) M1.OL(position); //power
//                 else M2.OL(position);
//             }
//             else if(comp("SC")){
//                 if(ID == 1) M1.SC(speed);
//                 else M2.SC(speed);
//             }
//             else if(comp("PC1")){
//                 if(ID == 1) M1.PC1(position);
//                 else M2.PC1(position);
//             }
//             else if(comp("PC2")){
//                 if(ID == 1) M1.PC2(position, speed);
//                 else M2.PC2(position, speed);
//             }
//             else if(comp("PC3")){
//                 if(ID == 1) M1.PC3(position, direction);
//                 else M2.PC3(position, direction);
//             }
//             else if(comp("PC4")){
//                 if(ID == 1) M1.PC4(position, speed, direction);
//                 else M2.PC4(position, speed, direction);
//             }
//             while(PC.readable()) PC.getc();
// }