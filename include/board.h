#ifndef __BOARD_H_
#define __BOARD_H_

#include "nrf_gpio.h"
#include "nrf_ppi.h"

/* General defines */
/* #define PLATFORM_832_DK */
#define PLATFORM_WATCH
#define RGB_LED_MAX_NUM 100
#define NRF_GPIO_NC 0xFFFFFFFF

/**************** Peripherals ****************/
#define LED_DRV_SPI     NRF_SPIM0
#define LED_DRV_COUNTER NRF_TIMER1
#define CLOCK_RTC       NRF_RTC2
#define I2C_DRV_TWI     NRF_TWIM1

/**************** Priorities ****************/
#define CLOCK_IRQ_PRIO  6
#define CLOCK_IRQ_NUM   RTC2_IRQn
#define GPIOTE_IRQ_PRIO 6

/**************** PPI channels ****************/
#define LED_DRV_PPI_0 NRF_PPI_CHANNEL5
#define LED_DRV_PPI_1 NRF_PPI_CHANNEL6
#define RTC_DRV_PPI   NRF_PPI_CHANNEL7

/**************** Pinout ****************/
#ifdef PLATFORM_WATCH

/* Connector GPIOs */
#define PIN_SENS_SDA NRF_GPIO_PIN_MAP(0, 13)
#define PIN_SENS_SCL NRF_GPIO_PIN_MAP(0, 15)

#define PIN_EXT_MOSI NRF_GPIO_PIN_MAP(0, 20)
#define PIN_EXT_MISO NRF_GPIO_PIN_MAP(0, 22)
#define PIN_EXT_SCK  NRF_GPIO_PIN_MAP(0, 24)

#define PIN_EXT_AIN0_IO2 NRF_GPIO_PIN_MAP(0, 31)
#define PIN_EXT_AIN1_IO3 NRF_GPIO_PIN_MAP(0, 29)
#define PIN_EXT_AIN2_IO4 NRF_GPIO_PIN_MAP(0, 2)

#define PIN_EXT_IO0 NRF_GPIO_PIN_MAP(1, 0)
#define PIN_EXT_IO1 NRF_GPIO_PIN_MAP(1, 2)
#define PIN_EXT_IO6 NRF_GPIO_NC

/* Internal GPIOs */
#define PIN_CHG_ACTIVE NRF_GPIO_PIN_MAP(0, 17)
#define PIN_BATMON_EN  NRF_GPIO_PIN_MAP(0, 4)
#define PIN_BATMON_OUT EXT_AIN0_IO2

#define PIN_SENS_EN    NRF_GPIO_PIN_MAP(1, 9)
#define PIN_ACC_INT1   NRF_GPIO_PIN_MAP(0, 6)
#define PIN_ACC_INT2   NRF_GPIO_PIN_MAP(0, 8)
/* LSB of accel i2c address */
#define PIN_ACC_SA0_LVL 1

#define PIN_SW_0       NRF_GPIO_PIN_MAP(1, 13)
#define PIN_SW_1       NRF_GPIO_PIN_MAP(1, 10)
#define PIN_SW_2       NRF_GPIO_PIN_MAP(1, 4)

#define PIN_HAPT_PWM   NRF_GPIO_PIN_MAP(1, 15)

/* These are not connected to anything on the PCB */
#define PIN_DBG_LED_0  NRF_GPIO_PIN_MAP(0, 25)
#define PIN_DBG_LED_1  NRF_GPIO_PIN_MAP(0, 26)
#define PIN_DBG_LED_2  NRF_GPIO_PIN_MAP(0, 27)
#define PIN_DBG_LED_3  NRF_GPIO_PIN_MAP(0, 23)

/* Accelerometer Y is inverted */
#define ACCEL_Y_ORIENTATION (-1)

#endif  /* PLATFORM_WATCH */

#ifdef PLATFORM_832_DK

/* Connector GPIOs */
#define PIN_SENS_SDA 27
#define PIN_SENS_SCL 26

#define PIN_EXT_MOSI 24
#define PIN_EXT_MISO 23
#define PIN_EXT_SCK  25

#define PIN_EXT_AIN0_IO2 NRF_GPIO_NC
#define PIN_EXT_AIN1_IO3 NRF_GPIO_NC
#define PIN_EXT_AIN2_IO4 NRF_GPIO_NC

#define PIN_EXT_IO0 22
#define PIN_EXT_IO1 NRF_GPIO_NC
#define PIN_EXT_IO6 NRF_GPIO_NC

/* Internal GPIOs */
#define PIN_CHG_ACTIVE NRF_GPIO_NC
#define PIN_BATMON_EN  NRF_GPIO_NC
#define PIN_BATMON_OUT NRF_GPIO_NC

#define PIN_SENS_EN    NRF_GPIO_NC
#define PIN_ACC_INT1   11
#define PIN_ACC_INT2   12
/* LSB of accel i2c address */
#define PIN_ACC_SA0_LVL 0

/* Devkit buttons */
#define PIN_SW_0       13
#define PIN_SW_1       14
#define PIN_SW_2       15

#define PIN_HAPT_PWM   NRF_GPIO_NC

/* Devkit leds */
#define PIN_DBG_LED_0    17
#define PIN_DBG_LED_1    18
#define PIN_DBG_LED_2    19
#define PIN_DBG_LED_3    20

#endif

/* Front-panel connector assignment */
#define PIN_FP_MOSI  PIN_EXT_MOSI
#define PIN_FP_SCK   PIN_EXT_SCK

#define PIN_FP_SCL   PIN_SENS_SCL
#define PIN_FP_SDA   PIN_SENS_SDA
#define PIN_FP_5V_EN PIN_EXT_IO0
#define PIN_FP_INT   PIN_EXT_MISO

/* Devkit buttons */
#define ADVERTISING_LED  PIN_DBG_LED_0 /**< Is on when device is advertising. */
#define CONNECTED_LED    PIN_DBG_LED_1 /**< Is on when device has connected. */
#define LEDBUTTON_LED    PIN_DBG_LED_2 /**< LED to be toggled with the help of the LED Button Service. */
#define DEBUG_LED        PIN_DBG_LED_3 /**< Used to debug loops n stuff */

/* Devkit leds */
#define LED_BUTTON     PIN_SW_0    /**< Button that will trigger the notification event with the LED Button Service */
#define ANIMATE_BUTTON PIN_SW_1    /**< Button that will trigger a simple animation */
#define MENU_BUTTON    PIN_SW_2    /**< Button that will trigger a simple animation */

/**************** Accelerometer-related utility functions ****************/
void acc_enable_orientation();
void acc_disable_orientation();
void acc_enable_hpf();
void acc_disable_hpf();
void acc_enable_click();
void acc_disable_4D();
void acc_enable_4D();
void acc_app_init();

/**************** Debug-related utility functions ****************/
void dbg_led_init();
void dbg_led_on(uint32_t led);
void dbg_led_off(uint32_t led);

#endif // __BOARD_H_
