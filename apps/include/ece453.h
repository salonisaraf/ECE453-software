/*
 */

#ifndef __ECE453_APP_H__
#define __ECE453_APP_H__

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>

#define CONTROL_REG	"/sys/kernel/ece453/control"
#define DEVICE_ID_REG	"/sys/kernel/ece453/device_id"
#define GPIO_IN_REG	"/sys/kernel/ece453/gpio_in"
#define GPIO_OUT_REG	"/sys/kernel/ece453/gpio_out"
#define IM_REG		"/sys/kernel/ece453/interrupt_mask"
#define IRQ_REG		"/sys/kernel/ece453/irq"
#define PID_REG		"/sys/kernel/ece453/pid"
#define STATUS_REG	"/sys/kernel/ece453/status"

//*****************************************************************************
//*****************************************************************************
int ece453_reg_read(char *reg_name);

//*****************************************************************************
//*****************************************************************************
int ece453_reg_write(char *reg_name, int value);

#endif
