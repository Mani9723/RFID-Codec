/* Do not modify this file. */
#ifndef RFID_H
#define RFID_H

#include "tag_data.h"
/* RFID Library Functions */
void rfid_initialize(int mode);
void rfid_activate(int* is_error);
void rfid_deactivate(int* is_error);
void rfid_read(long int* raw_data, int* is_error);
void rfid_send(long int raw_data, int* is_error);

#endif /* RFID_H */
