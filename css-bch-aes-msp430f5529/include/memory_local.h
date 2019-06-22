/*
 * memory_local.h
 *
 *  Created on: 6 may. 2019
 *      Author: Rommel
 */

#ifndef MEMORY_LOCAL_H_
#define MEMORY_LOCAL_H_

    #define PAGE_SIZE       128
    #define CODEWORD        31
    #define DATA_SIZE       16
    #define BUFFER_TMP_SIZE (((PAGE_SIZE*8)/(CODEWORD+1))*(DATA_SIZE/8))

    #define KEY_BUFFER_SIZE 16
    #define NO_OPERATION    0x00
    #define OPERATION       0x01

    #define INFO_D          0x1800
    #define INFO_A          0x1900

    extern unsigned char index_dmabuff;
    extern unsigned char index_keybuff;

    extern unsigned char buffer_tmp[BUFFER_TMP_SIZE];
    extern unsigned char buffer[PAGE_SIZE];
    extern unsigned char key_1[KEY_BUFFER_SIZE];

    void read_from_flash();
    void write_key_to_flash();
    void write_to_flash();

#endif /* MEMORY_LOCAL_H_ */
