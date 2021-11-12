
#include "endianness.h"
#include <float.h>
#include <string.h>
#include <errno.h>

static enum endianness int32_machine_endianness = NOTCALCULATED_ENDIAN;
//static enum endianness int64_machine_endianness = NOTCALCULATED_ENDIAN;
static enum endianness float32_machine_endianness = NOTCALCULATED_ENDIAN;
//static enum endianness double64_machine_endianness = NOTCALCULATED_ENDIAN;

#define CALCUL_OU_RETOUR(var) {\
    if(var == NOTCALCULATED_ENDIAN)\
        calculateMachineEndianness();\
    return var;\
}
#define areReverseEndian(a,b) ((b) == LITTLE_ENDIAN_REPR && (a) == BIG_ENDIAN_REPR || (b) == BIG_ENDIAN_REPR && (a) == LITTLE_ENDIAN_REPR) 
#define _UFT(type, size) union { type value; uint8_t bytes[size]; } 
#define _COMP_4BYTES(_b_, b1, b2, b3, b4) ((_b_)[0] == (b1) && (_b_)[1] == (b2) && (_b_)[2] == (b3) && (_b_)[3] == (b4))


static void calculateMachineEndianness(void) {

    
    _UFT(int32_t, 4) i32;
    //UFT(uint64_t, 8) i64;
    _UFT(float, 4) f;
    //UFT(double, 8) d;
    
    i32.value = 0x00010203;
    if(_COMP_4BYTES(i32.bytes, 0, 1, 2, 3))
        int32_machine_endianness = BIG_ENDIAN_REPR;
    else if (_COMP_4BYTES(i32.bytes, 3, 2, 1, 0))
        int32_machine_endianness = LITTLE_ENDIAN_REPR;
    else
        int32_machine_endianness = UNRECOGNIZED_ENDIAN;

    
    f.value = FLT_MAX; // Signe=1, Exposant=1111 1110, Mantisse=111 1111 1111 1111 1111 1111
    //En découpant en 4 octets, on a du plus signifiant au moins signifiant :
    // s<---e---><-----------m----------->
    // 01111111 01111111 11111111 11111111
    int32_t fb0 = 0b01111111;
    int32_t fb1 = 0b01111111;
    int32_t fb2 = 0b11111111;
    int32_t fb3 = 0b11111111;
    if(_COMP_4BYTES(f.bytes, fb0, fb1, fb2, fb3))
        float32_machine_endianness = BIG_ENDIAN_REPR;
    else if(_COMP_4BYTES(f.bytes, fb3, fb2, fb1, fb0))
        float32_machine_endianness = LITTLE_ENDIAN_REPR;
    else
        float32_machine_endianness = UNRECOGNIZED_ENDIAN;
}


int getInt32End(void) { CALCUL_OU_RETOUR(int32_machine_endianness); }

int getFloatEnd(void) { CALCUL_OU_RETOUR(float32_machine_endianness); }

int writeInt32InBuffer(int32_t i, uint8_t *buffer, enum endianness end) {
    if(end == MACHINE_ENDIAN || end == getInt32End())
        memmove(buffer, &i, 4);
    else if ( areReverseEndian(getInt32End(), end) ) {
        _UFT(int32_t, 4) i32;
        i32.value = i;
        for(int j=3; j>=0;j--, buffer)
            *(buffer++) = i32.bytes[j];
    } else {
        return INVALID_ENDIAN_ARG;
    }
    
    return 0;
}

int writeFloatInBuffer(float f, uint8_t *buffer, enum endianness end) {
    if(end == MACHINE_ENDIAN || end == getFloatEnd()) {
        memmove(buffer, &f, 4);
    } else if (areReverseEndian(getFloatEnd(), end)) {
        _UFT(float, 4) f32;
        f32.value = f;
        for (int j=3; j>=0; j--)
            *(buffer++) = f32.bytes[j];
    } else {
        return INVALID_ENDIAN_ARG;
    }
    
    return 0;  
}

int32_t readInt32FromBuffer(const uint8_t *buffer, enum endianness end) {
    _UFT(int32_t, 4) i32;
    if(end == MACHINE_ENDIAN || end == getInt32End())
        memmove(i32.bytes, buffer, 4);
    else if (areReverseEndian(getInt32End(), end)) {
        for(int j=3; j>=0; j--, buffer++)
            i32.bytes[j] = *buffer;
    } else {
        errno = INVALID_ENDIAN_ARG;
        return INVALID_ENDIAN_ARG;
    }
    return i32.value;
}

float readFloatFromBuffer(const uint8_t *buffer, enum endianness end) {
    _UFT(float, 4) f32;
    if(end == MACHINE_ENDIAN || end == getFloatEnd())
        memmove(f32.bytes, buffer, 4);
    else if (areReverseEndian(getFloatEnd(), end)) {
        for(int j=3; j>=0; j--, buffer++)
            f32.bytes[j] = *buffer;
    } else {
        errno = INVALID_ENDIAN_ARG;
        return INVALID_ENDIAN_ARG;
    }
    return f32.value;
}


/*int main(void) {
    uint8_t buf[4], buf2[4], buf3[4], buf4[4];
    writeInt32InBuffer(1, buf, MACHINE_ENDIAN);
    writeFloatInBuffer(FLT_MAX, buf2, LITTLE_ENDIAN_REPR);
    writeFloatInBuffer(FLT_MAX, buf3, BIG_ENDIAN_REPR);
    writeFloatInBuffer(FLT_MAX, buf4, MACHINE_ENDIAN);
    int printf(char *format, ...);
    printf("%d, %d\n", readInt32FromBuffer(buf, MACHINE_ENDIAN), readInt32FromBuffer(buf, BIG_ENDIAN_REPR));
    printf("%f, %f, %f\n", readFloatFromBuffer(buf2, LITTLE_ENDIAN_REPR), readFloatFromBuffer(buf3, BIG_ENDIAN_REPR), readFloatFromBuffer(buf4, MACHINE_ENDIAN));
}*/
