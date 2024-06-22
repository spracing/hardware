//
// Generates IR diode bitstream for Robitronic IR transponder
//  
// Assuming little endian data format on given processor
// if CPU uses big endian ID needs to be converted to big endian format

#include <stdio.h>
#include <cstdint>

#define ID 80175

//
// calculate CRC-8 with polynom 0x07 and init crc 0x00
//
static uint8_t crc8(uint8_t *data, uint8_t nBytes) {
  uint8_t crc = 0x00; 

  for(uint8_t i = 0 ; i < nBytes;) {
    crc ^= data[i++]; 

		for(uint8_t n = 0; n < 8; n++) {
			if ((crc & 0x80) != 0)
				crc = (crc << 1) ^ 0x07;
			else
				crc <<= 1;
    }
	}

	return crc;
}

// 
// generate 44 bit sequence for given ID
//
static uint64_t generateBitStream(uint32_t id) {
	uint8_t byte;
	uint64_t mask;

	id |= crc8((uint8_t *)&id, 3) << 24;
  id ^= 0xffffffff;

	uint64_t bitStream = 0;

	for(uint8_t i = 0; i < 4; i++) {
		// start bit
		bitStream |= 1; bitStream <<= 1;

		byte = ((uint8_t *)&id)[i];
		mask = 0x80;

		// 8 data bits
		for(uint8_t n = 0; n < 8; n++) {
			if((byte & mask))
				bitStream |= 1;
			bitStream <<= 1;

			mask >>= 1;
		}

		// parity bit
		if(__builtin_popcount(byte) % 2 == 0)
			bitStream |= 1;
		bitStream <<= 1;

		//stop bit
		bitStream <<= 1;
	}
	bitStream >>= 1;

	return bitStream;
}

//
// show 44 data bits
//
static void showBitStream(uint32_t id, uint64_t bitStream) {
	printf("44 bit sequence for ID %d to be transmitted via IR\n", id);

	uint64_t mask = 0x100000000000;	// 1 << 44

	for(uint8_t n = 0; n < 44; n++) {
		mask >>= 1;
		
		if(bitStream & mask)
			printf("1");
		else
			printf("0");
	}

	printf("\n");
	printf("encode 0: 10us IR off\n");
	printf("encode 1: 2us IR on, 8us IRoff\n");
	printf("1ms to 4.5ms off after packet transmission\n");	
}

//
// main
//
int main(int argc, char **argv) {
	uint64_t bitStream = generateBitStream(ID); 

	showBitStream(ID, bitStream);
}