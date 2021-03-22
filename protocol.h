#include "mbed.h"

#define POSITION_MAX 3600
#define POWER_MAX 850
#define SPEED_MAX 720

using namespace std;
class DM50{
private:
    DigitalOut enable;
    Serial RS485, pc;
    uint8_t ID;
    float my_position, zero;

    union Data{
        int64_t dec;
        uint8_t byte[8];
    };
    
    // Calculate Check Sum
    int Check_Sum(uint8_t* data, int8_t pos, int8_t len){
        int cs = 0;
        for(int i = 0; i < len; i++)
            cs += *(data + i + pos);
        return cs & 0xFF;
    }
    
    // Set Data
    void Set_Data(uint8_t* target, uint8_t* data, int8_t pos, int8_t len){
        for(int i = 0; i < len; i++)
            *(target + i + pos) = *(data + i);
    }
    
    // Set a datum (overroading) 
    void Set_Data(uint8_t* target, int8_t data, int8_t pos){
            *(target + pos) = data;
    }

    // print transmit data at Serial Monitor of PC
    void print_Data(uint8_t* data, uint8_t len){
        pc.printf("  transfer data: ");
        for(int i = 0; i < len; i++)
            pc.printf("%#X ", *(data + i));
        pc.printf("\n");
    }

    // Write data and read data
    void transmit(uint8_t* data, uint8_t len){
        enable = 1;
        if(RS485.writable())
            for(int i = 0; i < len; i++)
                RS485.putc(*(data + i));
          else
              pc.printf("Motor %d :: RS485 write is failed\n", ID);
        wait_us(600);
    }

    // calculate delay_us
    float delay_us(float position){
        float speed = SPEED_MAX; //720 dps/lsb
        float ang = fabs(my_position - position);

        delay = ang / speed * 1000000;
        //pc.printf("ang = %f, myang = %f, delay = %f\n", ang, my_position, delay);
        my_position = position;
        return delay;
    }

public:
    float delay;
    DM50(uint8_t id, PinName TX, PinName RX , PinName enb)
    : ID(id), RS485(TX, RX, 115200),pc(USBTX, USBRX, 115200), enable(enb), my_position(0) { }

    // initiate initial position
    void init_zero(float data){
        zero = data;
    }
    // return my position
    int32_t position(){
        return my_position - zero;
    }

    // Read Encoder Data
    void read_encoder(){ //====================================================================================================
        enable = 1;
        uint8_t s[] = {0x3E, 0x90, ID, 0x00, 0xCF};
        Set_Data(s, Check_Sum(s, 0, 4), 4);
        //pc.printf("Motor %d :: Read Encoder Data\n", ID);
        //print_Data(s, 5);
        transmit(s, 5);

        wait_us(500);
        enable = 0;
        if(RS485.readable()){
            uint8_t tmp[30];
            RS485.scanf("%s", tmp);
            for(int i = 0; i < sizeof(tmp) / sizeof(uint8_t); i++)
                pc.printf("%#X ", tmp[i]);
            pc.printf("\n");
        }
        else
            pc.printf("Motor %d :: RS485 read is failed\n", ID);
    }
    // Open Loop: power [-850, 850], it moves at least power 100
    void OL(int32_t power){ //====================================================================================================
        if(power < -POWER_MAX || power > POWER_MAX){
            pc.printf("Motor %d failed :: Power is overed\n", ID);
            return;
        }
        Data pwr;
        pwr.dec = power;
        uint8_t s[] = {0x3E, 0xA0, ID, 0x02, 0xE1, 0x00, 0x01, 0x01}; //open loop control
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, pwr.byte, 5, 2); // speed data
        Set_Data(s, Check_Sum(s, 5, 2), 7); // check sum data
        print_Data(s, 8);
        transmit(s, 8);
    }


    // Speed Closed Loop: 0.01 dps/LSB, speed_MAX = 720 dps/LSB
    void SC(float speed){ //====================================================================================================
        pc.printf("Motor %d :: Speed Closed Loop Control :: Speed = %.2f\n",ID, speed);
        if(speed > SPEED_MAX || speed < -SPEED_MAX){
            pc.printf("Motor %d failed :: Speed is overed\n", ID);
            return;
        }
        uint8_t s[] = {0x3E, 0xA2, ID, 4, 0xE5, 0x40, 0x019, 0x01, 0x00, 0x5A}; //velocity of 720 dps
        Data spd;
        spd.dec = speed * 100;
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, spd.byte, 5, 4); // speed data
        Set_Data(s, Check_Sum(s, 5, 4), 9); // check sum data
        print_Data(s, 10);
        transmit(s, 10);
    }
    
    // Position Closed Loop1: Position_MAX = 3600 deg, Speed is fixed with 720 dps /LSB
    void PC1(float position){ //====================================================================================================
        //pc.printf("Motor %d :: Position Closed Loop1 :: Position = %.2f\n",ID, position);
        if(position > POSITION_MAX || position < -POSITION_MAX){
            pc.printf("Motor %d failed :: Position is overed\n", ID);
            return;
        }
        position += zero;
        Data pos;
        pos.dec = position * 100;
        uint8_t s[] = {0x3E, 0xA3, ID, 0x08, 0xEA, 0xA0, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C}; //360 degree move
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, pos.byte, 5, 8); // position data
        Set_Data(s, Check_Sum(s, 5, 8), 13); // check sum data
        //print_Data(s, 14);
        transmit(s, 14);
        delay_us(position);
        wait_us(500);
    }
    
    // Position Closed Loop2: position_MAX = 3600 deg,speed_MAX = 720 dps/LSB
    void PC2(float position, float speed){ //====================================================================================================
        pc.printf("Motor %d :: Position Closed Loop2 :: Position = %.2f, Speed = %.2f\n",ID, position, speed);
        if(position > POSITION_MAX || position < -POSITION_MAX ){
            pc.printf("Motor %d failed :: Position is Overed\n", ID);
            return;
        }
        else if (speed > SPEED_MAX || speed < -SPEED_MAX){
            pc.printf("Motor %d failed :: Speed is Overed\n", ID);
            return;
        }
        position += zero;
        Data pos, spd;
        spd.dec = speed * 100;
        pos.dec = position * 100;
        uint8_t s[] = {0x3E, 0xA4, ID, 0x0C, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x23, 0x00, 0x00, 0x4B}; // 0 deg and 90dps move
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, pos.byte, 5, 8); // position data
        Set_Data(s, spd.byte, 13, 4); // speed data
        Set_Data(s, Check_Sum(s, 5, 12), 17); // check sum
        print_Data(s, 18);
        transmit(s, 18);
        delay_us(position);
    }
    
    // Position Closed Loop3: dir = [ CCW = 0, CW = 1 ], position = [0 359.99 deg] 
    void PC3(float position, int8_t direction){ //====================================================================================================
        pc.printf("Motor %d :: Position Closed Loop3 :: Position = %.2f, Direction = %d\n",ID, position, direction);
        if(position > POSITION_MAX || position < -POSITION_MAX ){
            pc.printf("Motor %d failed :: Position is Overed\n", ID);
            return;
        }
        Data pos;
        position += zero;
        pos.dec = position * 100;
        uint8_t s[] = {0x3E, 0xA5, ID, 0x04, 0xE8, 0x00, 0x0C, 0x7B, 0x00, 0x87}; // 315 deg and CW move
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, direction, 5); // direction data
        Set_Data(s, pos.byte, 6, 3); // position data
        Set_Data(s, Check_Sum(s, 5, 4), 9); // check sum data
        print_Data(s, 10);
        transmit(s, 10);
        delay_us(position);
    }
    
    // Position Closed Loop3: dir = [ CCW = 0, CW = 1 ], position = [0 359.99 deg], speed = 720 dps/LSB
    void PC4(float position, float speed, int8_t direction){ //====================================================================================================
        pc.printf("Motor %d :: Position Closed Loop4 :: Position = %.2f, Speed = %.2f, Direction = %d\n",ID, position, speed, direction);
        if(position > POSITION_MAX || position < -POSITION_MAX ){
            pc.printf("Motor %d failed :: Position is Overed\n", ID);
            return;
        }
        else if (speed > SPEED_MAX || speed < -SPEED_MAX){
            pc.printf("Motor %d failed :: Speed is Overed\n", ID);
            return;
        }
        position += zero;
        Data pos, spd;
        pos.dec = position * 100;
        spd.dec = speed * 100;
        uint8_t s[] = {0x3E, 0xA6, ID, 0x08, 0xED, 0x00, 0x50, 0x46, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x81}; // 180 deg and 10 dps and CW
        Set_Data(s, Check_Sum(s, 0, 4), 4); // check sum data
        Set_Data(s, direction, 5); // direction data
        Set_Data(s, pos.byte, 6, 4); // position data
        Set_Data(s, spd.byte, 9, 4); // speed data
        Set_Data(s, Check_Sum(s, 5, 8), 13); // check sum data
        print_Data(s, 14);
        transmit(s, 14);
        delay_us(position);
    }
};
