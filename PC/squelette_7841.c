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

#define _CAN_PC2ARCOM
#include "../CAN/can.h"

MODULE_LICENSE("GPL");

/* define pour gestion t�che p�riodique */
#define STACK_SIZE  2000
#define TICK_PERIOD 1000000    //  1 ms
#define PERIODE_CONTROL 150000000 //1.5s
#define N_BOUCLE 10000000
#define NUMERO 1
#define PRIORITE 1

/* define pour gestion PCI CARTE CAN 7841 */
#define CAN_VENDOR_ID 0x144A
#define CAN_DEVICE_ID 0x7841
#define CAN_OUR_ID 8

/* d�claration variables pour addresse et irq */
/*  de la carte CAN PCI 7841 */    
static u32 base;  //adresse de base SJA1000 1
static u32 base2; //adresse de base SJA1000 2
static unsigned int irq_7841; // IRQ carte CAN

/*d�claration de t�ches */
static RT_TASK ma_tache;

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
 
  
  irq_7841 = dev->irq;  //on r�cup�re l'irq de la carte CAN
  
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

/************************************************/
/* squelette Tache p�riodique                   */
/************************************************/
void tache_periodique(){
  unsigned long int n=N_BOUCLE;
  unsigned long int i;
  char hello[8];
  unsigned short id;
  unsigned short dlc;

  while(n-->0){
    /* a completer */

    /* TX */
    dlc=8;
    hello[0]='s';
    hello[1]='a';
    hello[2]='k';
    hello[3]='h';
    hello[4]='e';
    hello[5]='l';
    hello[6]='l';
    hello[7]='o';
    can_send(CAN_OUR_ID, dlc, hello);

    /* RX */
    can_read(&id, dlc, hello);
    hello[7]='\0';
    printk("\nCAN::read : id=%u \'", id);
    for(i=0;i<dlc;i++){
      printk("%02x", (unsigned int)(*(hello+i)));
    }
    printk("\'\n\n");

    rt_task_wait_period();
  }

}

int caninit(void) {
  int ierr;
  RTIME now;

  /* init can  */
  ierr=init_7841();
  init_can();

  /* creation taches p�riodiques*/
  rt_set_oneshot_mode();
  ierr = rt_task_init(&ma_tache, tache_periodique, 0, STACK_SIZE, PRIORITE, 0, 0); 
  start_rt_timer(nano2count(TICK_PERIOD));
  now = rt_get_time();
  rt_task_make_periodic(&ma_tache, now, nano2count(PERIODE_CONTROL));

  return(0);
}

void canexit(void) {
  printk("uninstall 7841 CAN PCI driver\n");
  stop_rt_timer(); 
  rt_task_delete(&ma_tache);
}

module_init(caninit);
module_exit(canexit);
