#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"
#include "EEPROMx.h"

void EEPROMxClass::writeToEEPROM( char* input, int startPosition )
{
  if ( input != NULL )
  {
    int length = strlen( input );

    for ( int i = 0; i < length; i++ )
    {
      EEPROMx.write( startPosition + i, input[ i ] );
    }
    EEPROMx.write( startPosition + length, NULL );
  }
}

void EEPROMxClass::writeToEEPROM( char* input )
{
  writeToEEPROM( input, 0 );
}

void EEPROMxClass::readFromEEPROM( char* result, int startPosition, int length )
{
  if ( result == NULL )
  {
    result = (char*) malloc ( sizeof ( char ) * length );
  }

  for ( int i = 0; i < length; i++ )
  {
    result [ i ] = EEPROMx.read( startPosition + i );
  }
  result[ length ] = 0;
}

void EEPROMxClass::readFromEEPROM( char* result, int length )
{
  readFromEEPROM( result, 0, length );
}

uint8_t EEPROMxClass::read(int address)
{
	return eeprom_read_byte((unsigned char *) address);
}

void EEPROMxClass::write(int address, uint8_t value)
{
	eeprom_write_byte((unsigned char *) address, value);
}

EEPROMxClass EEPROMx;
