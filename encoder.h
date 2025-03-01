#ifndef ENCODER_H
#define ENCODER_H

#include "types.h"
#include <stdio.h>

void encode_yuyv_to_h264(struct frame_queue *queue, int width, int height, FILE *output_file);

#endif