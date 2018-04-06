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

#define SIG_TEST 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 

#define PID "/sys/kernel/ece453/pid"
bool busy = true;

//*****************************************************************************
// Helper function that is used to set the process ID of the user application
// in kerenl space
//*****************************************************************************
int set_pid(void)
{

	char buf[10];
	int fd = open(PID, O_WRONLY);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	sprintf(buf, "%i", getpid());
	if (write(fd, buf, strlen(buf) + 1) < 0) {
		perror("fwrite"); 
		return -1;
	}
  close(fd);
  return 0;
}

//*****************************************************************************
// Helper function that is used to clear the process ID of the user application
// when the application exits
//*****************************************************************************
int clear_pid(void)
{

	char buf[10];
	int fd = open(PID, O_WRONLY);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	
 memset(buf,0,10);
 if (write(fd, buf, strlen(buf) + 1) < 0) {
		perror("fwrite"); 
		return -1;
	}
  close(fd);
  return 0;
}


//*****************************************************************************
// This function acts as the "interrupt" handler when the kerel sends a signal
// to the user application
//*****************************************************************************
void receiveData(int n, siginfo_t *info, void *unused)
{
  // Data can be passed from the kernel to the application via the info struct.
  printf("Entering State %d\n\r",info->si_int);
}

//*****************************************************************************
// If the user presses CTRL-C, this function is called.  
//*****************************************************************************
void control_c_handler(int n, siginfo_t *info, void *unused)
{
  clear_pid();

  // Disable interrupts from the FPGA
  ece453_reg_write(IM_REG, 0);

  busy = false;
}


//*****************************************************************************
//*****************************************************************************
int main(int argc, char **argv)
{
	struct sigaction led_sig;
	struct sigaction ctrl_c_sig;

  // Set up handler for information set from the kernel driver
	led_sig.sa_sigaction = receiveData;
	led_sig.sa_flags = SA_SIGINFO;
	sigaction(SIG_TEST, &led_sig, NULL);

  // Set up handler for when the user presses CNTL-C to stop the application
	ctrl_c_sig.sa_sigaction = control_c_handler;
	ctrl_c_sig.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &ctrl_c_sig, NULL);

  // Configure the kernel so that it knows which PID (process ID) to send
  // the signal to
  set_pid();

  // Enable the FSM.  The LED should move right to left
//  ece453_reg_write(CONTROL_REG, 0x03);

  // enable reception of a signal the FSM reaches state 1 or 4. 
  ece453_reg_write(IM_REG, 0x01);
      
  printf("Please toggle slide switch 1!\n\r");

  /* Loop forever, waiting for interrupts */
  while (busy) {
	sleep(86400);	/* This will end early when we get an interrupt. */
  }

  // The only way that the application gets here is if the user presses
  // CTRL-C

  //Disable the FSM 
//  ece453_reg_write(CONTROL_REG, 0x0);

  clear_pid();

  // Disable interrupts
  ece453_reg_write(IM_REG, 0);
   
  return 0;
}

