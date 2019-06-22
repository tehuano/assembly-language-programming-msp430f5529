/*
 * memory_local.c
 *
 *  Created on: 6 may. 2019
 *      Author: Rommel
 */

#include "memory_local.h"
#include <msp430.h>

/* Performs read operation to the flash
 */
void read_from_flash(unsigned char *buffer_out) {
    unsigned int i;
    unsigned char *RAM_ptr;
    unsigned char *Flash_ptrD;

    Flash_ptrD = (unsigned char *) INFO_D;             // Initialize Flash segment D ptr
    RAM_ptr = (unsigned char *) buffer_out;
    for (i = 0; i < PAGE_SIZE; i++) {
        *RAM_ptr++ = *Flash_ptrD++;          // copy value segment C to seg D
    }
}

/* Performs write operation to the flash
 */
void write_to_flash(unsigned char *buffer_in) {
      unsigned int i;
      char *RAM_ptr;
      char *Flash_ptrD;

      RAM_ptr = (char *) buffer_in;
      Flash_ptrD = (char *) INFO_D;             // Initialize Flash segment D ptr

      __disable_interrupt();                    // 5xx Workaround: Disable global
                                                // interrupt while erasing. Re-Enable
                                                // GIE if needed

      FCTL3 = FWKEY;                            // Clear Lock bit
      FCTL1 = FWKEY+ERASE;                      // Set Erase bit
      *Flash_ptrD = 0;                          // Dummy write to erase Flash seg D
      FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

      for (i = 0; i < PAGE_SIZE; i++) {
        *Flash_ptrD++ = *RAM_ptr++;             // copy value segment C to seg D
      }

      FCTL1 = FWKEY;                            // Clear WRT bit
      FCTL3 = FWKEY+LOCK;                       // Set LOCK bit

}

/* Performs write key operation to the flash
 */
void write_key_to_flash(unsigned char *pkey_1, unsigned int size) {
      unsigned int i;
      char *RAM_ptr;
      char *Flash_ptrA;

      RAM_ptr = (unsigned char *) pkey_1;
      Flash_ptrA = (unsigned char *) INFO_A;             // Initialize Flash segment A ptr

      __disable_interrupt();                    // 5xx Workaround: Disable global
                                                // interrupt while erasing. Re-Enable
                                                // GIE if needed

      FCTL3 = FWKEY;                            // Clear Lock bit
      FCTL1 = FWKEY+ERASE;                      // Set Erase bit
      *Flash_ptrA = 0;                          // Dummy write to erase Flash seg A
      FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation

      for (i = 0; i < size; i++) {
          *Flash_ptrA++ = *RAM_ptr++;           // copy value to seg D
      }

      FCTL1 = FWKEY;                            // Clear WRT bit
      FCTL3 = FWKEY+LOCK;                       // Set LOCK bit

}

