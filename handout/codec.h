/* Do not modify this file */
#ifndef CODEC_H
#define CODEC_H

#include "protocol.h"

void codec_decode_tid(long int raw_data, Decode_Workbook* workbook, int* is_valid);
void codec_decode_data(long int raw_data, Decode_Workbook* workbook, int* is_valid);
void codec_encode_read(int command_code, int address_code, long int* raw_data);

#endif /* CODEC_H */
