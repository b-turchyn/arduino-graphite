#ifndef EEPROMx_h
#define EEPROMx_h

#include <inttypes.h>

class EEPROMxClass
{
  public:
    void writeToEEPROM( char* input, int startPosition );
    void writeToEEPROM( char* input );
    void readFromEEPROM( char* result, int startPosition, int length );
    void readFromEEPROM( char* result, int length );
    uint8_t read(int);
    void write(int, uint8_t);
};
extern EEPROMxClass EEPROMx;
#endif
