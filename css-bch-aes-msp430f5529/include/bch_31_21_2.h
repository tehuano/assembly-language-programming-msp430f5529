/*
 * bch_31_21_2.h
 *
 *  Created on: 22 jun. 2019
 *      Author: Rommel
 */

#ifndef INCLUDE_BCH_31_21_2_H_
#define INCLUDE_BCH_31_21_2_H_

/*
 * bch_31_21_2.c
 *
 *  Created on: 22 jun. 2019
 *      Author: Rommel
 */


/*
 * File:    bch3121.c
 * Author:  Robert Morelos-Zaragoza
 *
 * %%%%%%%%%%% Encoder/Decoder for a (31,21,5) binary BCH code %%%%%%%%%%%%%
 *
 *  This code is used in the POCSAG protocol specification for pagers.
 *
 *  In this specific case, there is no need to use the Berlekamp-Massey
 *  algorithm, since the error locator polynomial is of at most degree 2.
 *  Instead, we simply solve by hand two simultaneous equations to give
 *  the coefficients of the error locator polynomial in the case of two
 *  errors. In the case of one error, the location is given by the first
 *  syndrome.
 *
 *  This program derivates from the original bch2.c, which was written
 *  to simulate the encoding/decoding of primitive binary BCH codes.
 *  Part of this program is adapted from a Reed-Solomon encoder/decoder
 *  program,  'rs.c', to the binary case.
 *
 *  rs.c by Simon Rockliff, University of Adelaide, 21/9/89
 *  bch2.c by Robert Morelos-Zaragoza, University of Hawaii, 5/19/92
 *
 * COPYRIGHT NOTICE: This computer program is free for non-commercial purposes.
 * You may implement this program for any non-commercial application. You may
 * also implement this program for commercial purposes, provided that you
 * obtain my written permission. Any modification of this program is covered
 * by this copyright.
 *
 * %%%% Copyright 1994 (c) Robert Morelos-Zaragoza. All rights reserved. %%%%%
 *
 * m = order of the field GF(2**5) = 5
 * n = 2**5 - 1 = 31 = length
 * t = 2 = error correcting capability
 * d = 2*t + 1 = 5 = designed minimum distance
 * k = n - deg(g(x)) = 21 = dimension
 * p[] = coefficients of primitive polynomial used to generate GF(2**5)
 * g[] = coefficients of generator polynomial, g(x)
 * alpha_to [] = log table of GF(2**5)
 * index_of[] = antilog table of GF(2**5)
 * data[] = coefficients of data polynomial, i(x)
 * bb[] = coefficients of redundancy polynomial ( x**(10) i(x) ) modulo g(x)
 * numerr = number of errors
 * errpos[] = error positions
 * recd[] = coefficients of received polynomial
 * decerror = number of decoding errors (in MESSAGE positions)
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void read_p();
void generate_gf();
void gen_poly();
void encode_bch();
void decode_bch();

#endif /* INCLUDE_BCH_31_21_2_H_ */
