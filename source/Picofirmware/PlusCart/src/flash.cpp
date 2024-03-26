#include <string.h>
#include <stdlib.h>
#include "global.h"
#if USE_WIFI
#include "esp8266.h"
#endif
#include "flash.h"
#include "cartridge_firmware.h"

USER_SETTINGS flash_get_eeprom_user_settings(void){

   USER_SETTINGS user_settings = {TV_MODE_DEFAULT, FONT_DEFAULT, SPACING_DEFAULT};

   user_settings.tv_mode = EEPROM.read(TV_MODE_EEPROM_ADDR);
   user_settings.font_style = EEPROM.read(FONT_STYLE_EEPROM_ADDR);
   user_settings.line_spacing =  EEPROM.read(LINE_SPACING_EEPROM_ADDR);
   user_settings.first_free_flash_sector = EEPROM.read(FREE_SECTOR_EEPROM_ADDR);
       
   return user_settings;
}

void flash_set_eeprom_user_settings(USER_SETTINGS user_settings){

   EEPROM.write(TV_MODE_EEPROM_ADDR, user_settings.tv_mode);
   EEPROM.write(FONT_STYLE_EEPROM_ADDR, user_settings.font_style);
   EEPROM.write(LINE_SPACING_EEPROM_ADDR, user_settings.line_spacing);
   EEPROM.write(FREE_SECTOR_EEPROM_ADDR, user_settings.first_free_flash_sector);
   EEPROM.commit();
}

void flash_erase_eeprom(){

   USER_SETTINGS user_settings = {TV_MODE_DEFAULT, FONT_DEFAULT, SPACING_DEFAULT, 0};

   EEPROM.write(TV_MODE_EEPROM_ADDR, user_settings.tv_mode);
   EEPROM.write(FONT_STYLE_EEPROM_ADDR, user_settings.font_style);
   EEPROM.write(LINE_SPACING_EEPROM_ADDR, user_settings.line_spacing);
   EEPROM.write(FREE_SECTOR_EEPROM_ADDR, user_settings.first_free_flash_sector);
   EEPROM.commit();
}

void flash_erase_storage(void) {

   LittleFS.format();
}

void flash_erase_storage(uint32_t start_addr, uint32_t end_addr) {

   flash_range_erase(start_addr, (end_addr - start_addr));
}

#if USE_WIFI

/* write to flash with multiple HTTP range requests */
uint32_t flash_download(char *filename, uint32_t download_size, uint32_t http_range_start) {

	uint32_t start_address, end_address;
   uint8_t start_sector, end_sector;

   //FIXME
   //add (basic) flash wear leveling strategy
   //start_sector = user_settings.first_free_flash_sector;
   start_sector = 0;
   end_sector = start_sector + (download_size/4096);

   start_address = FLASH_AREA_OFFSET + (start_sector * 4096);
   end_address = FLASH_AREA_OFFSET + (end_sector * 4096);

	uint32_t address = FLASH_AREA_OFFSET + (start_sector * 4096);

   uint32_t irqstatus = save_and_disable_interrupts();
      flash_erase_storage(start_address, end_address);
   restore_interrupts(irqstatus);

   flash_download_at(filename, download_size, http_range_start, address);

   // flash new usersettings
   user_settings.first_free_flash_sector = end_sector + 1;
   flash_set_eeprom_user_settings(user_settings);

	return address;
}

void flash_download_at(char *filename, uint32_t download_size, uint32_t http_range_start, uint32_t flash_address) {

   WiFiClient plusstore; 

   if(!plusstore.connect(PLUSSTORE_API_HOST, 80))
    	return;

   uint8_t c;
   uint32_t count;
   uint32_t http_range_end = http_range_start + (download_size < MAX_RANGE_SIZE ? download_size : MAX_RANGE_SIZE) - 1;

   size_t http_range_param_pos_counter, http_range_param_pos = strlen((char *)http_request_header) - 5;

   uint8_t parts = (uint8_t)(( download_size + MAX_RANGE_SIZE - 1 ) / MAX_RANGE_SIZE);
   uint16_t last_part_size = (download_size % MAX_RANGE_SIZE)?(download_size % MAX_RANGE_SIZE):MAX_RANGE_SIZE;
   char range_str[24];

   while(parts != 0 ) {

      esp8266_PlusStore_API_prepare_request_header((char *)filename, true);

      sprintf(range_str, "%lu-%lu", http_range_start, http_range_end);

      strcat(http_request_header, range_str);
      strcat(http_request_header, "\r\n\r\n");

      plusstore.write(http_request_header, strlen(http_request_header));
      plusstore.flush();
      esp8266_skip_http_response_header(&plusstore);

      // Now for the HTTP Body
      count = 0;
      uint8_t buf[FLASH_PAGE_SIZE];
      while(count < MAX_RANGE_SIZE && (parts != 1 || count < last_part_size )){

         int len = plusstore.readBytes(buf, FLASH_PAGE_SIZE);   
         
         uint32_t irqstatus = save_and_disable_interrupts();
            flash_range_program(flash_address, buf, len);
         restore_interrupts(irqstatus);

         flash_address += len;
         count += len;
      }

      http_range_start += MAX_RANGE_SIZE;
      http_range_end += (--parts==1)?last_part_size:MAX_RANGE_SIZE;
    }

   plusstore.stop();
}

//FIXME
void flash_buffer_at(uint8_t* buffer, uint32_t buffer_size, uint8_t* flash_address) {

	   __disable_irq();

		for(int i=0; i<buffer_size; i++)
		{
			*(uint8_t*)flash_address = buffer[i];

			flash_address++;
	    }

	    __enable_irq();
}

#if 0

/* write (firmware) to flash from buffer */
void flash_firmware_update(uint32_t filesize){

    uint32_t count;
    uint32_t Address = ADDR_FLASH_SECTOR_0;
    HAL_StatusTypeDef status;


    //HAL_FLASHEx_Erase();
    // Process Locked
    // __HAL_LOCK(&pFlash);
    pFlash.Lock = HAL_LOCKED;

    // Wait for last operation to be completed
    if(FLASH_WaitInRAMForLastOperationWithMaxDelay() == HAL_OK){
        uint32_t sectors[4] = { FLASH_SECTOR_0, FLASH_SECTOR_2, FLASH_SECTOR_3, FLASH_SECTOR_4 };

        for( count = 0 ; count < 4; count++){
//          FLASH_Erase_Sector(count, (uint8_t) FLASH_VOLTAGE_RANGE_3);
            CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
            FLASH->CR |= FLASH_PSIZE_WORD;
            CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
            FLASH->CR |= FLASH_CR_SER | (sectors[count] << FLASH_CR_SNB_Pos);
            FLASH->CR |= FLASH_CR_STRT;

            /* Wait for last operation to be completed */
            status = FLASH_WaitInRAMForLastOperationWithMaxDelay();

            /* If the erase operation is completed, disable the SER and SNB Bits */
            CLEAR_BIT(FLASH->CR, (FLASH_CR_SER | FLASH_CR_SNB));

            if(status != HAL_OK){
                /* In case of error, stop erase procedure and return the faulty sector*/
                // break; Todo wat nu
            }
        }
    }else{
        return; // or try flashing anyway ??
    }

    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
    //end HAL_FLASHEx_Erase();

    /* Flush the caches to be sure of the data consistency */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

    __HAL_FLASH_DATA_CACHE_RESET();
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();


    //__HAL_LOCK(&pFlash);

    pFlash.Lock = HAL_LOCKED;
    FLASH_WaitInRAMForLastOperationWithMaxDelay();


    uint8_t* data_pointer = buffer;
    count = 0;
    while(count < filesize ){
        //HAL_FLASH_Program();
        /* Program the user Flash area byte by byte
         (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
        /* Wait for last operation to be completed */
        FLASH_WaitInRAMForLastOperationWithMaxDelay() ;
        /*Program byte (8-bit) at a specified address.*/
        // FLASH_Program_Byte(Address, (uint8_t) c);
        CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
        FLASH->CR |= FLASH_PSIZE_BYTE;
        FLASH->CR |= FLASH_CR_PG;

        *(uint8_t*)Address = data_pointer[count];
        // end FLASH_Program_Byte(Address, (uint8_t) c);

        /* Wait for last operation to be completed */
        FLASH_WaitInRAMForLastOperationWithMaxDelay();

        /* If the program operation is completed, disable the PG Bit */
        FLASH->CR &= (~FLASH_CR_PG);
        Address++;
        count++;
        if( Address == ADDR_FLASH_SECTOR_1){
        	Address = ADDR_FLASH_SECTOR_2; // Skip user settings area
        } else if(Address == ( ADDR_FLASH_SECTOR_4 + 48 * 1024 ) ){
        	data_pointer = ((uint8_t*)0x10000000) - 96 * 1024 ;
        }
    }
    __HAL_UNLOCK(&pFlash);

    __enable_irq();
    NVIC_SystemReset();
}
#endif
#endif

uint32_t flash_file_request(uint8_t *ext_buffer, char *path, uint32_t start, uint32_t length){

   const char *filename = 0;
   filename = &path[sizeof(MENU_TEXT_OFFLINE_ROMS)];

   File f = LittleFS.open(filename, "r");
   
   f.seek(start, SeekSet);
   uint32_t bytes = f.readBytes((char *)ext_buffer, length);

   f.close();

   return bytes;
}

bool flash_has_downloaded_roms(){
   return LittleFS.begin();
}

void flash_file_list(char *path, MENU_ENTRY **dst, int *num_menu_entries ) {

   Dir dir = LittleFS.openDir(path);

   while(dir.next() && (*num_menu_entries) < NUM_MENU_ITEMS) {
      if (dir.isFile() || dir.isDirectory()) {   // ignore hard/symbolic link, (block) device files and named pipes
         (*dst)->entryname[0] = '\0';
         strncat((*dst)->entryname, dir.fileName().c_str(), 32);
         (*dst)->type = dir.isDirectory()?Offline_Sub_Menu:Offline_Cart_File;
         (*dst)->filesize = dir.fileSize();

         (*dst)++;
         (*num_menu_entries)++;
      }
   }
}
