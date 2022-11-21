#pragma once

#include <cinttypes>

extern void dump_data(const char *tag, const uint8_t * data, int len);
extern int    to_json(uint8_t * in_buffer, int in_len, char * out_buffer, int out_len);