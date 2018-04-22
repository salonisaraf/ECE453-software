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
#include <curl/curl.h>

//Sending start and end of session to web application
#define GET_STATE_ON "https://us-central1-ece453smartstation.cloudfunctions.net/toggle?state=on"
#define GET_STATE_OFF "https://us-central1-ece453smartstation.cloudfunctions.net/toggle?state=off"

#define SIG_TEST 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 

#define I2C_ADDR 0x1C //I2C slave device address

#define PID "/sys/kernel/ece453/pid"
bool busy = true;
int detect_touch(void);
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
  printf("\n\rEntering State %d: ",info->si_int);
    CURL *curl;
    CURLcode res;
    char* header_message;
    volatile bool UPDATE_PAGE = false;
    volatile int state = 0;
    state = info->si_int;
    if (state == 1){
        header_message = GET_STATE_ON;
    }else if (state == 2){
        header_message = GET_STATE_OFF;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, header_message);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    
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
//  Determine which capactitive touch sensor is being pressed
//*****************************************************************************
int detect_touch(void){
	char buffer[1];
	int fd;

	fd = open("/dev/i2c-2", O_RDONLY);

	if (fd < 0) {
		printf("Error opening file\n");
		return 1;
	}

	if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
		printf("ioctl error\n");
		return 1;
	}

	buffer[0]=0xFF;
	write(fd, buffer, 1);
	
	read(fd, buffer, 1);

	if(fd != 0x3E){
	printf("0x%02X\n", buffer[0]);
	}
	close(fd);
	return fd;
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

  // enable reception of a signal the FSM reaches state 1 or 4. 
  ece453_reg_write(IM_REG, 0x01);
      
  printf("Please toggle slide switch 1!\n\r");


  /* Loop forever, waiting for interrupts */
  while (busy) {
      	int cap_touch_sig = 8;
      	//Print I2C value to console
  	cap_touch_sig = detect_touch();
   	// Write the capacitive touch values to the control register
  	ece453_reg_write(CONTROL_REG, cap_touch_sig);
//	sleep(86400);	/* This will end early when we get an interrupt. */
  }

  // The only way that the application gets here is if the user presses
  // CTRL-C
  

  clear_pid();

  // Disable interrupts
  ece453_reg_write(IM_REG, 0);
   
  return 0;
}

