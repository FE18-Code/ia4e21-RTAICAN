
/* Todo module format */

#ifndef CAN_H
#define CAN_H

/* define pour gestion registres CAN 7841 */
/* CONTROL REGS */
#define SJA1000_REG_CONTROL _SJA1000_REGS_BASEADDR
#define SJA1000_REG_COMMAND (SJA1000_REG_CONTROL+1)
#define SJA1000_REG_STATUS (SJA1000_REG_COMMAND+1)
#define SJA1000_REG_INTERRUPT (SJA1000_REG_STATUS+1)
#define SJA1000_REG_ACC_CODE (SJA1000_REG_INTERRUPT+1)
#define SJA1000_REG_ACC_MASK (SJA1000_REG_ACC_CODE+1)
#define SJA1000_REG_BUS_TIME0 (SJA1000_REG_ACC_MASK+1)
#define SJA1000_REG_BUS_TIME1 (SJA1000_REG_BUS_TIME0+1)
#define SJA1000_REG_OUTPUT_CTRL (SJA1000_REG_BUS_TIME1+1)
#define SJA1000_REG_TEST (SJA1000_REG_OUTPUT_CTRL+1)
/* TX REGS */
#define SJA1000_REG_TX_ID (SJA1000_REG_TEST+1)
#define SJA1000_REG_TX_RTRDLC (SJA1000_REG_TX_ID+1)
#define SJA1000_REG_TX_BYTES (SJA1000_REG_TX_RTRDLC+1) /* 8 REGS : BYTES */
/* RX REGS */
#define SJA1000_REG_RX_ID (SJA1000_REG_TX_BYTES+8)
#define SJA1000_REG_RX_RTRDLC (SJA1000_REG_RX_ID+1)
#define SJA1000_REG_RX_BYTES (SJA1000_REG_RX_RTRDLC+1) /* 8 REGS : BYTES */

/** can initialisation function (all registers)
*/
void init_can();

void can_read(unsigned short *id, unsigned short dlc, void *buffer);

void can_send(unsigned short id, unsigned short dlc, void *buffer);

/** clears RBS bit so other msgs can be received
*/
void clearRBS();

/** any msg received ?
  @retval >0 : YES
*/
unsigned char readRBS();

/** can transmit msg ?
  @retval >0 : YES
*/
unsigned char readTBS();

/** sub-function of can_send()
*/
void can_com_tx(unsigned short id, unsigned short rtr, unsigned short dlc);

#endif

