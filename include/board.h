#ifndef __BOARD_H_
#define __BOARD_H_

void board_enable_5v(bool enable);
void board_enable_vdd_ext(bool enable);
void board_gpio_setup(void);
void board_suspend(void);

#endif // __BOARD_H_
