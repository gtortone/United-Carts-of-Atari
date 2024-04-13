#ifndef PLUSROM_H_
#define PLUSROM_H_

enum Transmission_State{
   No_Transmission,
   Send_Start,
   Send_Prepare_Header,
   Send_Header,
   Send_Content,
   Send_Finished,
   Receive_Header,
   Receive_Length,
   Receive_Content,
   Receive_Finished
};

#define UART_DR            uart0_hw->dr
#define UART_RX_AVAIL      !(uart0_hw->fr & UART_UARTFR_RXFE_BITS)
#define UART_TX_BUSY       (uart0_hw->fr & UART_UARTFR_BUSY_BITS)
#define UART_FIFO_TX_FULL  (uart0_hw->fr & UART_UARTFR_TXFF_BITS)

extern volatile uint8_t receive_buffer_read_pointer, receive_buffer_write_pointer; \
extern volatile uint8_t out_buffer_write_pointer, out_buffer_send_pointer; \
extern uint8_t receive_buffer[256], out_buffer[256]; \
extern volatile uint8_t content_counter;
extern volatile uint8_t prev_c, prev_prev_c, c, len;
extern volatile uint16_t i;
extern volatile uint16_t content_len;
extern volatile int content_length_pos;
extern volatile int16_t http_header_length;
extern volatile enum Transmission_State uart_state;

void handle_plusrom_comms(void);

#define process_transmission() \
   switch(uart_state) { \
\
      case Send_Start: \
         content_len = out_buffer_write_pointer; \
         content_len++; \
         i = content_length_pos; \
         uart_state = Send_Prepare_Header; \
         break; \
\
      case Send_Prepare_Header: \
         if (content_len != 0) { \
            c = (uint8_t) (content_len % 10); \
            http_request_header[i--] =  (char) (c + '0'); \
            content_len = content_len/10; \
         } else { \
            i = 0; \
            uart_state = Send_Header; \
         } \
         break; \
\
      case Send_Header: \
         if(!UART_FIFO_TX_FULL) { \
            UART_DR = http_request_header[i]; \
            if(++i == http_header_length) \
               uart_state = Send_Content; \
         } \
         break; \
\
      case Send_Content: \
         if(!UART_FIFO_TX_FULL) { \
            UART_DR = out_buffer[out_buffer_send_pointer]; \
            if(out_buffer_send_pointer == out_buffer_write_pointer) \
               uart_state = Send_Finished; \
            else \
               out_buffer_send_pointer++; \
         } \
         break; \
\
      case Send_Finished: \
         if(!UART_TX_BUSY) { \
            out_buffer_write_pointer = 0; \
            out_buffer_send_pointer = 0; \
            uart_state = Receive_Header; \
         } \
         break; \
\
      case Receive_Header: \
         if(UART_RX_AVAIL) { \
            c = (uint8_t) UART_DR; \
            if(c == '\n' && c == prev_prev_c) { \
               uart_state = Receive_Length; \
            } else { \
               prev_prev_c = prev_c; \
               prev_c = c; \
            } \
         } \
         break; \
\
      case Receive_Length: \
         if(UART_RX_AVAIL) { \
            len = (uint8_t) UART_DR; \
            uart_state = Receive_Content; \
            if(len == 0) \
               uart_state = Receive_Finished; \
         } \
         break;  \
\
      case Receive_Content: \
         if(UART_RX_AVAIL) { \
            receive_buffer[receive_buffer_write_pointer++] = (uint8_t) UART_DR; \
            if(++content_counter == len) \
               uart_state = Receive_Finished;\
         } \
         break; \
\
      case Receive_Finished: \
         http_request_header[content_length_pos - 1] = ' '; \
         http_request_header[content_length_pos - 2] = ' '; \
         content_counter = 0; \
         uart_state = No_Transmission; \
         break; \
\
      case No_Transmission: \
      default: \
         break; \
   } 

#endif
