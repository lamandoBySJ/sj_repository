#include <MFRC522.h>

void MFRC522_UART::PCD_WriteRegister(MFRC522::PCD_Register reg,		///< The register to write to. One of the PCD_Register enums.
									byte value		///< The value to write.
                                    )
{
	//_serial.print("test serial0");
	byte buff[2]={reg,value};
	byte tmp=0;
	
	
	
	while(tmp!=buff[0]){
		while(_serial.available()>0){
		_serial.read();
		}
		_serial.flush();
		_serial.write(buff,2);
		delay(5);
		tmp=_serial.read();
	}
	
} // End PCD_WriteRegister()

	/**
 * Writes a number of bytes to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
void MFRC522_UART::PCD_WriteRegister(MFRC522::PCD_Register  reg,		///< The register to write to. One of the PCD_Register enums.
									byte count,		///< The number of bytes to write to the register
									byte *values	///< The values to write. Byte array.
								) {
							// MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
	
	
	for (byte index = 0; index < count; index++) {
		PCD_WriteRegister(reg, values[index]	);
	}

} // End PCD_WriteRegister()

	
/**
 * Reads a byte from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
byte MFRC522_UART::PCD_ReadRegister(MFRC522::PCD_Register  reg	///< The register to read from. One of the PCD_Register enums.
								) {
	
	
	byte value;
	_serial.flush();
	delay(5);
	while(_serial.available()>0){
		_serial.read();
	}
	
	
	_serial.write( 0x80 | reg);
	delay(5);
	//Serial1.print( "Address: ");
	//Serial1.println( 0x80 | reg,HEX);
	while(!(_serial.available()>0));
	
	value = _serial.read();
	//Serial1.print( "value: ");
	//Serial1.println( value,HEX);
	
	// Read the value back. Send 0 to stop reading.
	return value;
} // End PCD_ReadRegister()

/**
 * Reads a number of bytes from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
void MFRC522_UART::PCD_ReadRegister(MFRC522::PCD_Register  reg,		///< The register to read from. One of the PCD_Register enums.
								byte count,		///< The number of bytes to read
								byte *values,	///< Byte array to store the values in.
								byte rxAlign	///< Only bit positions rxAlign..7 in values[0] are updated.
								) {
									
	
	
	if (count == 0) {
		return;
	}
	_serial.flush();
	delay(5);
	while(_serial.available()>0){
		_serial.read();
	}
	
	byte address = 0x80 | reg;				// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
	byte index = 0;			
	
	
	_serial.write( 0x80 | reg);
	delay(5);
	//Serial1.print( "Address: ");
	//Serial1.println( 0x80 | reg,HEX);
	while(!(_serial.available()>0));
	
	
	
	while (index<count) {
		if (index == 0 && rxAlign) {		// Only update bit positions rxAlign..7 in values[0]
			// Create bit mask for bit positions rxAlign..7
			byte mask = 0;
			for (byte i = rxAlign; i <= 7; i++) {
				mask |= (1 << i);
			}
			// Read value and tell that we want to read the same address again.
			byte value = _serial.read();
			delay(5);
			_serial.write( 0x80 | reg);
			delay(5);
			// Apply mask to both current value of values[0] and the new data in value.
			values[0] = (values[index] & ~mask) | (value & mask);
		}
		else { // Normal case
			values[index] = _serial.read();
			delay(5);
			_serial.write( 0x80 | reg);
			delay(5);
			while(!(_serial.available()>0));
		}
		index++;
	}
} // End PCD_ReadRegi

bool MFRC522_UART::PCD_Init() {
	_serial.begin(9600);
	_serial.flush();
        // Set the chipSelectPin as digital output, do not select the slave yet

        // Set the resetPowerDownPin as digital output, do not reset or power down.
        pinMode(_resetPowerDownPin, OUTPUT);

        if (digitalRead(_resetPowerDownPin) == LOW) {   //The MFRC522 chip is in power down mode.
                digitalWrite(_resetPowerDownPin, HIGH);         // Exit power down mode. This triggers a hard reset.
                // Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74�s. Let us be generous: 50ms.
                delay(50);
		return true;
        }
	return false;
};