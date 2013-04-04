/*
 * AD7705 test program for the Raspberry PI
 * Authors: Jillian Thomas, Richard Mitic
*/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "gz_clk.h"
	
// ADC class header file
#include "ad7705.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static void writeReset(int fd, uint8_t bits, uint32_t speed, uint16_t delay)
{
	int ret;
	uint8_t tx1[5] = {0xff,0xff,0xff,0xff,0xff};
	uint8_t rx1[5] = {0};
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx1;
	tr.rx_buf = (unsigned long)rx1;
	tr.len = ARRAY_SIZE(tx1);
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}

static void writeReg(int fd, uint8_t v, uint8_t bits, uint32_t speed, uint16_t delay)
{
	int ret;
	uint8_t tx1[1];
	tx1[0] = v;
	uint8_t rx1[1] = {0};
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx1;
	tr.rx_buf = (unsigned long)rx1;
	tr.len = ARRAY_SIZE(tx1);
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

}

static uint8_t readReg(int fd, uint32_t speed, uint16_t delay)
{
	int ret;
	uint8_t tx1[1];
	tx1[0] = 0;
	uint8_t rx1[1] = {0};
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx1;
	tr.rx_buf = (unsigned long)rx1;
	tr.len = ARRAY_SIZE(tx1);
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = 8;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	  pabort("can't send spi message");
	  
	return rx1[0];
}

static int readData(int fd, uint32_t speed, uint16_t delay)
{
	int ret;
	uint8_t tx1[2] = {0,0};
	uint8_t rx1[2] = {0,0};
	struct spi_ioc_transfer tr;
	tr.tx_buf = (unsigned long)tx1;
	tr.rx_buf = (unsigned long)rx1;
	tr.len = ARRAY_SIZE(tx1);
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = 8;
		
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	  pabort("can't send spi message");
	  
	return (rx1[0]<<8)|(rx1[1]);
}



AD7705::AD7705() : mode(SPI_CPHA | SPI_CPOL), device("/dev/spidev0.0"), bits(8), speed(50000), dly(10)
{
	int ret = 0;

	int no_tty = !isatty( fileno(stdout) );

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	fprintf(stderr, "spi mode: %d\n", mode);
	fprintf(stderr, "bits per word: %d\n", bits);
	fprintf(stderr, "max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	// enable master clock for the AD
	// divisor results in roughly 4.9MHz
	gz_clock_ena(GZ_CLK_5MHz,5);

	// initialise channel 1
	initChannel(CHAN1);

}


AD7705::~AD7705()
{
	close(fd);
}

void AD7705::initChannel ( int channel ) {
	
	int data = 0x20 + channel;
	
	// tell the AD7705 that the next write will be to the clock register, set channel
	writeReg(fd, data, bits, speed, dly);
	// write 00001100 : CLOCKDIV=1,CLK=1,expects 4.9152MHz input clock
	writeReg(fd,0x0C, bits, speed, dly);

	// tell the AD7705 that the next write will be the setup register
	writeReg(fd,0x10, bits, speed, dly);
	// intiates a self calibration and then after that starts converting
	writeReg(fd,0x40, bits, speed, dly);
}


int AD7705::read(int channel)
{
	int d=0;
	int channelReady = 0x08 + channel;
	int readDataChannel = 0x38 + channel;
  
	// resets the AD7705 so that it expects a write to the communication register
	writeReset(fd, bits, speed, dly);
  
	do {
    	// tell the AD7705 to send back the communications register
		writeReg(fd, channelReady, bits, speed, dly);
    	// we get the communications register
		d = readReg(fd, speed, dly);
    	// loop while /DRDY is high
	} while ( d & 0x80 );
	  
  	// tell the AD7705 to read the data register (16 bits)
	writeReg(fd, readDataChannel, bits, speed, dly);
  	// read the data register by performing two 8 bit reads
	int value = readData(fd, speed, dly)-0x8000;
	return value;
}
