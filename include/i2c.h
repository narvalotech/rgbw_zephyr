#ifndef __I2C_H_
#define __I2C_H_

#include <stdbool.h>
#include <stdint.h>

// I2C peripherial usage
void i2c_init(int address, uint32_t scl, uint32_t sda);
void i2c_read(uint8_t address, uint8_t* rxbuf, uint8_t rxbytes);
void i2c_write(uint8_t address, uint8_t* txbuf, uint8_t txbytes);
void i2c_write_read(uint8_t address, uint8_t* txbuf, uint8_t txbytes, uint8_t* rxbuf, uint8_t rxbytes);

#endif // __I2C_H_
