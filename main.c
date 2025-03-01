#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "webcam.h"
#include "rle.h"
#include "types.h"
#include "encoder.h"

#define FPS 30
#define OUTPUT "video.h264"

int record_frame();
void dispose();

struct frame_queue *queue;

int main(void) {
  int frames_count = 0;

  if (initialize_cam() != 0) {
    return 1;
  }

  queue = malloc(sizeof(struct frame_queue));

  while (frames_count < 30) {
    if (record_frame() == 0) {
      frames_count++;
    }

    usleep(100000);
  }

  rle_dispose();
  dispose_cam();

  FILE *output_file = fopen(OUTPUT, "wb");
  if (!output_file) {
      perror("Could not open output file\n");
      return 1;
  }

  int width = 640, height = 480;
  encode_yuyv_to_h264(queue, width, height, output_file);
  
  fclose(output_file);
  dispose();

  return 0;
}

int record_frame() {
  struct frame_buffer buffer = {0};

  if (get_frame(&buffer) == 0) {
    enqueue_frame(queue, buffer);

    return 0;
  }

  return -1;
}

void dispose() {
  struct frame_buffer buffer = {0};

  while (!is_frame_queue_empty(queue)) {
    dequeue_frame(queue, &buffer);
    free(buffer.data);
  }
  free(queue);
}