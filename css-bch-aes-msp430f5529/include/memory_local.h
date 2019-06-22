/*
 * memory_local.h
 *
 *  Created on: 6 may. 2019
 *      Author: Rommel
 */

#ifndef MEMORY_LOCAL_H_
#define MEMORY_LOCAL_H_

    #define PAGE_SIZE         128
    #define BCH31163_CODEWORD 31
    #define BCH31163_DATA     16
    #define BCH31212_CODEWORD 31
    #define BCH31212_DATA     21

    #define BCH31163_BUFFER_TMP_SIZE (((PAGE_SIZE*8)/(BCH31163_CODEWORD+1))*(BCH31163_DATA/8))
    #define BCH31212_BUFFER_TMP_SIZE (((PAGE_SIZE*8)/(BCH31212_CODEWORD+1))*(BCH31212_DATA/8))

    #define INFO_D 0x1800
    #define INFO_A 0x1900

    void read_from_flash();
    void write_key_to_flash();
    void write_to_flash();

#endif /* MEMORY_LOCAL_H_ */
