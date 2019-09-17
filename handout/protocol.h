/* Do not modify this file */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "tag_data.h"

#define MAX_SEGMENTS 10

/* Struct Definitions */
typedef struct decode_segment_struct {
	long int data;
	int parity;
} Decode_Segment;

typedef struct decode_workbook_struct {
	long int preamble;
	Decode_Segment segment[MAX_SEGMENTS];
	long int parity_word;
	long int end;
	long int decoded_data;
} Decode_Workbook;

/* Macro Definitions
 *===========================================================
 * - Parity.  This RFID emulator uses 2D Parity Checks
 */
#define EVEN_PARITY 0 /* 1 - Even Parity, 0 - Odd Parity */


 /* - Tag ID Encoding Scheme (POR Mode Reads)
  * Format: PREAMBLE+DATA1+Parity1+...+DATAn+Parityn+ParityWord+End
  */
#define RFID_TID_PREAMBLE_BITS 9
#define RFID_TID_PREAMBLE 0x1FF
#define RFID_TID_DATA_SEGMENTS 10
#define RFID_TID_DATA_SEGMENT_BITS 4
#define RFID_TID_END_BITS 1
#define RFID_TID_END 0x0

  /* Read Command Code Information
   * Format: CMD+Parity+ADDRESS+PAD+Parity
   */
#define RFID_CMD_BITS 3 
#define RFID_CMD_CODE_READ_ADDRESS 0x4
#define RFID_ADDRESS_BITS 4
#define RFID_ADDRESS_PAD_BITS 2 
#define RFID_ADDRESS_PAD_CHAR 0 

   /* - Addressed Data Encoding Scheme (CMD_READ Mode Reads)
	* Format: PREAMBLE+DATA1+Parity1+...+DATAn+Parityn+ParityWord+End
	*/
#define RFID_ADDRESS_PREAMBLE_BITS 8
#define RFID_ADDRESS_PREAMBLE 0x0A
#define RFID_ADDRESS_DATA_SEGMENTS 4
#define RFID_ADDRESS_DATA_SEGMENT_BITS 8
#define RFID_ADDRESS_END_BITS 1
#define RFID_ADDRESS_END 0x0

#endif /* PROTOCOL_H */
