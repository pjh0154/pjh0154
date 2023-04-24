/*
 * spi.c
 *
 * @date 2019/08/09
 * @author Cosmin Tanislav
 */

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ep.h"

#include "../include/spi.h"

/*
 * Start the SPI device.
 *
 * @param dev points to the SPI device to be started, must have filename,
 *  mode, bpw and speed populated.
 *
 * @return - 0 if the starting procedure succeeded
 *         - negative if the starting procedure failed
 */
int spi_start(struct SpiDevice *dev) {
//int spi_start(struct SpiDevice *dev, int i) {
	int fd;
	int rc;

	fd = open(dev->filename, O_RDWR);
	if (fd < 0) {
		printf("%s: failed to start SPI\r\n", __func__);
		goto fail_open;
	}
	/*
	 * Set the given SPI mode.
	 */
	rc = ioctl(fd, SPI_IOC_WR_MODE, &dev->mode);
	if (rc < 0) {
		printf("%s: failed to set SPI write mode\r\n", __func__);
		goto fail_configure;
	}
	rc = ioctl(fd, SPI_IOC_RD_MODE, &dev->mode);
	if (rc < 0) {
		printf("%s: failed to set SPI read mode\r\n", __func__);
		goto fail_configure;
	}
	/*
	 * Set the given SPI bits-per-word.
	 */
	rc = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &dev->bpw);
	if (rc < 0) {
		printf("%s: failed to set SPI write bits-per-word\r\n", __func__);

		goto fail_configure;
	}
	rc = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &dev->bpw);
	if (rc < 0) {
		printf("%s: failed to set SPI read bits-per-word\r\n", __func__);
		goto fail_configure;
	}
	/*
	 * Set the given SPI speed.
	 */
	rc = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &dev->speed);
	if (rc < 0) {
		printf("%s: failed to set SPI write speed\r\n", __func__);
		goto fail_configure;
	}
	rc = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &dev->speed);
	if (rc < 0) {
		printf("%s: failed to set SPI read speed\r\n", __func__);
		goto fail_configure;
	}
	dev->fd = fd;
	return 0;

fail_configure:
	close(fd);
fail_open:
	return rc;
}

/*
 * Transfer data with the SPI device.
 *
 * @param dev points to the I2C device to be read from
 * @param write_buf points to the start of the buffer to be written from
 * @param read_buf points to the start of the buffer to be read into
 * @param buf_len length of the buffers
 *
 * @return - 0 if the read procedure succeeded
 *         - negative if the read procedure failed
 */
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 20000000;
static uint16_t delay;

int spi_transfer(struct SpiDevice *dev, uint8_t *write_buf, uint8_t *read_buf, uint32_t buf_len) {
	struct spi_ioc_transfer transfer;
	int rc;	
	uint32_t read_buff = 0;

	memset(&transfer, 0, sizeof(transfer));
	/*transfer.tx_buf = (uint32_t)write_buf;
	transfer.rx_buf = (uint32_t)read_buf;
	transfer.len = buf_len;
	transfer.speed_hz = dev->speed;
	transfer.bits_per_word = dev->bpw;
	transfer.cs_change = 1;*/
	transfer.tx_buf = 0xaaaaaa;
	transfer.rx_buf = (uint32_t)read_buff;
	transfer.len = 24;
	transfer.speed_hz = speed;
	transfer.bits_per_word = bits;
	transfer.cs_change = 1;	

	usleep(10);
	printf("transfer_2 = %x / %x / %x / %x / %x\r\n", transfer.tx_buf,transfer.rx_buf,transfer.len,transfer.speed_hz,transfer.bits_per_word);

	rc = ioctl(dev->fd, SPI_IOC_MESSAGE(1), &transfer);
	if (rc < 0) {
		printf("%s: failed to start SPI transfer\r\n", __func__);
	}

	return rc;
}

void transfer(int fd)
{
	int ret;
	uint8_t tx[] = {
		0xaa, 0xaa, 0xaa,
	};
	uint32_t rx_data;
	struct spi_ioc_transfer tr;
	
	memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (unsigned long)tx;
	tr.rx_buf = (unsigned long)rx_data;
	tr.len = 3;
	tr.delay_usecs = 0;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
	tr.cs_change = 0;
	printf("1fd = %d\r\n", fd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message");
}

void transfer_spi_dac(int fd, uint8_t *write_buf, uint32_t buf_len)
{
	int ret;
	struct spi_ioc_transfer tr;
	
	memset(&tr, 0, sizeof(tr));
	tr.tx_buf = (uint32_t)write_buf;
	tr.len = buf_len;
	tr.delay_usecs = 0;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
	tr.cs_change = 0;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message");
}

/*
 * Stop the SPI device.
 *
 * @param dev points to the SPI device to be stopped to
 */
void spi_stop(struct SpiDevice *dev) {
	close(dev->fd);
}
