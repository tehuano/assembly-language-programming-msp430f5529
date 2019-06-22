/*
 * bch_15_7.c
 *
 *  Created on: 6 may. 2019
 *      Author: Rommel
 */


#include "memory_local.h"
#include "bch_31_16_3.h"
#include <math.h>

/* Define GF32 - ZERO as negative one since a zero would */
/* zero out various multiplications */
#define ZERO -1

/* Variable Definitions */
/* Various vars for user input and command control with HyperTerm */
static unsigned char char_count, char_ready, cmd_str[8], cmd_ready;


//Global Vars
static unsigned long IntEncodedMsg=0; // Encoded Message as a long
static char EncBuffer[32];           // Encoded Message in Array format
static char n=31, k=16, t=3;           // BCH Code definitions
static unsigned long GeneratorPoly = 36783;    // Generator Polynomial: 1000111110101111

static char seed=0;  // Random seed for Error module
//Counters to keep track of errors for BER calculations
static int trans_error_count=0,rec_error_count=0;
static int enc_message_bits;
// Flags for Display and Error Module
static char detail_toggle=0, Add_Error=0;

// lookup tables for GF32
// Alpha exponents - computed as powers of 2
// Alpha is from the Galois Field(32)
static char lookup[] = {
                1,  // a^0  = 0
                2,  // a^1  = 1
                4,  // a^2  = 2
                8,  // a^3  = 3
                16, // a^4  = 4
                5,  // a^5  = 2 0
                10, // a^6  = 3 1
                20, // a^7  = 4 2
                13, // a^8  = 3 2 0
                26, // a^9  = 4 3 1
                17, // a^10 = 4 0
                7,  // a^11 = 2 1 0
                14, // a^12 = 3 2 1
                28, // a^13 = 4  3 2
                29, // a^14 = 4 3 2 0
                31, // a^15 = 4 3 2 1 0
                27, // a^16 = 4 3 1 0
                19, // a^17 = 4  1 0
                3,  // a^18 = 1 0
                6,  // a^19 = 2 1
                12, // a^20 = 3 2
                24, // a^21 = 4 3
                21, // a^22 = 4 2 0
                15, // a^23 = 3 2 1 0
                30, // a^24 = 4 3 2 1
                25, // a^25 = 4 3 0
                23, // a^26 = 4 2 1 0
                11, // a^27 = 3 1 0
                22, // a^28 = 4 2 1
                9,  // a^29 = 3 0
                18, // a^30 = 4 1
                1   // a^31 = 0
            };

static char reverseLookup[] = {
                ZERO, // a^0
                0,  // a^1
                1,  // a^2
                18, // a^3
                2,  // a^4
                5,  // a^5
                19, // a^6
                11, // a^7
                3,  // a^8
                29, // a^9
                6,  // a^10
                27, // a^11
                20, // a^12
                8,  // a^13
                12, // a^14
                23, // a^15
                4,  // a^16
                10, // a^17
                30, // a^18
                17, // a^19
                7,  // a^20
                22, // a^21
                28, // a^22
                26, // a^23
                21, // a^24
                25, // a^25
                9,  // a^26
                16, // a^27
                13, // a^28
                14, // a^29
                24, // a^30
                15  // a^31
            };
static unsigned int RegLFSR=0;


void bch_31163_buffer_encoder(unsigned char *buffer_in,unsigned char *buffer_out) {
    unsigned char  a, b;
    unsigned int   i;
    unsigned long  codeword;
    unsigned long  *ptr_buff;
    unsigned int   *ptr_tmp;

    ptr_buff = (unsigned long *)buffer_in;
    ptr_tmp  = (unsigned int  *)buffer_out;

    for (i = 0; i < BCH31163_BUFFER_TMP_SIZE/sizeof(unsigned int); i++) {
        a = 0xff & ptr_tmp[i];
        b = 0xff & (ptr_tmp[i] >> 8);
        encoder_bch(a,b);
        error_module(EncBuffer, 3);
        codeword = bytes_2_bits(EncBuffer);
        ptr_buff[i] = codeword;
    }
}

void bch_31163_buffer_decoder(unsigned char *buffer_in, unsigned char *buffer_out) {
    unsigned char a,b;
    unsigned int i;
    unsigned long  *ptr_buff;
    unsigned int   *ptr_tmp;
    unsigned long  word;
    unsigned char out[2];

    ptr_buff = (unsigned long *)buffer_in;
    ptr_tmp  = (unsigned int  *)buffer_out;

    for (i = 0; i < BCH31163_BUFFER_TMP_SIZE/sizeof(unsigned int); i++) {
        bits_2_bytes(ptr_buff[i], EncBuffer);    // Convert to a GF32 polynomial
        decoder_bch();
        word = bytes_2_bits(EncBuffer);
        de_concate(word, out);
        a = out[0];
        b = out[1];
        ptr_tmp[i] =  0x00ff & a;
        ptr_tmp[i] |= 0xff00 & (b << 8);
    }
}

void bch_encoder(unsigned char b, unsigned char a, unsigned long *w) {
    unsigned int aa, bb;
    aa = a;
    bb = b;
    *w = 0xAAAA0000;
    *w |= aa;
    *w |= (bb << 8);
}

void bch_decoder(unsigned long w, unsigned char *b, unsigned char *a) {
    *a = 0x000000ff & w;
    *b = 0x000000ff & (w >> 8);
}

void initialize() {
    n=31; k=16; t=3;    // BCH Code parameters to (31,16)
    IntEncodedMsg=0;
    GeneratorPoly = 36783; // Init the Generator polynomial

    // initialize the toggles and the random seed to No Errors
    detail_toggle=0;
    Add_Error=0;
    seed=0;

    // initialize the Error Counters for BER
    trans_error_count=0;
    rec_error_count=0;
    // The # of bits in the Full encoded msg
    enc_message_bits=0;
}



/*16 bit Message Encoder*/
void encoder_bch(unsigned char a, unsigned char b) {

    // (m(x) * x^(n-k)) mod g(x) + (m(x) * x^(n-k))

    // Shift by x^(n-k) = x*15
    IntEncodedMsg = concate(a, b); // Concatenates the two chars to a 16bit message
    IntEncodedMsg = IntEncodedMsg << (n-k);   // Multiply by x^(n-k)

    IntEncodedMsg = GF2Add(GF2Mod(IntEncodedMsg, GeneratorPoly), IntEncodedMsg);

    //  IntEncodedMsg ^= 0x20010008;   // Error manually added

    GF32Init(EncBuffer); // initialize to ZERO's
    bits_2_bytes(IntEncodedMsg, EncBuffer);    // Convert to a GF32 polynomial
}

/* 32 bit BerleKamp Decoder for BCH Codes */
void decoder_bch() {
    unsigned int i, j;
    char dk = 0;
    char delta[7];
    char Syndromes[32];
    //Lambda is an array of 7 GF32 polynomials structs
    //Note: Lambda/T arrays can be small (i.e length 4) if rewrite multiply/divide, etc.
    Poly32 lambda[7];
    Poly32 T[7];
    Poly32 temp;

    GF32Init(Syndromes);
    /* Create Syndrome Polynomial */
    for (i = 0; i < (2*t); i++) {
        Syndromes[i + 1] = (char)GF32Evaluate(i + 1, EncBuffer);
    }

    /* 1  - Initialization */
    /* add 1 to S(x) and initialize Berlekamp's Algorithm */
    Syndromes[0] = 0;

    /* Init Lambda[i] polynomials */
    for (i = 0; i < 7; i++) {
        for (j=0; j<32; j++) {
            lambda[i].p[j]=ZERO;
            T[i].p[j]=ZERO;
        }
    }

    // lambda_0 (x) = 1
    lambda[0].p[0] = 0;
        // T_0 (x) = 1
    T[0].p[0] = 0;

    while( dk < t ) {
        /* Berlekamp Algorithm */
        /* 2 */    // Delta[2k] = coeff. of x^(2k+1) in the product Lambda[2k](x) * [1 + Syn(x)]
        GF32Multiply(lambda[2*dk].p, Syndromes, &temp);
        delta[2*dk]  = (char)temp.p[2*dk+1];

        /* 3 */
        // Lambda[2k+2](x) = Lambda[2k](x) + Delta[2k]*(x*T[2k](x))
        multiply_x(1, T[2*dk].p, &temp);
        multiply_constant(delta[2*dk], temp.p, &temp);
        GF32Add(lambda[2*dk].p, temp.p, &lambda[2*dk+2]);

        /* 4 */
        if (delta[2*dk] == ZERO || (char)GF32FindDegree(lambda[2*dk].p) > k) {
            multiply_x(2, T[2*dk].p, &T[2*dk+2]);
        } else {
            multiply_x(1, lambda[2*dk].p, &temp);
            multiply_constant((char)(31-delta[2*dk]), temp.p, &T[2*dk+2]);
        }

        /* 5 */
        dk++; // Increment for next iteration
    }
    correct_errors(lambda[2*dk].p);   /* Correct the errors as determined by the locator Lambda polynomial */
}

/* Concatenate */
unsigned long concate(unsigned char num1, unsigned char num2) {
    unsigned long temp = 0;
    temp |= num1;   // Or in the num1 and shift it up
    temp <<= 8;
    temp |= num2;   // Or in num2
    return temp;
}

/* Find Degree of a Polynomial in GF2*/
unsigned char GF2FindDegree(unsigned long num) {
    char i=0;
    num = num << 1; // Shift left since top bit is ignored in algorithm
    for(i=0; i<30; i++) {
        if (num & 0x80000000) { /* Mask the current top bit, to see if it's a one */
            return (30-i);      /* if so, that's the degree */
        }
        num = num << 1;         /* otherwise, keep shifting */
    }
    return 0;
}

/* Polynomial Addition in GF2 */
unsigned long GF2Add(unsigned long a, unsigned long b) {
    return (a^b); /* (GF2 addition for polynomials) */
}

/* Polynomial Multiplication in GF2 */
unsigned long GF2Multiply(unsigned long a, unsigned long b) {
    unsigned long mul = 0;
    unsigned long add;

    char i;
    add = b;
    for(i=0; i <= GF2FindDegree(a); i++) { /* loop while not to the end of the poly */
        if(getBit(a, i) == 1) {            /* If coeff. is a one, then add multiplicand */
            mul ^= add;
        }
        add = add<<1;                      /* and shift the multiplicand up one */
    }

    return mul;
}

/* Polynomial Long Division in GF2 */
void GF2Divide(unsigned long a, unsigned long b, unsigned long *qr) {
    unsigned long dividend;
    unsigned long divisor;

    unsigned long q;
    int deg = 0;

    dividend = a;
    divisor = b;
    qr[0] = 0;

    while(1) {  // Keep doing this until break is activated
        // Subtract degrees to find what the degree of each term in the quotient
        deg = (int)(GF2FindDegree(dividend) - (int)GF2FindDegree(divisor));
        if (deg < 0) {  // If negative, then you are done
            qr[1] = dividend;       // return the dividend as the remainder
            return ;
        }
        if (deg > 0) {  // otherwise find the appropriate degree for the term
            q = (unsigned long)pow((float)2,(float)deg)+1;
        } else {
            q = 1;
        }
        qr[0] = GF2Add(qr[0], q);   // and add the term to the quotient
        // finally, reduce (i.e add mod 2) the divided by (term*divisor)
        dividend = GF2Add(dividend, (GF2Multiply(q, divisor)));
    }
    qr[1] = dividend;       // Return the remainder
}

/* Polynomial Modulo in GF2 */
unsigned long GF2Mod(unsigned long a, unsigned long b) {
    unsigned long qr[2] = {0,0};
    GF2Divide(a, b, &qr[0]);
    return qr[1];
}

/* Get a bit from a Long */
unsigned char getBit(unsigned long r, char i) {
    unsigned char ret;
    // Shifts and Masks to get the appropriate bit
    ret = ((r<<(32-i-1))>>31)& 0x00000001;
    return ret;
}

/* Long to Array convertor */
void bits_2_bytes(unsigned long num, char *p) {
    unsigned int i = 0;
    char temp = 0;
    for(i = 0; i < 32; i++) {
        temp = num % 2;
        if (temp == 0) {
            p[i] = ZERO;    // -1 is ZERO, i.e. coeff = 0
        } else {
            p[i] = 0;       // alpha**0, i.e. coeff = 1
        }
        num = num >> 1;     // shift for next iteration
    }
}

/* GF32 initialize */
void GF32Init(char *p) {
     unsigned int i = 0;
     for (i = 0; i < 32; i++) {
         p[i] = ZERO;
     }
}

/* Add Two Alpha Coeff. in GF32 */
char GF32add2alpha(char a, char b) {
        if ((a == ZERO) && (b == ZERO)) {   // ZERO+ZERO=ZERO
            return ZERO;
        }
        if (a == ZERO) {
            return b;
        } else if (b == ZERO) {
            return a;
        } else {                    // Simply XOR and use lookup
            return reverseLookup[(unsigned int )(lookup[(unsigned int)a]^lookup[(unsigned int)b])];
        }
}

/* Find Degree of GF32 Polynomial */
char GF32FindDegree(char *p) {
        unsigned int i = 32;
        while(--i > 0) {
            if (p[i] != ZERO) {
                return i;
            }
        }
        return 0;
}

/* GF32 Polynomial Multiplication */
void GF32Multiply(char *a, char *b, Poly32 *mul) {
    Poly32 add;
    unsigned int i,j;
    unsigned int dega, degb;
    for(i=0; i<32; i++) { /* initialize the arrays */
        mul->p[i] = ZERO;
        add.p[i] = ZERO;
    }
    dega = GF32FindDegree(a);
    degb = GF32FindDegree(b);
    for(i=0; i <= dega; i++) {
        if(a[i] != ZERO) { /* multiply only non-zero terms */
            for(j=0; j <= degb; j++) { /* add then shift */
                if(b[j] != ZERO) {
                    if ((j+i) < 32) {
                        add.p[j+i] = (char)((a[i]+b[j]) % 31);
                    }
                }
            }
            GF32Add(mul->p, add.p, mul);
            GF32Init(add.p);
        }
    }
    return;
}

/* GF32 Polynomial Evaluation */
char GF32Evaluate(char a, char *p) {
    char ret = ZERO;
    unsigned int i=0;
    char pow=0;

    for(i=0; i <= GF32FindDegree(p); i++) { /* evaluate over the length of the polynomial */
        if (p[i] != ZERO) {
            pow = (char)((p[i] + (a*i)) % 31); // index is the degree, multiply exponents
            if (pow < 0) {
                pow = (char)(31+pow); // Evaluate mod 32
            }
            ret = GF32add2alpha(ret, pow);  // exponent multiplication = add
        }
    }
    return ret;
 }

/* GF32 Add Two Polynomials */
void GF32Add(char *a, char *b, Poly32 *powers) {
    unsigned int i=0;
    for (i = 0; i < 32; i++) {
        powers->p[i] = GF32add2alpha(b[i], a[i]);
    }
}


/* Multiply GF32 Polynomial by x^power */
void multiply_x(char x_power, char *p, Poly32 *ret) {
    unsigned int i;
    for(i = 0; i < 32; i++) {
        ret->p[(i+x_power) % 32] = p[i];    // cyclic shift mod 32
    }
}

/* Multiply GF32 Polynomial by Constant */
void multiply_constant(char c, char *p, Poly32 *ret) {
    unsigned int i;
    /* if multiplying by zero, return zero */
    if (c == ZERO) {
        GF32Init(ret->p);
        return ;
    }
    for (i = 0; i < 32; i++) {
        if(p[i] != ZERO ) {
            ret->p[i] = (char)((p[i]+c) % 31); // add the constant exponent, mod 32
        } else {
            ret->p[i] = ZERO;
        }
    }
}


/* Correct the Errors in the Encoded Message */
void correct_errors(char *p) {
    unsigned int i;
    // evaluate roots of lambda[2*k] and flip the received code word bits accordingly
    for(i = 0; i < 32; i++) {
        if (GF32Evaluate(i, p) == ZERO ) {  // Find the roots of Lambda
        //  //printf("\r\nError at index: %d\r\n", (31-i));
            if (EncBuffer[31-i] == ZERO) {    // Simply flip the bits
                EncBuffer[31-i] = (char)0;
            } else {
                EncBuffer[31-i] = ZERO;
            }
        }
    }
}


/* Convert GF32 polynomial (array) to a GF2 polynomial (long) */
unsigned long bytes_2_bits(char *p) {
    char i;
    unsigned long ret=0;

    for(i = 0; i < 31; i++) {
        ret = ret | (p[31-i] == 0); /* if 0, or in a 1, if ZERO, or in a 0 */
        ret = ret << 1; /* and then shift it up */
    }
    ret |= (p[0] == 0); /* shift up the last one - since only 31 elements */

    return ret;
}

/* De-Concatenate */
// Returns the bottom 16 bits of a long as 2 chars
void de_concate(unsigned long a, unsigned char *ret) {
    a = a << 1;
    ret[0] = (unsigned char)((a & 0xFF000000) >> 24);
    ret[1] = (unsigned char)((a & 0x00FF0000) >> 16);
}


/* Random Noise Module */
void error_module(char *p, char num_errors) {
    char i, val;
    for (i=1; i<=num_errors; i++) { // num_errors is random and seeded by the user
        val = (int)(rand_local() * (32.0) / (RAND_MAX + 1.0)) + 1;    //random integer in 1-32 range
        //printf("%d ", val-1);
        if (p[val-1] == ZERO) { // Flip the appropriate bits
            p[val-1] = 0;
        }  else {
            p[val-1] = ZERO;
        }
    }
}

unsigned int rand_local() {
    LFSRs(0,0);
    return RegLFSR;
}

/**************************************************************************************/
void LFSRs(unsigned int reset, unsigned int semilla) {
    unsigned int tap1;
    unsigned int tap3;
    unsigned int tap4;
    unsigned int tap13;
    unsigned int fb;
    if (reset==1){
        RegLFSR=semilla;
    }
    if (RegLFSR & 0x0001)
        {tap1=1;}
    else
        {tap1=0;}
    if (RegLFSR & 0x0004)
        {tap3=1;}
    else
        {tap3=0;}
    if (RegLFSR & 0x0008)
       {tap4=1;}
    else
       {tap4=0;}
    if (RegLFSR & 0x1000)
        {tap13=1;}
    else
        {tap13=0;}
    fb=tap13^(tap4^(tap1^tap3));
    RegLFSR=(RegLFSR>>1)|(fb*0x1000);
}
