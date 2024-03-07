#ifndef FLASH_H
#define	FLASH_H

#include <stdint.h>
#include "hardware/flash.h"
#include "global.h"
#include "LittleFS.h"
#include <EEPROM.h>

#define TV_MODE_EEPROM_ADDR      0x00
#define FONT_STYLE_EEPROM_ADDR   0x01
#define LINE_SPACING_EEPROM_ADDR 0x02

/*
void flash_firmware_update(uint32_t)__attribute__((section(".data#")));

uint32_t flash_download(char *, uint32_t , uint32_t , bool );
void flash_download_at(char *filename, uint32_t download_size, uint32_t file_offset, uint8_t* flash_address);
void flash_buffer_at(uint8_t* buffer, uint32_t buffer_size, uint8_t* flash_address);
uint32_t flash_check_offline_roms_size(void);
*/

uint32_t flash_file_request( uint8_t *, char *, uint32_t, uint32_t );
bool flash_has_downloaded_roms(void);
void flash_file_list(char *, MENU_ENTRY **, int *);

USER_SETTINGS flash_get_eeprom_user_settings(void);
void flash_set_eeprom_user_settings(USER_SETTINGS);
void flash_erase_eeprom(void);
void flash_erase_storage(void);

#endif	/* FLASH_H */
