/* Header file for AD converter class */
/* Authors: Jillian Thomas, Richard Mitic */

#ifndef AD7705_H
#define AD7705_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "gz_clk.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Define constants for channel numbers */
#define CHAN1 0
#define CHAN2 1

class AD7705
{
private:
	char *device;
	uint8_t mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t dly;
	
	int fd; // SPI file descriptor

public:
	void initChannel(int channel);
	int dataReady(int channel);
	int read(int channel);
    AD7705(); //  default constructor
	~AD7705();//  default destructor
};
 
#endif
