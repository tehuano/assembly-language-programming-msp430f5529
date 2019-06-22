/*
 * bch_31_21_2.h
 *
 *  Created on: 22 jun. 2019
 *      Author: Rommel
 */

#ifndef INCLUDE_BCH_31_21_2_H_
#define INCLUDE_BCH_31_21_2_H_

#define BCH_31212_ZERO -1

void bch_31212_buffer_encoder(unsigned char *buffer_in, unsigned char *buffer_out);
void bch_31212_buffer_decoder(unsigned char *buffer_in, unsigned char *buffer_out);

void bch31212_bits_2_bytes(unsigned long num, char *p);
unsigned long bch31212_bytes_2_bits(char *p);

void read_p();
void generate_gf();
void gen_poly();
void encode_bch();
void decode_bch();

#endif /* INCLUDE_BCH_31_21_2_H_ */
