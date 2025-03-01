#ifndef RLE_H
#define RLE_H

#include <stdint.h>
#include <stdio.h>

size_t rle_encode_yuyv(const uint8_t *input, size_t size, uint8_t **out);
size_t rle_decode_yuyv(struct frame_buffer* comp_buffer, struct frame_buffer* frame_buffer);
void rle_dispose();

#endif