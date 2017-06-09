/**********************************************************/
/* squelette TP CAN carte 7841 communication CAN PCI      */
/* communication bus can                                  */
/* version : rtai 3.4                                     */
/* auteur: KOCIK R.                                       */
/**********************************************************/

#include<linux/init.h>
#include<linux/module.h>

#include <asm/io.h>
#include <linux/pci.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

MODULE_LICENSE("GPL");

/* define pour gestion tâche périodique */
#define STACK_SIZE  2000
#define TICK_PERIOD 1000000    //  1 ms
#define PERIODE_CONTROL 150000000 //1.5s
#define N_BOUCLE 10000000
#define NUMERO 1
#define PRIORITE 1

/* define pour gestion PCI CARTE CAN 7841 */
#define CAN_VENDOR_ID 0x144A
#define CAN_DEVICE_ID 0x7841

/* define pour gestion registres CAN 7841 */
/* CONTROL REGS */
#define SJA1000_REG_CONTROL base
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

/* déclaration variables pour addresse et irq */
/*  de la carte CAN PCI 7841 */    
static u32 base;  //adresse de base SJA1000 1
static u32 base2; //adresse de base sJA1000 2
static unsigned int irq_7841; // IRQ carte CAN

/*déclaration de tâches */
static RT_TASK ma_tache;

/* custom fcts */
void init_can();
int can_read(unsigned short *id, unsigned short dlc, void *buffer);
int can_send(unsigned short id, unsigned short dlc, void *buffer);
void clearRBS();
unsigned char readRBS();
unsigned char readTBS();
void can_com_tx(unsigned short id, unsigned short rtr, unsigned short dlc);

/*******************************************************/
/* recherche adresse et IRQ de la carte CAN sur le PCI */
/******************************************************/

int init_7841(void)
{
   u16 vendor,device;   
   u8 revision;
 struct pci_dev *dev=NULL;
 
  // RECHERCHE DE L'ADRESSE DE LA CARTE SUR LE BUS PCI
      dev = pci_get_device(CAN_VENDOR_ID, CAN_DEVICE_ID,dev);
      pci_read_config_dword(dev,PCI_BASE_ADDRESS_2,&base);
      if(dev==NULL) 
	{
	  printk("cannot init PCI 7841 card\n");
	  return -1;
	}
 
  base &= PCI_BASE_ADDRESS_IO_MASK; //adresse de base du 1er SJA1000
  base2 = base + 0x80; //adresse de base du 2eme SJA1000
  printk("CARTE 7841 : BASE  0x%4x\n",base);
 
  
  irq_7841 = dev->irq;  //on récupère l'irq de la carte CAN
  
  printk("CARTE 7841 Irq: %d\n", irq_7841);
  pci_read_config_byte(dev,PCI_REVISION_ID,&revision);
  printk("CARTE 7841 Revision : %u\n", revision);
  pci_read_config_word(dev,PCI_VENDOR_ID,&vendor);
  printk("CARTE 7841, VendorId : %x\n", vendor);
  pci_read_config_word(dev,PCI_DEVICE_ID,&device);
  printk("CARTE 7841 DeviceId : %x\n", device);
  printk("CARTE 7841 Base : %x\n", base);
  
  outb(0,base+9);            //Clear FIFO
  
  outb(0,base2+9);           //Clear FIFO du 2eme SJA1000
  
  printk(",CARTE PCI 7841 Init Ok ");
  return(0);  
}

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

void can_main(){
  char hello[8]={'s', 'a', 'k', 'h', 'e', 'l', 'l', 'o'};
  unsigned short id;  

  init_can();

  can_send(6, 8, hello);
  can_read(&id, 1, hello);
  hello[7]='\0';
  printk("\n\nCAN::read : %s\n\n\n", hello);
}

int can_read(unsigned short *id, unsigned short dlc, void *buffer){
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

int can_send(unsigned short id, unsigned short dlc, void *buffer){
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
  outb((inb(SJA1000_REG_STATUS)&0xFE), SJA1000_REG_STATUS);
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

/************************************************/
/* squelette Tache périodique                   */
/************************************************/
void tache_periodique(long id)
{
  while(1){
      /* a completer */

     rt_task_wait_period();
  }

}


int caninit(void) {

  int ierr;
  RTIME now;

   /* init can  */
   ierr=init_7841();
   init_can();

   /* creation taches périodiques*/
   //rt_set_oneshot_mode();
   // ierr = rt_task_init(&ma_tache,tache_periodique,0,STACK_SIZE, PRIORITE, 0, 0); 
   //   start_rt_timer(nano2count(TICK_PERIOD));
   //now = rt_get_time();
   // rt_task_make_periodic(&ma_tache, now, nano2count(PERIODE_CONTROL));
 
 
 return(0);
 
}

void canexit(void) {
  printk("uninstall 7841 CAN PCI driver\n");
   //stop_rt_timer(); 
  // rt_task_delete(&ma_tache);
}

module_init(can_main);
module_exit(canexit);
