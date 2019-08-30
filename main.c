#include "nrf24l01+.h"
#include "spi_dev.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/spi/spidev.h>
#include <time.h>

struct nrf24l01_conf radio;
struct spi_dev spi;
int error;
time_t timestamp;
char rx_buf[100];
int expire = 5, num = 0;

uint8_t write_reg(uint8_t reg, uint8_t *buf, uint8_t size)
{
	uint8_t i;

	/*printf("Write to reg 0x%2.2X:\t", reg);
	for (i = 0; i != size; i++)
		printf("0x%2.2X ", buf[i]);
	printf("\n");*/

	error = spi.send_reg(reg, buf, size);
	return 0;
}

uint8_t read_reg (uint8_t reg, uint8_t *buf, uint8_t size)
{
	uint8_t i;

	/*printf("Read from reg 0x%2.2X:\t", reg);
	for (i = 0; i != size; i++)
		printf("0x%2.2X", buf[i]);
	printf("\n");*/

	error = spi.recv_reg(reg, buf, size);
	return 0;
}

void radio_en (bool state)
{
	printf("Radio %s\n", state ? "enabled" : "disabled");
}

int main(void)
{
	spi.dbg = printf;
	spi.freq = SPI_FREQ_MAX_HZ;
	spi.bits_per_word = SPI_BITS_PER_WORD;
	spi.mode = SPI_MODE;

	if (!spi_init(&spi, NULL)) {
		printf("Fail to open SPI\n");
		return 1;
	}

	printf("SPI: %s\n", spi.name);
	{
		uint8_t config;
		read_reg(0x07, &config, 1);
		printf("Config reg: 0x%02X\n", config);
	}

	memset(&radio, 0, sizeof(radio));

	/* config */
	radio.config.mode = RADIO_RX;
	radio.config.power_enable = true;
	radio.config.crc_config = CRC_1B;
	radio.config.crc_enable = true;
	radio.config.rx_irq = true;
	radio.enabled_rx_addresses.pipe0 = true;
	radio.enabled_rx_addresses.pipe1 = true;
	radio.enabled_rx_addresses.pipe2 = true;
	radio.enabled_rx_addresses.pipe3 = true;
	radio.enabled_rx_addresses.pipe4 = true;
	radio.enabled_rx_addresses.pipe5 = true;
	radio.address_widths.address_len = ADDRESS_5_BYTES;
	radio.channel = 72;
	radio.setup.data_rate = R_2MPS;
	radio.setup.power = P_0dBm;

	/* pipes */
	radio.rx_address_p0 = (uint8_t *)"HALLO";
	radio.tx_address = radio.rx_address_p0;
	radio.rx_pipe_size[0] = 32;
	radio.rx_pipe_size[1] = 32;
	radio.rx_pipe_size[2] = 32;
	radio.rx_pipe_size[3] = 32;
	radio.rx_pipe_size[4] = 32;
	radio.rx_pipe_size[5] = 32;
	radio.read_cnt = 1000000;

	radio.interface.write = write_reg;
	radio.interface.read = read_reg;
	radio.interface.radio_en = radio_en;

	printf("Status reg: %d\n", error);
	nrf24l01_init(&radio);

	timestamp = time(NULL);

	do {
		if (radio.recv(rx_buf, NULL)) {
			//uint8_t i;
			if (rx_buf[0])
				printf("Data: %s\n", rx_buf);
			num++;
			/*for (i = 0; i != 32; i++)
				printf("0x%02X ", rx_buf[i]);
			printf("\n");*/
		}
	} while (time(NULL) - timestamp < expire);

	spi_deinit();

	printf("Transfer speed: %0.0f pps\n", (float)num / (float)expire);
	return 0;
}
