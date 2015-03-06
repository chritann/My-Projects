#include <avr/eeprom.h>
#include <stdlib.h>
//increment DEVICE_NO EVERYTIME
#define DEVICE_NO 1
#define UID_BYTES 1
int main() {
  uint8_t uid[UID_BYTES];
  srand(DEVICE_NO);
  for (int i = 0; i < UID_BYTES; i++) {
    uid[i] = DEVICE_NO;
  }
  uint8_t eeprom_addr = 0x00;
  for(int i = 0; i < UID_BYTES; i++) {
    eeprom_busy_wait();
    eeprom_write_byte(eeprom_addr, uid[i]);
    eeprom_addr += 0x01;
  }
}
