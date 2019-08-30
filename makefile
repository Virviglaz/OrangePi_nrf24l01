all: nrf_driver
FLAGS=all
CC=gcc

.PHONY=all clean

access:
	sudo chmod 777 /dev/spidev0.0
clean:
	rm *.o
nrf24l01: nrf24l01+.c
	${CC} -c -o nrf24l01+.o nrf24l01+.c -W${FLAGS}
spi_dev: spi_dev.c
	${CC} -c -o spi_dev.o spi_dev.c -W${FLAGS}
main: main.c
	${CC} -c -o main.o main.c -W${FLAGS}
nrf_driver: nrf24l01+.o spi_dev.o main.o
	${CC} -o nrf nrf24l01+.o spi_dev.o main.o
