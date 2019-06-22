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

static int m = 5, n = 31, k = 21, t = 2, d = 5;
static int length = 31;
static int p[6];       /* irreducible polynomial */
static int alpha_to[32], index_of[32], g[11];
static int recd[31], data[21], bb[11];
static int numerr, errpos[32], decerror = 0;

/* Primitive polynomial of degree 5 */
void read_p() {
    p[0] = p[2] = p[5] = 1; p[1] = p[3] = p[4] =0;
}

/*
 * generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
 * lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
 * polynomial form -> index form  index_of[j=alpha**i] = i alpha=2 is the
 * primitive element of GF(2**m)
 */
void generate_gf() {
    register int i, mask;
    mask = 1;
    alpha_to[m] = 0;
    for (i = 0; i < m; i++) {
        alpha_to[i] = mask;
        index_of[alpha_to[i]] = i;
        if (p[i] != 0)
            alpha_to[m] ^= mask;
        mask <<= 1;
    }
    index_of[alpha_to[m]] = m;
    mask >>= 1;
    for (i = m + 1; i < n; i++) {
        if (alpha_to[i - 1] >= mask)
          alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
        else
          alpha_to[i] = alpha_to[i - 1] << 1;
        index_of[alpha_to[i]] = i;
    }
    index_of[0] = -1;
}

/*
 * Compute generator polynomial of BCH code of length = 31, redundancy = 10
 * (OK, this is not very efficient, but we only do it once, right? :)
 */
void gen_poly() {
    register int ii, jj, ll, kaux;
    int test, aux, nocycles, root, noterms, rdncy;
    int cycle[15][6], size[15], min[11], zeros[11];
    /* Generate cycle sets modulo 31 */
    cycle[0][0] = 0; size[0] = 1;
    cycle[1][0] = 1; size[1] = 1;
    jj = 1;         /* cycle set index */
    do {
        /* Generate the jj-th cycle set */
        ii = 0;
        do {
            ii++;
            cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
            size[jj]++;
            aux = (cycle[jj][ii] * 2) % n;
        } while (aux != cycle[jj][0]);
        /* Next cycle set representative */
        ll = 0;
        do {
            ll++;
            test = 0;
            for (ii = 1; ((ii <= jj) && (!test)); ii++)
            /* Examine previous cycle sets */
              for (kaux = 0; ((kaux < size[ii]) && (!test)); kaux++)
                    if (ll == cycle[ii][kaux])
                        test = 1;
        } while ((test) && (ll < (n - 1)));
        if (!(test)) {
            jj++;   /* next cycle set index */
            cycle[jj][0] = ll;
            size[jj] = 1;
        }
    } while (ll < (n - 1));
    nocycles = jj;      /* number of cycle sets modulo n */
    /* Search for roots 1, 2, ..., d-1 in cycle sets */
    kaux = 0;
    rdncy = 0;
    for (ii = 1; ii <= nocycles; ii++) {
        min[kaux] = 0;
        for (jj = 0; jj < size[ii]; jj++)
            for (root = 1; root < d; root++)
                if (root == cycle[ii][jj])
                    min[kaux] = ii;
        if (min[kaux]) {
            rdncy += size[min[kaux]];
            kaux++;
        }
    }
    noterms = kaux;
    kaux = 1;
    for (ii = 0; ii < noterms; ii++)
        for (jj = 0; jj < size[min[ii]]; jj++) {
            zeros[kaux] = cycle[min[ii]][jj];
            kaux++;
        }
    // printf("This is a (%d, %d, %d) binary BCH code\n", length, k, d);
    /* Compute generator polynomial */
    g[0] = alpha_to[zeros[1]];
    g[1] = 1;       /* g(x) = (X + zeros[1]) initially */
    for (ii = 2; ii <= rdncy; ii++) {
      g[ii] = 1;
      for (jj = ii - 1; jj > 0; jj--)
        if (g[jj] != 0)
          g[jj] = g[jj - 1] ^ alpha_to[(index_of[g[jj]] + zeros[ii]) % n];
        else
          g[jj] = g[jj - 1];
      g[0] = alpha_to[(index_of[g[0]] + zeros[ii]) % n];
    }
    // printf("g(x) = ");
    for (ii = 0; ii <= rdncy; ii++) {
      // printf("%d", g[ii]);
      if (ii && ((ii % 70) == 0)){
        // printf("\n");
      }
    }
    // printf("\n");
}

/*
 * Calculate redundant bits bb[], codeword is c(X) = data(X)*X**(n-k)+ bb(X)
 */
void encode_bch() {
    register int i, j;
    register int feedback;
    for (i = 0; i < length - k; i++)
        bb[i] = 0;
    for (i = k - 1; i >= 0; i--) {
        feedback = data[i] ^ bb[length - k - 1];
        if (feedback != 0) {
            for (j = length - k - 1; j > 0; j--)
                if (g[j] != 0)
                    bb[j] = bb[j - 1] ^ feedback;
                else
                    bb[j] = bb[j - 1];
            bb[0] = g[0] && feedback;
        } else {
            for (j = length - k - 1; j > 0; j--)
                bb[j] = bb[j - 1];
            bb[0] = 0;
        };
    };
};

/*
 * We do not need the Berlekamp algorithm to decode.
 * We solve before hand two equations in two variables.
 */
void decode_bch() {
    register int i, j, q;
    int elp[3], s[5], s3;
    int count = 0, syn_error = 0;
    int loc[3], reg[3];
    int aux;
    /* first form the syndromes */
    // printf("s[] = (");
    for (i = 1; i <= 4; i++) {
        s[i] = 0;
        for (j = 0; j < length; j++)
            if (recd[j] != 0)
                s[i] ^= alpha_to[(i * j) % n];
        if (s[i] != 0)
            syn_error = 1;  /* set flag if non-zero syndrome */
                            /* NOTE: If only error detection is needed,
                             * then exit the program here...
                             */
        /* convert syndrome from polynomial form to index form  */
        s[i] = index_of[s[i]];
        // printf("%3d ", s[i]);
    };
    // printf(")\n");
    if (syn_error) {    /* If there are errors, try to correct them */
        if (s[1] != -1) {
            s3 = (s[1] * 3) % n;
            if ( s[3] == s3 )  /* Was it a single error ? */
                {
                // printf("One error at %d\n", s[1]);
                recd[s[1]] ^= 1;        /* Yes: Correct it */
                }
            else {              /* Assume two errors occurred and solve
                                 * for the coefficients of sigma(x), the
                                 * error locator polynomail
                                 */
                if (s[3] != -1)
                  aux = alpha_to[s3] ^ alpha_to[s[3]];
                else
                  aux = alpha_to[s3];
                elp[0] = 0;
                elp[1] = (s[2] - index_of[aux] + n) % n;
                elp[2] = (s[1] - index_of[aux] + n) % n;
                // printf("sigma(x) = ");
                for (i = 0; i <= 2; i++)
                    // printf("%3d ", elp[i]);
                // printf("\n");
                // printf("Roots: ");
                /* find roots of the error location polynomial */
                for (i = 1; i <= 2; i++)
                    reg[i] = elp[i];
                count = 0;
                for (i = 1; i <= n; i++) { /* Chien search */
                    q = 1;
                    for (j = 1; j <= 2; j++)
                        if (reg[j] != -1) {
                            reg[j] = (reg[j] + j) % n;
                            q ^= alpha_to[reg[j]];
                        }
                    if (!q) {   /* store error location number indices */
                        loc[count] = i % n;
                        count++;
                        // printf("%3d ", (i%n));
                    }
                }
                // printf("\n");
                if (count == 2){
                /* no. roots = degree of elp hence 2 errors */
                    for (i = 0; i < 2; i++)
                        recd[loc[i]] ^= 1;
                }
                else {    /* Cannot solve: Error detection */
                    // printf("incomplete decoding\n");
                }
                }
            } else if (s[2] != -1) {/* Error detection */
            // printf("incomplete decoding\n");
        }
    }
}
