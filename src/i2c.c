#include <stdint.h>
#include <stdbool.h>
#include "nrf_gpio.h"
#include "nrf_twim.h"
#include "i2c.h"
#include "board.h"

static uint8_t last_address = 0;
typedef enum {
    WRITE = 1,
    READ,
    WRITE_READ
} op_t;
static op_t last_op = READ;

void i2c_init(int address, uint32_t scl, uint32_t sda) {
	last_address = (uint8_t)address;

	/* Platform-specific code to init comm bus */
	/* Disable I2C */
	nrf_twim_enable(I2C_DRV_TWI);

	/* Init SCL */
	nrf_gpio_pin_clear(scl);
	nrf_gpio_cfg(scl,
				 NRF_GPIO_PIN_DIR_INPUT,
				 NRF_GPIO_PIN_INPUT_CONNECT,
				 NRF_GPIO_PIN_NOPULL,
				 NRF_GPIO_PIN_S0D1,
				 NRF_GPIO_PIN_NOSENSE);
	/* Init SDO */
	nrf_gpio_pin_clear(sda);
	nrf_gpio_cfg(scl,
				 NRF_GPIO_PIN_DIR_INPUT,
				 NRF_GPIO_PIN_INPUT_CONNECT,
				 NRF_GPIO_PIN_NOPULL,
				 NRF_GPIO_PIN_S0D1,
				 NRF_GPIO_PIN_NOSENSE);

	/* Init TWIM peripheral */
	nrf_twim_pins_set(I2C_DRV_TWI,
		scl,
		sda);

    /* Set address */
    nrf_twim_address_set(NRF_TWIM1, address);

	/* Enable I2C */
	nrf_twim_enable(I2C_DRV_TWI);

	nrf_twim_frequency_set(I2C_DRV_TWI, NRF_TWIM_FREQ_100K);
}

void i2c_write_read(uint8_t address, uint8_t* txbuf, uint8_t txbytes, uint8_t* rxbuf, uint8_t rxbytes) {
	if(nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_ERROR))
		nrf_twim_errorsrc_get_and_clear(I2C_DRV_TWI);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_TXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_RXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_ERROR);

    if(last_op != WRITE_READ)
    {
        nrf_twim_disable(I2C_DRV_TWI);
        nrf_twim_shorts_set(I2C_DRV_TWI,
                            (NRF_TWIM_SHORT_LASTRX_STOP_MASK |
                            NRF_TWIM_SHORT_LASTTX_STARTRX_MASK));
    }
    if(address != last_address)
    {
        nrf_twim_address_set(I2C_DRV_TWI, address);
        last_address = address;
    }

	nrf_twim_tx_buffer_set(I2C_DRV_TWI, txbuf, txbytes);
	nrf_twim_rx_buffer_set(I2C_DRV_TWI, rxbuf, rxbytes);

    if(last_op != WRITE_READ)
    {
        nrf_twim_enable(I2C_DRV_TWI);
    }

	nrf_twim_task_trigger(I2C_DRV_TWI, NRF_TWIM_TASK_STARTTX);
	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_TXSTARTED));

	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED));
    last_op = WRITE_READ;
}

void i2c_write(uint8_t address, uint8_t* txbuf, uint8_t txbytes) {
	if(nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_ERROR))
		nrf_twim_errorsrc_get_and_clear(I2C_DRV_TWI);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_TXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_RXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED);

    if(last_op != WRITE)
    {
        nrf_twim_disable(I2C_DRV_TWI);
        nrf_twim_shorts_set(I2C_DRV_TWI, NRF_TWIM_SHORT_LASTTX_STOP_MASK);
    }

    if(address != last_address)
    {
        nrf_twim_address_set(I2C_DRV_TWI, address);
        last_address = address;
    }
	nrf_twim_tx_buffer_set(I2C_DRV_TWI, txbuf, txbytes);

    if(last_op != WRITE)
    {
        nrf_twim_enable(I2C_DRV_TWI);
    }

	nrf_twim_task_trigger(I2C_DRV_TWI, NRF_TWIM_TASK_STARTTX);
	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_TXSTARTED));

	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED));
    last_op = WRITE;
}

void i2c_read(uint8_t address, uint8_t* rxbuf, uint8_t rxbytes) {
	if(nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_ERROR))
		nrf_twim_errorsrc_get_and_clear(I2C_DRV_TWI);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_TXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_RXSTARTED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED);
	nrf_twim_event_clear(I2C_DRV_TWI, NRF_TWIM_EVENT_ERROR);

    if(last_op != READ)
    {
        nrf_twim_disable(I2C_DRV_TWI);
        nrf_twim_shorts_set(I2C_DRV_TWI, NRF_TWIM_SHORT_LASTRX_STOP_MASK);
    }

    if(address != last_address)
    {
        nrf_twim_address_set(I2C_DRV_TWI, address);
        last_address = address;
    }
	nrf_twim_rx_buffer_set(I2C_DRV_TWI, rxbuf, rxbytes);

    if(last_op != READ)
    {
        nrf_twim_enable(I2C_DRV_TWI);
    }

	nrf_twim_task_trigger(I2C_DRV_TWI, NRF_TWIM_TASK_STARTRX);
	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_RXSTARTED));

	while(!nrf_twim_event_check(I2C_DRV_TWI, NRF_TWIM_EVENT_STOPPED));
    last_op = READ;
}
