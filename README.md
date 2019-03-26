This is the source code for the VESC DC/BLDC/FOC controller. Read more at  
http://vesc-project.com/

#############################################

I saw some implementation of VESC position mode that has velocity parameter
So i just brutally merge them manually, and it somehow works fine

Now with Firmware 3.4
+ Multi-turn positioning
+ Positon mode with Velocity parameter

All credits belongs to https://github.com/raess1/vesc-FW
http://vedder.se/forums/viewtopic.php?f=5&p=9459&sid=eac9679065d12af51106adaa0b530a6f#p9459

#################################################
comm_can.c and mbed program changed.

****
In mbed  
void comm_can_set_vpos() 
originally only 4 byte, which is reponsible for the position
now changed buffer 4 to 8     seems 1 int32 is 4 byte, so 2 int32 => 8 byte
position, rpm   => 4 + 4   => so in total 8!

```
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

```

****************
In comm_can.c
CAN_PACKET_SET_POS
data8 seems to be for position  (confiusing)
so data8 changed to data16.... 
position  then  velocity

```
//mc_interface_set_pid_pos(buffer_get_float32(rxmsg.data8, 1e6, &ind),600);   before parameter mod

float pos = ((float)buffer_get_float32(rxmsg.data16, 1e6, &ind)); // get position		    
float rpm = ((float)buffer_get_float32(rxmsg.data16, 1e0, &ind)); // get rpm parameter 
mc_interface_set_pid_pos(pos,rpm);
```


!!!!!!!!!!!!!!!!!!!!!!!
*****************************
Serious Problem found!
in mcpwm_foc.c
```
// PID is off. Return.
	if (m_control_mode != CONTROL_MODE_POS) {
		pos_i_term = 0;
		pos_prev_error = 0;
		return;
}
```
It seems that the position PID error (I & D) only reset if ```m_control_mode != CONTROL_MODE_POS```

And the velocity PID error (I & D) NEVER reset! WTF?!

This all causes the instability of the position mode with speed control

Pure position => OK

Pure velocity => OK

Position with velocity => Position ok(acceptable, seems still have problem?),  Velocity fucked
