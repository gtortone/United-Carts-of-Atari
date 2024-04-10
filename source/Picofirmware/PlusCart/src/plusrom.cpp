#include "esp8266.h"
#include "plusrom.h"
#include "cartridge_emulation.h"

#if USE_WIFI

volatile uint8_t __not_in_flash() receive_buffer_write_pointer, receive_buffer_read_pointer;
volatile uint8_t __not_in_flash() out_buffer_write_pointer, out_buffer_send_pointer;
uint8_t __not_in_flash() receive_buffer[256], out_buffer[256];
volatile uint8_t content_counter = 0;
volatile uint8_t prev_c = 0, prev_prev_c = 0, c = 0, len = 0;
volatile uint16_t i;
volatile uint16_t content_len;
volatile enum Transmission_State __not_in_flash() uart_state;

void handle_plusrom_comms(void) {

   uint8_t flag;
   int16_t http_header_length;
   int content_length_pos;

   uart_state = No_Transmission;
   receive_buffer_read_pointer = 0;
   receive_buffer_write_pointer = 0;
   out_buffer_send_pointer = 0;
   out_buffer_write_pointer = 0;

   http_header_length = strlen(http_request_header);
   content_length_pos = http_header_length - 5;

   while(true) {

      queue_remove_blocking(&qprocs, &flag);
     
      if(flag == EMU_EXITED) {

         // quit
         break;

      } else if(flag == EMU_PLUSROM_SENDSTART) {

         // send request
      
         i = 0;
         content_len = out_buffer_write_pointer;
         content_len++;

         if(content_len == 0) {
            http_request_header[content_length_pos] = (char)'0';
         } else {
            i = content_length_pos;
            while(content_len != 0) {
               c = (uint8_t) (content_len % 10);
               http_request_header[i--] = (char) (c + '0');
               content_len = content_len/10;
            }
         }

         espSerial.write(http_request_header, http_header_length);
         espSerial.write(&out_buffer[out_buffer_send_pointer], out_buffer_write_pointer - out_buffer_send_pointer + 1);
         espSerial.flush();
         out_buffer_write_pointer = 0;
         out_buffer_send_pointer = 0;

         // receive response

         char ch;
         
         // skip HTTP header (jump to first byte of payload)
         char line[128];
         while(espSerial.readBytesUntil('\n', line, 128) != 1)
            ;

         // read payload len (first byte)
         while(true) {
            if(espSerial.available() > 0) {
               ch = espSerial.read();
               if(ch != -1) {
                  len = (uint8_t) ch;
                  break;
               }
            }
         }

         // read payload
         if(len != 0) {
            espSerial.readBytes(&receive_buffer[receive_buffer_write_pointer], len);
            receive_buffer_write_pointer += len;
         }

         // debug receive buffer read/write index
   
         http_request_header[content_length_pos - 1] = ' ';
         http_request_header[content_length_pos - 2] = ' ';
         content_counter = 0;
         uart_state = No_Transmission;
         len = c = prev_c = prev_prev_c = 0;
      }

   } // end while

}
#endif

