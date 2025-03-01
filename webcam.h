#ifndef WEBCAM_H
#define WEBCAM_H

#include "types.h"

int initialize_cam();
int dispose_cam();
int get_frame(struct frame_buffer *dest_buffer);

#endif