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

static unsigned char tmp_key[AES256_KEY_SIZE];

void aes_128_enc_dec_buffer(unsigned char *buffer_128, unsigned char dir) {
    unsigned int i,k;
    char *Flash_ptrA;
    unsigned char *state;

    Flash_ptrA = (char *) INFO_A;
    state = buffer_128;

    for (i = 0; i < BCH31163_BUFFER_TMP_SIZE/AES_128_SIZE; i++) {
        for (k = 0; k < AES128_KEY_SIZE; k++) {
            tmp_key[k] = Flash_ptrA[k];
        }
        aes_enc_dec(state,tmp_key, dir);
        state += AES_128_SIZE;
    }
}

void aes_256_enc_dec_buffer(unsigned char *buffer_256, unsigned char dir) {
    unsigned int i,k;
    char *Flash_ptrA;
    unsigned char *state;

    Flash_ptrA = (char *) INFO_A;
    state = buffer_256;

    for (i = 0; i < BCH31212_BUFFER_TMP_SIZE/AES_256_SIZE; i++) {
        for (k = 0; k < AES256_KEY_SIZE; k++) {
            tmp_key[k] = Flash_ptrA[k];
        }
        aes_enc_dec(state,tmp_key, dir);
        state += AES_256_SIZE;
    }
}
