/* Fill in your Name and GNumber in the following two comment fields
 * Name: Mani Shah
 * GNumber: 00974705
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "codec.h"
#include "protocol.h"
#include "tag_data.h"

/*
This method gets a particular byte of data from the raw_data used in the decoding TID and data processes
*/
long int getData(int *startIndex, int maxBit, unsigned long int *mask, long int raw_data);
/*
This method calculates the parity of the parity word in both decoding TID and data processes
*/
int calcParityWord(Decode_Workbook *book,int col,int row);
/*
This method calculates the parity bit of a given byte.
*/
int calcParityBit(int number, int bits);

/* Read the Project Document for all of the instructions.
 * You may add any functions that you like to this file.
 * - Recommendations: Add a function to calculate your parity and a function 
 *   to get bit X from an int.
 *
 * Decode to extract the Tag ID (TID) according to the project documentation
 * Fill in workbook completely for full credit.
 * If any parity bits are wrong, set *is_valid to 0 and return immediately.
 *   Else, set *is_valid to 1 and return when workbook is completely filled in.
 */
void codec_decode_tid(long int raw_data, Decode_Workbook *workbook, int *is_valid) {
  /* Complete this Function */
	int i = 0, j, bitLen = 1;
	unsigned long int mask = 0x01;
	long int preamble =0, end, currParity,actualParity,currData=0;
	long int oneCount = 0, parShiftValue = 5, decodeShiftValue = 0, shiftValue =6;

	// Get the end value and verify
	end = getData(&i,bitLen,&mask, raw_data);
	if(end != 0){
		*is_valid = 0;
		return;
	}
	workbook->end = end;
	bitLen += 4;
	workbook->parity_word = getData(&i,bitLen,&mask, raw_data) >> 1;	

	// Go throught the whole segment data
	for(j = 0; j < 10; j++){
		bitLen += 1;
		// gets the parity bit of each segment data
		currParity = getData(&i,bitLen,&mask,raw_data)>>parShiftValue;
		bitLen += 4;
		// gets the following byte of segment data
		for(i = i; i < bitLen; i++){
			if((raw_data & mask) != 0){
				oneCount++;
			}
			currData += (raw_data & mask)>>shiftValue;
			mask = mask << 1;
		}
		//Verify the parity of each segment data
		actualParity = oneCount%2;
		if(actualParity != currParity){
			*is_valid = 0;
			return;
		} 
		//Once verified, store in workbook
		workbook->segment[10-(j+1)].parity = currParity;
		workbook->segment[10-(j+1)].data = currData;
		workbook->decoded_data += workbook->segment[10-(j+1)].data << decodeShiftValue;
		currData = 0; oneCount = 0;
		shiftValue+=5; parShiftValue+=5;		
		decodeShiftValue += 4;
		
	}
	// Verfiy the given parity word
	if(calcParityWord(workbook,4,10) == 0){
		*is_valid = 0;
		return;
	}
	bitLen += 9;
	//Get the preamble and verify the preamble as well
	for(i = i; i < bitLen; i++){
		if((raw_data & mask) == 0){
			*is_valid = 0;
			return;
		}
		preamble += (raw_data & mask) >> (shiftValue - 1);
		mask = mask << 1;
	}
 	
	workbook->preamble = preamble;
	*is_valid = 1;
}
/*
Calculates the parity word of the decoded segments in both TID and decode_data processes
*/
int calcParityWord(Decode_Workbook *book, int col, int row)
{
	int i, j, mask = 0x01, parity=0, oneCount = 0;
	for(i = 0; i < col; i++){
		for(j = 0; j < row; j++){
			if((book->segment[j].data & mask) != 0){
				++oneCount;
			}	
		}
		parity += (oneCount%2) << i;
		mask = mask << 1; oneCount = 0;
	}	
	
	return parity == book->parity_word ? 1 : 0;
}
/*
Gets a particular byte of segment data from the raw_data
*/
long int getData(int *startIndex, int maxBit, unsigned long int *mask, long int rawData)
{
	int i;
	long int value = 0;
	for(i = *startIndex; i < maxBit; i++){
		value += rawData & *mask;
		*mask = *mask << 1;
	}
	*startIndex = i;
	return value;
}

/* Encodes the fields with parity bits and combines them according to the 
 * project documentation
 */
void codec_encode_read(int command_code, int address_code, long int *raw_data) {
  /* Complete this Function */
	int i, shiftVal = 3, mask = 0x01;
	//Parity of the address_code
	*raw_data += calcParityBit(address_code,4);
	//Calculate the rest of the bits
	for(i = 0; i < 8; i++){
		if(i < 4){//Address_code section
			*raw_data += ((address_code & mask) << shiftVal);
			mask = mask << 1;
		}else if(i == 4){//Parity of the command_code
			mask = 0x01;
			shiftVal += 4;
			*raw_data += (calcParityBit(command_code,3) << shiftVal);
			shiftVal++;
		}else{//command_code section
			*raw_data += ((command_code & mask) << shiftVal);
			mask = mask << 1;
		}
	}
}
/*
Calculate the parity of any given number
*/
int calcParityBit(int number, int bits)
{
	int i,mask = 0x01,oneCount=0;
	for(i = 0; i < bits; i++){
		if((number & mask) != 0){
			oneCount++;
		}
		mask = mask << 1;
	}
	return oneCount % 2 == 0 ? 0 : 1;
}

/* Decodes to extract the Addressed Data according to the project documentation
 * Fill in workbook completely for full credit.
 * If any parity bits are wrong, set *is_valid to 0 and return immediately
 *   Else, set *is_valid to 1 and return when workbook is completely filled in.
 */
void codec_decode_data(long int raw_data, Decode_Workbook *workbook, int *is_valid) {
  /* Complete this Function */
	int i = 0, j, bitLen = 1;
	unsigned long int mask = 0x01;
	long int preamble =0, end, currParity,actualParity,currData=0;
	long int oneCount = 0, parShiftValue = 9, decodeShiftValue = 0, shiftValue =10;
	// Gets the end data and verfies at the same time
	end = getData(&i,bitLen,&mask, raw_data);
	if(end != 0){
		*is_valid = 0;
		return;
	}
	workbook->end = end;
	bitLen += 8;
	workbook->parity_word = getData(&i,bitLen,&mask, raw_data) >> 1;	
	// Work on the rest of the bits
	for(j = 0; j < 4; j++){
		bitLen += 1;
		//Get the parity of each data segment
		currParity = getData(&i,bitLen,&mask,raw_data)>>parShiftValue;
		bitLen += 8;
		// Get the each data segment
		for(i = i; i < bitLen; i++){
			if((raw_data & mask) != 0){
				oneCount++;
			}
			currData += (raw_data & mask)>>shiftValue;
			mask = mask << 1;
		}
		// verify the parity of each data segment
		actualParity = oneCount%2;
		if(actualParity != currParity){
			*is_valid = 0;
			return;
		} 
		workbook->segment[4-(j+1)].parity = currParity;
		workbook->segment[4-(j+1)].data = currData;
		workbook->decoded_data += workbook->segment[4-(j+1)].data << decodeShiftValue;
		currData = 0; oneCount = 0;
		shiftValue+=9; parShiftValue+=9;		
		decodeShiftValue += 8;
		
	}
	// Calculate and verify the parity word against the given parity word
	if(calcParityWord(workbook,8,4) == 0){
		*is_valid = 0;
		return;
	}
	bitLen += 9;
	// Gather and verify the premable
	for(i = i; i < bitLen; i++){
		preamble += (raw_data & mask) >> 45;
		mask = mask << 1;
	}

	if(preamble != 10){
		*is_valid = 0;
		return;
	}
	workbook->preamble = preamble;
	*is_valid = 1;
}
