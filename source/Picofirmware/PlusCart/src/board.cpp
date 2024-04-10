
#include "pico/unique_id.h"
#include "board.h"

char pico_uid[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

#if DBG_SERIAL
   SoftwareSerial dbgSerial(DBG_SERIAL_RX, DBG_SERIAL_TX);
#endif

