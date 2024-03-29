/*
 * spi.h
 *
 * @date 2019/08/09
 * @author Cosmin Tanislav
 */

#include <stdint.h>

#ifndef SPI_H
#define SPI_H

/*
 * Configuration for the SPI device.
 */
struct SpiDevice {
	char *filename; /**< Path of the SPI bus, eg: /dev/spidev1.0 */
	uint8_t mode; /**< Mode of the SPI bus */
	uint8_t bpw; /**< Bits-per-word of the SPI bus */
	uint32_t speed; /**< Speed of the SPI bus */

	int fd; /**< File descriptor for the SPI bus */
};

int spi_start(struct SpiDevice *dev);
//int spi_start(struct SpiDevice *dev, int i);
int spi_transfer(struct SpiDevice *dev, uint8_t *write_buf, uint8_t *read_buf, uint32_t buf_len);
void spi_stop(struct SpiDevice *dev);
void transfer(int fd);
void transfer_spi_dac(int fd, uint8_t *write_buf, uint32_t buf_len);

#endif // SPI_H
