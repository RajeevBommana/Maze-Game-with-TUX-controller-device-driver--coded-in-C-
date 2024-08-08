/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"

#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tux_init(struct tty_struct* tty_struct);

int tux_buttons(struct tty_struct* tty_struct, unsigned long arg);

int set_LED(struct tty_struct* tty_struct, unsigned long arg);

unsigned char packets [PACKET_SIZE]; // 3 bytes from input
unsigned char prev [OUTPUT_SIZE]; // 6 bytes to restore clock

int ACK = 0; //command not successfully returned


//will process the incoming packets and populate my local data strctures so I can process
//them into button presses. In addition, it processes the the reset too by reinitializing
// and restoring the old clock;

void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	if(a == MTCP_BIOC_EVENT){
		packets[0] = b;
		packets[1] = c; // save these 2
	}
	else if(a == MTCP_RESET){
		tux_init(tty); // reinit
		tuxctl_ldisc_put(tty,prev,OUTPUT_SIZE); //restore old clock

	}
	else if(a == MTCP_ACK){ //response code
		ACK = 1;
	}
	else{
		return;
	}
	return;
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
//inputs are the tty struct ptr, file, cmd and ar
//will return 0 on success or -EINVAL if fail
//calls appropriate ioctl
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		tux_init(tty); //void return value
		return 0; //init should always succeed
	case TUX_BUTTONS:
		return tux_buttons(tty,arg);
	case TUX_SET_LED:
		return set_LED(tty,arg);
	case TUX_LED_ACK:
		return -EINVAL;
	case TUX_LED_REQUEST:
		return -EINVAL;
	case TUX_READ_LED:
		return -EINVAL;
	default:
	    return -EINVAL;
    }
}
//takes in ttystruct ptr and initializes tux controller
//returns void
//will send op codes to the tux
void tux_init(struct tty_struct* tty_struct)
{
	unsigned char buf[2]; // we will only send 2 opcodes so size will be 2
	buf[0] = MTCP_BIOC_ON;
	buf[1] = MTCP_LED_USR;
	tuxctl_ldisc_put(tty_struct,buf,2);
	ACK = 0;
};
//will take in an arg that is a reference to the variable in user mem
//returns 0 on success and -EINVAL on copy error
//it will return the proper low active code to the arg in user mem
int tux_buttons(struct tty_struct* tty_struct, unsigned long arg){

	int res = 0xFF; // should be an initial constant high signal
	unsigned char temp = packets[1]; //second byte taken in which contains left right up down
	temp = temp & 0x0F; //want to 0 out second half

	if((temp & 0x8) == 0){ //right button pressed, 0x8 is from the first 1 being raised
	 	res = 0x7F; // active low format of the output in the correct order for right button
	}
	if((temp & 0x4) == 0){ //down button pressed, 0x4 is from the second 1 being raised
	 	res = 0xDF; // active low format of the output in the correct order for down button
	}    
	if((temp & 0x2) == 0){ //left button pressed, 0x2 is from the first 2 being raised
	 	res = 0xBF ; // active low format of the output in the correct order for left button
	}    
	if((temp & 0x1) == 0){ //up button pressed, 0x1 is from the least significant bit being raised
	 	res = 0xEF; // active low format of the output in the correct order for up button
	}
	
	if(copy_to_user((unsigned long *)arg, &res, sizeof(res)) != 0){
		return -EINVAL;
	}
	
	return 0;                                                                                
}

//takes in tty struct ptr and a long arg with led display ifo
//processes the info and sends a 6 byte packet to the tux
// returns 0 on sucess
int set_LED(struct tty_struct* tty_struct, unsigned long arg){
	unsigned char masks [16] = {0xE7,0x06,0xCB,0x8F,0x2E,0xAD,0xED,0x86,0xEF,0xAE,0xEE,0x6D,0xE1,0x4F,0xE9,0xE8};//bit masks matching up with index

	unsigned char bitsize = ((arg >> 16) & 0xF); //get the mask for how many lights turned on, shift 16 to get the bits in the first byte, AND with 1111 or 0xF in order to only get the lower 4 bits
	
	int i ;
	unsigned char buf[OUTPUT_SIZE]; // 6 bytes of output into the led packet
	buf[0] = MTCP_LED_SET; //op code
	buf[1] = bitsize; // set how many lights r turned on

	
	
	for (i = 0; i < 4; i++){ //4 possible digits to be displayed
		
		unsigned char digit = arg & 0xF; //get the last digit so u only need the last 4 bits to be 1s so we need 0x1111 
		buf[2+i] = masks[digit]; //set the mask of the digit, we start at 2+i since the first 2 bits are already filled with opcode and led mask
		arg = arg >> 4; //move to next digit which is a hexadecimal so its 4 bits
	}
	buf[4] = buf[4] | 0x10; //make the 3rd led have the decimal always using 0x10 since 00010000 sets the decimal to 1, it is the 4 index since the first 2 are already filled

	for( i = 0; i < OUTPUT_SIZE; i++){
		prev[i] = buf[i]; //copt this state into previous in case reset calls
	}

	tuxctl_ldisc_put(tty_struct, buf,OUTPUT_SIZE); //put packet into the tux to display

	return 0;
}



