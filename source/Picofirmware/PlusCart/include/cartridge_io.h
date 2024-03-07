#ifndef CARTRIDGE_IO_H
#define CARTRIDGE_IO_H

#include <stdint.h>
#include "pico/stdlib.h"

#define SWCHB          0x282

#ifdef HARDWARE_TYPE
 #if HARDWARE_TYPE == UNOCART
   #include "stm32f4xx.h"
   #define CONTROL_IN GPIOC->IDR
 #endif
#endif

#define PINROMADDR  4
#define PINENABLE  16
#define PINROMDATA 17

#define ADDRWIDTH 12
#define DATAWIDTH 8

const uint32_t addr_gpio_mask = (0xFFF << PINROMADDR);
const uint32_t a12_gpio_mask = (0x1 << PINENABLE);
const uint32_t data_gpio_mask = (0xFF << PINROMDATA);

//FIXME
#define ADDR_GPIO_MASK  addr_gpio_mask
#define A12_GPIO_MASK   a12_gpio_mask
#define DATA_GPIO_MASK  data_gpio_mask
/*
#define ADDR_GPIO_MASK  (0xFFF << PINROMADDR)
#define A12_GPIO_MASK   (0x1 << PINENABLE)
#define DATA_GPIO_MASK  (0xFF << PINROMDATA)
*/

#define ADDR_IN (gpio_get_all() & (ADDR_GPIO_MASK | A12_GPIO_MASK)) >> PINROMADDR
#define DATA_OUT(v) gpio_put_masked(DATA_GPIO_MASK, ((uint32_t)v << PINROMDATA))
#define DATA_IN ((gpio_get_all() & DATA_GPIO_MASK) >> PINROMDATA) & 0xFF
#define DATA_IN_BYTE DATA_IN

//#define DATA_IN (*DATA_IDR)
//#define DATA_IN_BYTE (*DATA_IDR & 0xFF) // TODO deprecate this
//#define DATA_OUT *DATA_ODR

#define SET_DATA_MODE_IN    gpio_set_dir_in_masked(DATA_GPIO_MASK);
#define SET_DATA_MODE_OUT   gpio_set_dir_out_masked(DATA_GPIO_MASK);
//#define SET_DATA_MODE(m) *DATA_MODER = (m);

// Used to control exit function
extern uint16_t EXIT_SWCHB_ADDR;


#endif // CARTRIDGE_IO_H
