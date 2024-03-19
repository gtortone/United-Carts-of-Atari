#ifndef CARTRIDGE_IO_H
#define CARTRIDGE_IO_H

#include <stdint.h>
#include "pico/stdlib.h"

#define SWCHB          0x282

#define PINROMADDR  2
#define PINENABLE  14
#define PINROMDATA 15

#define ADDRWIDTH 12
#define DATAWIDTH 8

const uint32_t addr_gpio_mask = (0xFFF << PINROMADDR);
const uint32_t a12_gpio_mask = (0x1 << PINENABLE);
const uint32_t data_gpio_mask = (0xFF << PINROMDATA);

#define ADDR_GPIO_MASK  addr_gpio_mask
#define A12_GPIO_MASK   a12_gpio_mask
#define DATA_GPIO_MASK  data_gpio_mask

#define ADDR_IN (gpio_get_all() & (ADDR_GPIO_MASK | A12_GPIO_MASK)) >> PINROMADDR
#define DATA_OUT(v) gpio_put_masked(DATA_GPIO_MASK, ((uint32_t)v << PINROMDATA))
#define DATA_IN ((gpio_get_all() & DATA_GPIO_MASK) >> PINROMDATA) & 0xFF
#define DATA_IN_BYTE DATA_IN

#define SET_DATA_MODE_IN    gpio_set_dir_in_masked(DATA_GPIO_MASK);
#define SET_DATA_MODE_OUT   gpio_set_dir_out_masked(DATA_GPIO_MASK);

// Used to control exit function
extern uint16_t EXIT_SWCHB_ADDR;

#endif // CARTRIDGE_IO_H
