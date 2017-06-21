#include "can.h"

/********************************************************/
/*   Initialisation du/des SJA1000                      */
/********************************************************/
void init_can(void){
  outb(0x01, SJA1000_REG_CONTROL); /* Reset mode */
  outb(0xFF, SJA1000_REG_ACC_CODE); /* filter pattern */
  outb(0xFF, SJA1000_REG_ACC_MASK); /* accept all frames */
  outb(0x03, SJA1000_REG_BUS_TIME0); /* BTR */
  outb(0x1C, SJA1000_REG_BUS_TIME1); /* BTR */
  outb(0xFA, SJA1000_REG_OUTPUT_CTRL);
  outb(0x00, SJA1000_REG_CONTROL); /* no ITs & Reset mode off */
}

void can_read(unsigned short *id, unsigned short dlc, void *buffer){
  unsigned int i;
  unsigned short l_id=0;

  if(dlc>8){
    dlc=8;
  }

  if(readRBS()>0){
    /* RX DATA */
    for(i=0;i<dlc;i++){
      *(char*)(buffer+i)=inb(SJA1000_REG_RX_BYTES+i);
    }

    /* CAN ID */
    l_id=inb(SJA1000_REG_RX_ID)<<3;
    l_id|=(inb(SJA1000_REG_RX_RTRDLC)&0xE0)>>5;
    *id=l_id;

    clearRBS();
  }else{
    printk("CAN::read : nothing to read\n");
  }
}

void can_send(unsigned short id, unsigned short dlc, void *buffer){
  unsigned int i;

  if(dlc>8){
    dlc=8;
  }

  /* TX DATA */
  for(i=0;i<dlc;i++){
    outb(*(char*)(buffer+i), SJA1000_REG_TX_BYTES+i);
  }

  if(readTBS()>0){
    can_com_tx(id, 0, dlc);
    printk("can_send:txOK\n");
  }else{
    printk("can_send:failed\n");
  }
}

/** clears RBS bit so other msgs can be received
*/
void clearRBS(){
  outb(0x04, SJA1000_REG_COMMAND);
}

/** any msg received ?
  @retval >0 : YES
*/
unsigned char readRBS(){
  return (inb(SJA1000_REG_STATUS)&0x01);
}

/** can transmit msg ?
  @retval >0 : YES
*/
unsigned char readTBS(){
  return ((inb(SJA1000_REG_STATUS)&0x04)>>2);
}

void can_com_tx(unsigned short id, unsigned short rtr, unsigned short dlc){
  if(dlc>8){
    dlc=8;
  }

  if(readTBS()>0){
    /* CAN ID */
    outb((id>>3)&0xFF, SJA1000_REG_TX_ID);
    outb((inb(SJA1000_REG_TX_RTRDLC)&0x1F)|((id<<5)&0xE0), SJA1000_REG_TX_RTRDLC);
    /* CAN RTR */
    outb((inb(SJA1000_REG_TX_RTRDLC)&0xEF)|((rtr&0x01)<<4), SJA1000_REG_TX_RTRDLC);
    /* CAN DLC */
    outb((inb(SJA1000_REG_TX_RTRDLC)&0xF0)|(dlc&0x0F), SJA1000_REG_TX_RTRDLC);

    /* TX GO */
    outb(0x01, SJA1000_REG_COMMAND);
  }
}

