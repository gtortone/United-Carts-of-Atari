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
       
   return user_settings;
}

void flash_set_eeprom_user_settings(USER_SETTINGS user_settings){

   EEPROM.write(TV_MODE_EEPROM_ADDR, user_settings.tv_mode);
   EEPROM.write(FONT_STYLE_EEPROM_ADDR, user_settings.font_style);
   EEPROM.write(LINE_SPACING_EEPROM_ADDR, user_settings.line_spacing);
   EEPROM.commit();
}


void flash_erase_eeprom(){

   USER_SETTINGS user_settings = {TV_MODE_DEFAULT, FONT_DEFAULT, SPACING_DEFAULT};

   EEPROM.write(TV_MODE_EEPROM_ADDR, user_settings.tv_mode);
   EEPROM.write(FONT_STYLE_EEPROM_ADDR, user_settings.font_style);
   EEPROM.write(LINE_SPACING_EEPROM_ADDR, user_settings.line_spacing);
   EEPROM.commit();
}

void flash_erase_storage(void) {

   LittleFS.format();
}

#if 0

#if USE_WIFI
/* write to flash with multiple HTTP range requests */
uint32_t flash_download(char *filename, uint32_t download_size, uint32_t http_range_start, bool append){
	uint8_t start_sector;
	if(append)
		start_sector = user_settings.first_free_flash_sector;
	else
		start_sector = (uint8_t)FLASH_SECTOR_5;

	if( start_sector < (uint8_t)FLASH_SECTOR_5){
    	return 0;
	}

    flash_erase_storage(start_sector);

	uint32_t address = DOWNLOAD_AREA_START_ADDRESS + 128U * 1024U * (uint8_t)( start_sector - 5);
    flash_download_at(filename, download_size, http_range_start, (uint8_t*)address);

    // flash new usersettings .. (if not appended)
	if(! append){
		user_settings.first_free_flash_sector = (uint8_t)(get_sector(address+download_size) + 1);
    	flash_set_eeprom_user_settings(user_settings);
	}

	return address;
}

void flash_download_at(char *filename, uint32_t download_size, uint32_t http_range_start, uint8_t* flash_address){
	if( esp8266_PlusStore_API_connect() == false){
    	return;
	}


    __disable_irq();
	HAL_FLASH_Unlock();

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

    uint8_t c;
    uint32_t count, http_range_end = http_range_start + (download_size < MAX_RANGE_SIZE ? download_size : MAX_RANGE_SIZE) - 1;

	esp8266_PlusStore_API_prepare_request_header((char *)filename, true );
	strcat(http_request_header, (char *)"     0- 00000\r\n\r\n");
    size_t http_range_param_pos_counter, http_range_param_pos = strlen((char *)http_request_header) - 5;

    uint8_t parts = (uint8_t)(( download_size + MAX_RANGE_SIZE - 1 )  / MAX_RANGE_SIZE);
    uint16_t last_part_size = (download_size % MAX_RANGE_SIZE)?(download_size % MAX_RANGE_SIZE):MAX_RANGE_SIZE;

    while(parts != 0 ){
        http_range_param_pos_counter = http_range_param_pos;
        count = http_range_end;
        while(count != 0) {
            http_request_header[http_range_param_pos_counter--] = (char)(( count % 10 ) + '0');
            count = count/10;
        }
        http_range_param_pos_counter = http_range_param_pos - 7;
        count = http_range_start;
        while(count != 0) {
            http_request_header[http_range_param_pos_counter--] = (char)(( count % 10 ) + '0');
            count = count/10;
        }

    	esp8266_print(http_request_header);

        // Skip HTTP Header
        esp8266_skip_http_response_header();

        // Now for the HTTP Body
        count = 0;
        while(count < MAX_RANGE_SIZE && (parts != 1 || count < last_part_size )){
            if(( huart1.Instance->SR & UART_FLAG_RXNE) == UART_FLAG_RXNE){ // ! (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) ) ){
                c = (uint8_t)huart1.Instance->DR; // & (uint8_t)0xFF);

//HAL_FLASH_Program();
                /* Program the user Flash area byte by byte
                (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
                /* Wait for last operation to be completed */
                //if(FLASH_WaitInRAMForLastOperationWithMaxDelay() == HAL_OK){
                FLASH_WaitInRAMForLastOperationWithMaxDelay() ;
                    /*Program byte (8-bit) at a specified address.*/
                    // FLASH_Program_Byte(Address, (uint8_t) c);
                    CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
                    FLASH->CR |= FLASH_PSIZE_BYTE;
                    FLASH->CR |= FLASH_CR_PG;

                    *(uint8_t*)flash_address = c;
                    // end FLASH_Program_Byte(Address, (uint8_t) c);

                    /* Wait for last operation to be completed */
                    FLASH_WaitInRAMForLastOperationWithMaxDelay();

                    /* If the program operation is completed, disable the PG Bit */
                    FLASH->CR &= (~FLASH_CR_PG);
                    flash_address++;
                    // end HAL_FLASH_Program
               // }else{
                //    return;
               // }
                count++;
            }
        }

        http_range_start += MAX_RANGE_SIZE;
        http_range_end += (--parts==1)?last_part_size:MAX_RANGE_SIZE;

        count = 0;
        while(count++ < 25000000){
        }
    }
    __HAL_UNLOCK(&pFlash);
    HAL_FLASH_Lock();

    __enable_irq();

    // End Transparent Transmission
    esp8266_PlusStore_API_end_transmission();

}
#endif

void flash_buffer_at(uint8_t* buffer, uint32_t buffer_size, uint8_t* flash_address)
{
	   __disable_irq();
		HAL_FLASH_Unlock();

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


		for(int i = 0; i < buffer_size; i++)
		{
			//HAL_FLASH_Program();
			/* Program the user Flash area byte by byte
			(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
			/* Wait for last operation to be completed */
			//if(FLASH_WaitInRAMForLastOperationWithMaxDelay() == HAL_OK){
			FLASH_WaitInRAMForLastOperationWithMaxDelay() ;
			/*Program byte (8-bit) at a specified address.*/
			// FLASH_Program_Byte(Address, (uint8_t) c);
			CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
			FLASH->CR |= FLASH_PSIZE_BYTE;
			FLASH->CR |= FLASH_CR_PG;

			*(uint8_t*)flash_address = buffer[i];
			// end FLASH_Program_Byte(Address, (uint8_t) c);

			/* Wait for last operation to be completed */
			FLASH_WaitInRAMForLastOperationWithMaxDelay();

			/* If the program operation is completed, disable the PG Bit */
			FLASH->CR &= (~FLASH_CR_PG);
			flash_address++;
			// end HAL_FLASH_Program
	    }
	    __HAL_UNLOCK(&pFlash);
	    HAL_FLASH_Lock();

	    __enable_irq();
}

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

uint32_t flash_file_request(uint8_t *ext_buffer, char *path, uint32_t start, uint32_t length){

   const char *filename = 0;
   filename = &path[sizeof(MENU_TEXT_OFFLINE_ROMS)];

   File f = LittleFS.open(filename, "r");
   
   f.seek(start, SeekSet);
   uint32_t bytes = f.readBytes((char *)ext_buffer, length);

   //dbgSerial.printf("filename: %s, bytes read: %d\n\r", filename, bytes);

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

