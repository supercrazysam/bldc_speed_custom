#include "mbed.h"
#include "mbed.h"

#include <stdint.h>
//#include <mcp_can.h>
//#include <SPI.h>

bool sendPacket(uint32_t id, uint8_t packet[], int32_t len);
void vesc_can_set_duty(uint8_t controller_id, float duty);
void comm_can_set_rpm(uint8_t controller_id, float rpm);

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

typedef enum {
   CAN_PACKET_SET_DUTY = 0,
   CAN_PACKET_SET_CURRENT,
   CAN_PACKET_SET_CURRENT_BRAKE,
   CAN_PACKET_SET_RPM,
   CAN_PACKET_SET_POS,
   CAN_PACKET_FILL_RX_BUFFER,
   CAN_PACKET_FILL_RX_BUFFER_LONG,
   CAN_PACKET_PROCESS_RX_BUFFER,
   CAN_PACKET_PROCESS_SHORT_BUFFER,
   CAN_PACKET_STATUS
} CAN_PACKET_ID;


#define receive true;                            // Set INT to pin 2
//MCP_CAN CAN0(10);     // Set CS to pin 10
CAN CAN0(PA_11, PA_12, 500000);
Serial pc(USBTX, USBRX);
char display_buffer[8];


void buffer_append_int32(uint8_t* buffer, int32_t number, int32_t *index) {
   buffer[(*index)++] = number >> 24;
   buffer[(*index)++] = number >> 16;
   buffer[(*index)++] = number >> 8;
   buffer[(*index)++] = number;
}

void vesc_can_set_duty(uint8_t controller_id, float duty)
{
   int32_t send_index = 0;
   uint8_t buffer[4];
   buffer_append_int32(buffer, (int32_t)(duty * 100000.0), &send_index);
   sendPacket(controller_id | ((uint32_t)CAN_PACKET_SET_DUTY << 8), buffer, send_index);
}
void comm_can_set_rpm(uint8_t controller_id, float rpm) {
   int32_t send_index = 0;
   uint8_t buffer[4];
   buffer_append_int32(buffer, (int32_t)rpm, &send_index);
   sendPacket(controller_id | ((uint32_t)CAN_PACKET_SET_RPM << 8), buffer, send_index);
}

//https://github.com/skipper762/teensy_VESC_CANBUS
void comm_can_set_pos(uint8_t controller_id, float pos) {
  int32_t send_index = 0;
  uint8_t buffer[4];
  buffer_append_int32(buffer, (int32_t)(pos *1e6), &send_index);  //* 1e7 pos/5 = pos
  sendPacket(controller_id | ((uint32_t)CAN_PACKET_SET_POS << 8), buffer, send_index);
}

void comm_can_set_vpos(uint8_t controller_id, float pos, float rpm) {
  int32_t send_index = 0;
  uint8_t buffer[8];   //changed buffer 4 to 8     seems 1 int32 is 4 byte, so 2 int32 => 8 byte   .... while in firmware comm_can.h   data8 changed to data16.... with  "data<<4" 4 byte shift to ensure data receive
  buffer_append_int32(buffer, (int32_t)(pos *1e6), &send_index);  
  buffer_append_int32(buffer, (int32_t)(rpm *1e0), &send_index); 
  sendPacket(controller_id | ((uint32_t)CAN_PACKET_SET_POS << 8), buffer, send_index);
}


bool sendPacket(uint32_t id, uint8_t packet[], int32_t len)
{

   if (CAN0.write(CANMessage(id, (const char*)packet, sizeof(packet),CANData,CANExtended)))
   {
   
      return true;
   }
   else {
      //Serial.println("Error Sending Message...");
      return false;
   }
}


CANMessage receive_msg;
CANMessage send_msg;
int32_t erpm = 0;
int16_t current = 0;
int16_t duty = 0;

void show_status()
{
 if (CAN0.read(receive_msg)) 
{
        
    if ((receive_msg.id & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
         sprintf(display_buffer ,"Extended ID: 0x%.8lX  DLC: %1d  Data:", (receive_msg.id & 0x1FFFFFFF), sizeof(receive_msg.id));
      else
         sprintf(display_buffer, "Standard ID: 0x%.3lX       DLC: %1d  Data:", receive_msg.id, sizeof(receive_msg.id));
    pc.printf(display_buffer);
    pc.printf("\n\r");
    
    
    if ((receive_msg.id & 0x40000000) == 0x40000000) 
    {    // Determine if message is a remote request frame.
         sprintf(display_buffer, " REMOTE REQUEST FRAME");
         pc.printf(display_buffer);
         pc.printf("\n\r");
    }
    else 
    {
        erpm = (receive_msg.data[0] << 24) | (receive_msg.data[1] << 16) | (receive_msg.data[2] << 8) | receive_msg.data[3];
        current = (receive_msg.data[4] << 8) | receive_msg.data[5];
        duty = (receive_msg.data[6] << 8) | receive_msg.data[7];
    }
      
      sprintf(display_buffer, "RPM %d", erpm );
      pc.printf(display_buffer);
      pc.printf("\n\r");
      float curr = (float)current;
      curr = curr/10;
      float dut = (float)duty;
      dut = dut/1000;
      sprintf(display_buffer, "Current %f", curr );
      pc.printf(display_buffer);
      pc.printf("\n\r");
      
      sprintf(display_buffer, "DUTY %f", dut );
      pc.printf(display_buffer);
      pc.printf("\n\r");
      
      
      for (int i = 0; i< sizeof(receive_msg.data);i++){
                      sprintf(display_buffer, " 0x%.2X", receive_msg.data[i]);
            pc.printf(display_buffer);
            pc.printf("\n\r"); 
          
          }
      sprintf(display_buffer, "SIZE %d", sizeof(receive_msg.data) );
      pc.printf(display_buffer);
      pc.printf("\n\r");
          
}   
}


int main() 
{


while(1)
 {
   //vesc_can_set_duty(1, 0.5);
   for (int i =1; i < 10; i++)
   {
   
   //comm_can_set_rpm(1, 1000*i);
   //wait_ms(3500);
   
   
   comm_can_set_vpos(1, 0.0,600);
   wait_ms(3500);
   
   comm_can_set_vpos(1, 90.0,6000);
   wait_ms(3500);
   
   //vesc_can_set_duty(1,0.1);
   //wait_ms(500);
   
   
   //wait_ms(600);
   //comm_can_set_rpm(1, 0);
   
   //show_status();
   
   
   
   }
   

   
   

   
    
}
}



