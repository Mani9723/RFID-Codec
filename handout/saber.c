/* Do not modify this file.
 *----------..................................................................
 * This is the main RFID Codec (Encoder/Decoder) Driver Project File
 *   This program tests your Codec (codec.c) at decoding and encoding data
 *   from an RFID Tag (RFID chip).
 *
 * This program activates the RFID module, reads the Tag ID (TID) and then
 *   calls your Codec function to decode it.  If the read was good, this
 *   will then call your Codec function to encode a command to read from
 *   a provided address and send it to the RFID Tag.  After this, it will
 *   read the reply from the Tag and send that reply to your Codec for
 *   decoding.  At the end, if your data is decoded properly, it will
 *   print out the information and match it to a known Tag.
 *
 * You will be completing the functions in codec.c (prototypes in codec.h)
 *   to encode and decode data from an RFID Tag.  All the reading/writing
 *   to the tag is provided for you in this file.  All you need to do is
 *   write the functions for encoding and decoding data.
 *
 * *The data is based on the EM4305 RFID tag as used in the Disney(tm)
 *    sold lightsaber crystals at Star Wars Galaxy's Edge (Trademark of
 *    Lucasfilm)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <strings.h>
#include <errno.h>

#include "saber.h"
#include "tag_data.h"
#include "rfid.h"
#include "protocol.h"
#include "codec.h"

 /* Local Functions */
static int run_program(int mode);

/* Returns the color ID for the inserted crystal
 * Run without arguments for development.
 */
int main(int argc, char* argv[]) {
	int mode = 1;
	int result;

	/* Set the Mode (Use -1 for no parity errors) */
	if (argc == 2) {
		/* Convert first arg to a long. */
		errno = 0;
		mode = strtol(argv[1], NULL, 10);
		if (errno != 0) {
			printf("There was an error processing the mode value.\n");
			printf("Usage: %s <mode>\n", argv[0]);
			printf("Example: %s 1\n", argv[0]);
			printf("Exiting\n");
			exit(-1);
		}
	}
	/* Default mode */
	else {
		mode = 1;
	}

	/* Runs the program using the selected mode */
	result = run_program(mode);

	return result;
}

static int is_tid_valid(long int tid) {
	int i;
	for (i = 0; i < TAG_COUNT; i++) {
		if (tag_tids[i] == tid) {
			return 1;
		}
	}
	return 0;
}

static int is_data_valid(long int data) {
	int i;
	for (i = 0; i < TAG_COUNT; i++) {
		if (tag_data[i] == data) {
			return 1;
		}
	}
	return 0;
}

static int get_index(long int tid, long int data) {
	int i;
	for (i = 0; i < TAG_COUNT; i++) {
		if (tag_tids[i] == tid && tag_data[i] == data) {
			return i;
		}
	}
	return -1;
}

static int run_program(int mode) {
	Decode_Workbook* workbook = NULL; /* For decode operations */
	long int raw_data = 0; /* Raw RX/TX data stream */
	long int tag_address_data = 0;
	long int tag_id = 0;
	int attempts_left = 0;
	int tag_index = 0;
	int is_error = 0; /* Error Checking RFID Calls */
	int is_valid = 0; /* Validating Codec Data */
	/* Initialize Data Storage
	 * - workbook will contain the preamble, data segments + parity,
	 *   the parity word and the final end.  It will also contain
	 *   the decoded data.
	 */
	workbook = malloc(sizeof(Decode_Workbook));

	/* Step 0: Activate the RFID Reader */
	rfid_initialize(mode); /* Initialize the Reader */
	rfid_activate(&is_error); /* Send power to the RFID Tag */
	if (is_error == 1) {
		printf("Error: Failed to Activate the RFID Tag.  Exiting.\n");
		exit(-1);
	}

	/* Step 1: Read the Power On Response TID Data (until valid) */
	printf(".------\n");
	printf("| Starting to read the TID from the RFID TAG...\n");
	attempts_left = MAX_READ_TRIES;
	do {
		attempts_left--;
		if (attempts_left == 0) {
			printf("| Error: Exhausted Read Attempts. Exiting.\n");
			exit(-1);
		}
		/* Read the raw data from the Tag */
		rfid_read(&raw_data, &is_error);
		if (is_error == 1) {
			printf("| Error: RFID Reader failed to read.  Exiting.\n");
			exit(-1);
		}

		printf("| Read raw_data = 0x%010lx\n", raw_data);
		printf("| ... sending raw_data into codec_decode_tid...\n");

		/* Decode the raw data and fill in the worksheet. */
		codec_decode_tid(raw_data, workbook, &is_valid);
		/* Check for valid return */
		if (is_valid == 0) {
			printf("| Warning: Parity Failure on Read, trying again.\n");
		}
	} while (is_valid == 0);

	printf("| Tag ID:   0x%010lx\n", workbook->decoded_data);
	printf("| Tag ID is %s\n", is_tid_valid(workbook->decoded_data) ? "Valid" : "Invalid");
	tag_id = workbook->decoded_data;
	printf("\\----\n");

	/* Step 2: Encode the Read Address Command and Send it, */
	printf(".------\n");
	printf("| Starting to encode the command code\n");
	raw_data = 0;
	printf("| Encoding Command Code = 0x%x and Address = %d\n", RFID_CMD_CODE_READ_ADDRESS, 6);
	codec_encode_read(RFID_CMD_CODE_READ_ADDRESS, 6, &raw_data);
	attempts_left = MAX_READ_TRIES;
	do {
		attempts_left--;
		if (attempts_left == 0) {
			printf("| Error: Exhausted Send Attempts. Exiting.\n");
			exit(-1);
		}
		printf("| Sending raw_data = 0x%03lx\n", raw_data);
		rfid_send(raw_data, &is_error);
	} while (is_error == 1);
	printf("\\----\n");

	/* Step 3: Read the Address Response Data (until valid) */
	printf(".------\n");
	printf("| Starting to read the Data from the RFID TAG...\n");
	bzero(workbook, sizeof(Decode_Workbook));
	raw_data = 0;
	attempts_left = MAX_READ_TRIES;
	do {
		attempts_left--;
		if (attempts_left == 0) {
			printf("| Error: Exhausted Read Attempts. Exiting.\n");
			exit(-1);
		}
		/* Read the raw data from the Tag */
		rfid_read(&raw_data, &is_error);
		if (is_error == 1) {
			printf("| Error: RFID Reader failed to read.  Exiting.\n");
			exit(-1);
		}

		printf("| Read raw_data = 0x%010lx\n", raw_data);
		printf("| ... sending raw_data into codec_decode_data...\n");

		/* Decode the raw data and fill in the worksheet. */
		codec_decode_data(raw_data, workbook, &is_valid);
		/* Check for valid return */
		if (is_valid == 0) {
			printf("| Warning: Parity Failure on Read, trying again.\n");
		}
	} while (is_valid == 0);

	printf("| Tag Data: 0x%08lx\n", workbook->decoded_data);
	printf("| Tag Data is %s\n", is_data_valid(workbook->decoded_data) ? "Valid" : "Invalid");
	tag_address_data = workbook->decoded_data;
	printf("\\----\n");

	/* Step 4: Deactivate the RFID Reader */
	rfid_deactivate(&is_error);
	if (is_error == 1) {
		printf("Error: Failed to Deactivate the RFID Tag.  Exiting.\n");
		exit(-1);
	}

	/* Step 5: Identify the Tag */
	tag_index = get_index(tag_id, tag_address_data);

	printf("\n.----\n");
	if (tag_index == -1) {
		printf("| Error: Tag Unrecognized. Exiting.\n");
		exit(-1);
	}

	printf("| Tag Recognized.\n| Color = %s, Voice = %s\n", tag_colors[tag_index],
		tag_voices[tag_index]);
	printf("\\----\n");
	/* Clean Data Storage; */
	free(workbook);
	workbook = NULL;

	return tag_index;
}
