#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#include <stdint.h>

enum endianness { NOTCALCULATED_ENDIAN = -3, INVALID_ENDIAN_ARG = -2, UNRECOGNIZED_ENDIAN = -1, MACHINE_ENDIAN = 0, 
        BIG_ENDIAN_REPR, LITTLE_ENDIAN_REPR
     };



int getInt32End(void);
//int getInt64End(void);
int getFloatEnd(void);
//int getDoubleEnd(void);
int writeInt32InBuffer(int32_t i, uint8_t *buffer, enum endianness end);
//int writeInt64InBuffer(uint64_t i, uint8_t *buffer, enum endianness end);
int writeFloatInBuffer(float f, uint8_t *buffer, enum endianness end);
//int writeDoubleInBuffer(double d, uint8_t *buffer, enum endianness end);
int32_t readInt32FromBuffer(const uint8_t *buffer, enum endianness end);
//uint64_t readInt64FromBuffer(uint8_t *buffer, enum endianness end);
float readFloatFromBuffer(const uint8_t *buffer, enum endianness end);
//double readDoubleFromBuffer(uint8_t *buffer, enum endianness end);

#endif
