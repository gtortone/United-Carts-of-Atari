#ifndef CARTRIDGE_IO_H
#define CARTRIDGE_IO_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/structs/sio.h"
#include "board.h"

#define SWCHA          0x280
#define SWCHB          0x282

#define RESET_ADDR addr = addr_prev = 0xffff;

// Used to control exit function
extern uint16_t EXIT_SWCHB_ADDR;

#define ADDR_GPIO_MASK  (0xFFF << PINROMADDR)
#define A12_GPIO_MASK   (0x1 << PINENABLE)
#define DATA_GPIO_MASK  (0xFF << PINROMDATA)

#define ADDR_IN (sio_hw->gpio_in & (ADDR_GPIO_MASK | A12_GPIO_MASK)) >> PINROMADDR
#define DATA_OUT(v) sio_hw->gpio_togl = (sio_hw->gpio_out ^ (v << PINROMDATA)) & DATA_GPIO_MASK
#define DATA_IN ((sio_hw->gpio_in & DATA_GPIO_MASK) >> PINROMDATA) & 0xFF
#define DATA_IN_BYTE DATA_IN

#define SET_DATA_MODE_IN    sio_hw->gpio_oe_clr = DATA_GPIO_MASK;
#define SET_DATA_MODE_OUT   sio_hw->gpio_oe_set = DATA_GPIO_MASK;
#define SET_DATA_MODE(m)    sio_hw->gpio_oe_togl = (sio_hw->gpio_oe ^ (m << PINROMDATA));

#endif // CARTRIDGE_IO_H
