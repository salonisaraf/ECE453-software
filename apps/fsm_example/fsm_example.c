/*
 * fsm_example.c
 * Signal recving program in the user space
 * Originated from http://people.ee.ethz.ch/~arkeller/linux/code/signal_user.c
 * Modified by daveti
 * Aug 23, 2014
 * root@davejingtian.org
 * http://davejingtian.org
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "../include/ece453.h"

//*****************************************************************************
//*****************************************************************************
int main(int argc, char **argv)
{
  int state = 0;
  //int curr_led = 0;
  
  printf("Begin\n");
  // Enable the FSM.  The LED should move right to left
	ece453_reg_write(CONTROL_REG, 0x03);
	
  // Print out a message to the user to start pressing KEY0
	printf("Press KEY0 until the 4th RED LED is on\n");
	
  // Wait until the FSM reaches state LED3.
	while(ece453_reg_read(STATUS_REG) != 0x04) {
		//curr_led = ece453_reg_read(STATUS_REG);
		//printf("LED State is: %i \n", curr_led);
	}
	
  // Change the direction of the LEDs to that the RED led will move from left
  // to right
	ece453_reg_write(CONTROL_REG, 0x01);
	
	printf("Press KEY0 until the 1st RED LED is on\n");
  // Wait until the FSM reaches state LED0
	while(ece453_reg_read(STATUS_REG) != 0x01) {}

	printf("Done... exiting\n");
  // Exit
  return 0;
}

