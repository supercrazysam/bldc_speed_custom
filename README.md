This is the source code for the VESC DC/BLDC/FOC controller. Read more at  
http://vesc-project.com/

#############################################

I saw some implementation of VESC position mode that has velocity parameter
So i just brutally merge them manually, and it somehow works fine

Now with Firmware 3.4
+ Multi-turn positioning
+ Positon mode with Velocity parameter

All credits belongs to https://github.com/raess1/vesc-FW


#################################################
comm_can.c and mbed program changed.

****
In mbed  
void comm_can_set_vpos() 
originally only 4 byte, which is reponsible for the position
now changed buffer 4 to 8     seems 1 int32 is 4 byte, so 2 int32 => 8 byte
position, rpm   => 4 , 4   => so in total 8!

****************
In comm_can.c
CAN_PACKET_SET_POS
data8 seems to be for position  (confiusing)
so data8 changed to data16.... with  "data<<4" 4 byte shift left to ensure second set of data receive
position  then  velocity

