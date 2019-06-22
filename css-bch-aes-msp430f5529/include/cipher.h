/*
 * cipher.c
 *
 *  Created on: 7 may. 2019
 *      Author: Rommel
 */

#ifndef CIPHER_C_
#define CIPHER_C_

#define AES_128_SIZE 16
#define AES_256_SIZE 32

#define AES128_KEY_SIZE 16
#define AES256_KEY_SIZE 32

void aes_128_enc_dec_buffer(unsigned char *buffer_128, unsigned char dir);
void aes_256_enc_dec_buffer(unsigned char *buffer_256, unsigned char dir);

#endif /* CIPHER_C_ */
