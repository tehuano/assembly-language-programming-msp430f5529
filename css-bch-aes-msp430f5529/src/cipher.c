/*
 * cipher.c
 *
 *  Created on: 7 may. 2019
 *      Author: Rommel
 */
#include "cipher.h"
#include "memory_local.h"
#include "TI_aes_128.h"
#include "aes_256.h"

void aes_128_enc_dec_buffer(unsigned char dir) {
    unsigned int i,k;
    char *Flash_ptrA;
    unsigned char *state;

    Flash_ptrA = (char *) INFO_A;
    state = buffer_tmp;

    for (i = 0; i < BUFFER_TMP_SIZE/AES_128_SIZE; i++) {
        for (k = 0; k < KEY_BUFFER_SIZE; k++) {
            key_1[k] = Flash_ptrA[k];
        }
        aes_enc_dec(state,key_1, dir);
        state += AES_128_SIZE;
    }
}
