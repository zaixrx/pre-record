#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdint.h>

// data needs to be freed manually.
// frame_buffer should only be mainly stored in a queue.
struct frame_buffer {
  size_t size;
  uint8_t *data;
};

struct frame_queue_node {
  struct frame_buffer data;
  struct frame_queue_node *next;
};

struct frame_queue {
  struct frame_queue_node *front;
  struct frame_queue_node *rear;
};

int is_frame_queue_empty(struct frame_queue *queue);
void initialize_queue_frame(struct frame_queue *queue);
void enqueue_frame(struct frame_queue *queue, struct frame_buffer buffer);
void dequeue_frame(struct frame_queue *queue, struct frame_buffer* buffer);

#endif