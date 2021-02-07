/*
 * cartridge_setup.c
 *
 *  Created on: 05.07.2020
 *      Author: stubig
 */

#include "global.h"
#if USE_WIFI
#include "esp8266.h"
#endif
#include "flash.h"

#include "cartridge_setup.h"

#define BUFFER_SIZE_KB 96
#define CCM_SIZE_KB 64

#define RAM_BANKS (BUFFER_SIZE_KB / 4 )          //    24
#define CCM_BANKS (CCM_SIZE_KB / 4)              //    16
#define FLASH_BANKS (64 - RAM_BANKS - CCM_BANKS) //    24


#define CCM_RAM ((uint8_t*)0x10000000)
#define CCM_SIZE (CCM_SIZE_KB * 1024)

#define CCM_IMAGE_OFFSET (RAM_BANKS * 4096)      // 98304
#define CCM_IMAGE_SIZE (CCM_BANKS * 4096)        // 26864

#define FLASH_IMAGE_SIZE (FLASH_BANKS * 4096)
#define FLASH_IMAGE_OFFSET ((RAM_BANKS + CCM_BANKS) * 4096)


bool setup_cartridge_image(const char* filename, uint32_t image_size, uint8_t* buffer, cartridge_layout* layout, MENU_ENTRY *d, enum cart_base_type banking_type) {

    switch(banking_type){
        case(base_type_SB):
            if (image_size > 256*1024) return false;
        break;
        case(base_type_DF):
        case(base_type_DFSC):
            if (image_size != 128*1024) return false;
        break;
        case(base_type_BF):
        case(base_type_BFSC):
            if (image_size != 256*1024) return false;
        break;
        // these base types will never appear here, it is just to stop
        // the compiler from nagging!
        case base_type_None:
        case base_type_2K:
        case base_type_4K:
        case base_type_4KSC:
        case base_type_F8:
        case base_type_F6:
        case base_type_F4:
        case base_type_UA:
        case base_type_FE:
        case base_type_3F:
        case base_type_3E:
        case base_type_E0:
        case base_type_0840:
        case base_type_CV:
        case base_type_EF:
        case base_type_F0:
        case base_type_FA:
        case base_type_E7:
        case base_type_DPC:
        case base_type_AR:
        case base_type_PP:
        case base_type_3EPlus:
        case base_type_DPCplus:
        case base_type_ACE:
        case base_type_Load_Failed:
        default:
        	return false;
    }

    uint8_t banks = (uint8_t)(image_size / 4096);

    for (uint8_t i = 0; i < RAM_BANKS && i < banks; i++) layout->banks[i] = buffer + i * 4096;

    if(banks > RAM_BANKS){
    	uint32_t bytes_read;
    	uint8_t ccm_banks = (uint8_t) (banks - RAM_BANKS);
    	uint32_t ccm_size = ccm_banks * 4096U;
    	if(d->type == Cart_File ){
#if USE_WIFI
    		bytes_read = esp8266_PlusStore_API_file_request( CCM_RAM, (char*) filename, CCM_IMAGE_OFFSET, ccm_size );
#else
    		bytes_read = 0; // d->type == Cart_File and no WiFi ???
#endif
    	}
    	else if(d->type == SD_Cart_File ){
#if USE_SD_CARD
    		bytes_read = 0; // toDo read from SD-Card to CCM_RAM
#else
    		bytes_read = 0; // d->type == SD_Cart_File and no SD-Card ???
#endif

    	}
    	else
    		bytes_read = flash_file_request( CCM_RAM, d->flash_base_address, CCM_IMAGE_OFFSET, ccm_size );

        if (bytes_read != ccm_size)	return false;

        for (uint8_t i = 0; i < ccm_banks; i++) layout->banks[RAM_BANKS + i] = CCM_RAM + i * 4096;
    }

    if(banks > (RAM_BANKS + CCM_BANKS) ){
        uint32_t flash_part_address;
    	if(d->type == Cart_File ){
#if USE_WIFI
    		flash_part_address = flash_download((char*)filename, FLASH_IMAGE_SIZE, FLASH_IMAGE_OFFSET, true);
#else
    		flash_part_address = 0; // d->type == Cart_File and no WiFi ???
#endif
    	}else if(d->type == SD_Cart_File ){
#if USE_SD_CARD
    		flash_part_address = 0; // todo read from SD-Card to flash
#else
    		flash_part_address = 0; // d->type == SD_Cart_File and no SD-Card ???
#endif
    	}else{
    		flash_part_address = d->flash_base_address + FLASH_IMAGE_OFFSET;
    	}

    	if(flash_part_address == 0)
    		return false;

        for (uint8_t i = 0; i < FLASH_BANKS; i++) layout->banks[RAM_BANKS + CCM_BANKS + i] = (uint8_t *)(flash_part_address + i * 4096U);
    }

	return true;
}

