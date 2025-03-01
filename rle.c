#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

static uint8_t *compression_buffer = NULL;

// returns the size of the encoded buffer
int rle_encode_yuyv(struct frame_buffer* frame_buffer, struct frame_buffer* comp_buffer) {
  if (!compression_buffer)
    compression_buffer = malloc(1.5 * frame_buffer->size * sizeof(uint8_t));;
  
  int j = 0;
  size_t i = 0, k = 0;
  while (i < frame_buffer->size && k < 2 * frame_buffer->size) {
    j = 1;

    uint8_t y0 = frame_buffer->data[i];
    uint8_t u = frame_buffer->data[i + 1];
    uint8_t y1 = frame_buffer->data[i + 2];
    uint8_t v = frame_buffer->data[i + 3];
    
    while (i + j * 4 < frame_buffer->size && j < 255 &&
      frame_buffer->data[i + j * 4] == y0 &&
      frame_buffer->data[i + j * 4 + 1] == u &&
      frame_buffer->data[i + j * 4 + 2] == y1 &&
      frame_buffer->data[i + j * 4 + 3] == v)
    {
      j++;
    }

    compression_buffer[k] = j;
    compression_buffer[k + 1] = y0;
    compression_buffer[k + 2] = u;
    compression_buffer[k + 3] = y1;
    compression_buffer[k + 4] = v;
    
    k += 5;
    i += j * 4;
  }

  comp_buffer->data = malloc(k * sizeof(uint8_t));
  memcpy(comp_buffer->data, compression_buffer, k);

  return k;
}

// returns the size of the decoded buffer
size_t rle_decode_yuyv(struct frame_buffer* comp_buffer, struct frame_buffer* frame_buffer) {
  int j = 0;
  size_t i = 0, k = 0;
  while (i < comp_buffer->size && k < frame_buffer->size) {
    j = comp_buffer->data[i];

    while (j > 0) {
      // Copy in Y0 U Y1 V
      frame_buffer->data[k] = comp_buffer->data[i + 1];
      frame_buffer->data[k + 1] = comp_buffer->data[i + 2];
      frame_buffer->data[k + 2] = comp_buffer->data[i + 3];
      frame_buffer->data[k + 3] = comp_buffer->data[i + 4];

      k += 4;
      j--;
    }

    i += 5;
  }

  return k;
}

void rle_dispose() {
  if (compression_buffer) free(compression_buffer);
}