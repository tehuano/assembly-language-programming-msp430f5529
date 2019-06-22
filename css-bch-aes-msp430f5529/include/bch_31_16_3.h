/*
 * bch_15_7.h
 *
 *  Created on: 6 may. 2019
 *      Author: Rommel
 */

#ifndef BCH_31_16_3_H_
#define BCH_31_16_3_H_

    // Various vars for user input and command control with HyperTerm
    //unsigned char char_count, char_ready, cmd_str[8], cmd_ready;

    //Global Vars
    // extern unsigned long EncodedMessage;    // Encoded Message as a long
    // extern char EncMsgArray[32];            // Encoded Message in Array format
    // extern char n;
    // extern char k;
    // extern char t;          // BCH Code definitions
    // extern unsigned long GeneratorPol;  // Generator Polynomial: 107657 in octal
    // extern char seed;               // Random seed for Error module

    //Counters to keep track of errors for BER calculations
    // extern int trans_error_count;
    // extern int rec_error_count;
    // extern int enc_message_bits;

    // extern char Add_Error;

    typedef struct {
        char p[32];
    } Poly32;

    // Define GF32 - ZERO as negative one since a zero would
    // zero out various multiplications
    #define ZERO -1
    #define RAND_MAX    0x7FFF

    // extern char lookup[];
    // extern char reverseLookup[];

    /* random number */
    // extern unsigned int RegLFSR;



    void bch_31163_buffer_encoder(unsigned char *buffer_in,unsigned char *buffer_out);
    void bch_31163_buffer_decoder(unsigned char *buffer_in,unsigned char *buffer_out);
    void bch_encoder(unsigned char a, unsigned char b, unsigned long *w);
    void bch_decoder(unsigned long w, unsigned char *a, unsigned char *b);
    void multiply_constant(char c, char *p, Poly32 *ret);

    // Initialization function
    void initialize();

    // Initializes the UART interrupt and flags for reading commands
    void get_cmd_init();

    // Main Encoding/Decoding System
    void System();

    // All encoding for BCH ECC is done here - accepts 2 char message
    void encoder_bch(unsigned char a, unsigned char b);

    // All encoding for BCH ECC is done here
    void decoder_bch();

    /** Helper Functions **/

    // Concatenates 2 chars into a long
    unsigned long concate(unsigned char num1, unsigned char num2);

    // Finds the degree of a polynomial encoded as a long in GaloisField2
    unsigned char GF2FindDegree(unsigned long a);

    // Adds 2 polynomials encoded as a long - GF2
    unsigned long GF2Add(unsigned long a, unsigned long b);

    // Polynomial Multiplication for longs in GF2
    unsigned long GF2Multiply(unsigned long a, unsigned long b);

    // Polynomial Division for longs in GF2
    void GF2Divide(unsigned long a, unsigned long b, unsigned long *qr);

    // Polynomial Modulo for longs in GF2
    unsigned long GF2Mod(unsigned long a, unsigned long b);

    // Retrieves a specified bit from a long
    unsigned char getBit(unsigned long r, char i);

    // Converts a long into a 32 length byte array
    void Bits2Bytes(unsigned long num, char *p);

    // Initializes GF32 arrays to ZERO
    void GF32Init(char *p);

    // Adds two alpha coefficients in GF32
    char GF32add2alpha(char a, char b);

    // Finds the degree of a GF32 polynomial
    char GF32FindDegree(char *p);

    // Evaluates the result of a GF32 Polynomial for some x
    char GF32Evaluate(char a, char *p);

    // Adds all alpha coefficients pairwise in 2 Arrays in GF32
    void GF32Add(char *a, char *b, Poly32 *powers);

    // Polynomial Multiplication for longs in GF32
    void GF32Multiply(char *a, char *b, Poly32 *mul);

    // Multiplies a GF32 polynomial with some power of x
    void multiply_x(char x_power, char *p, Poly32 *ret);

    // Multiplies a GF32 polynomial by a constant
    void multiplyConstant(char c, char *p, Poly32 *ret);

    // Corrects the detected errors in an encoded message
    void CorrectErrors(char *p);

    // Converts an array of bytes into a long
    unsigned long bytes_2_bits(char *p);

    // Parses the lower 16bits of a long into 2 chars
    void de_concate(unsigned long a, unsigned char *ret);

    // A random error module that corrupts an encoded message
    void error_module(char *p, char numerrors);

    void LFSRs(unsigned int reset, unsigned int semilla);
    unsigned int rand_local();

#endif /* BCH_31_16_3_H_ */
