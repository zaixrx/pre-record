#include "types.h"
#include <stdio.h>
#include <stdlib.h>

int is_frame_queue_empty(struct frame_queue *queue) {
    return (queue->front == NULL && queue->rear == NULL);
}

void initialize_queue_frame(struct frame_queue *queue) {
    queue->front = queue->rear = NULL;
}

void enqueue_frame(struct frame_queue *queue, struct frame_buffer buffer) {
    struct frame_queue_node *node = malloc(sizeof(struct frame_queue_node));
    node->data = buffer;

    if (is_frame_queue_empty(queue)) {
        queue->front = queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
}

void dequeue_frame(struct frame_queue *queue, struct frame_buffer *buffer) {
    if (is_frame_queue_empty(queue)) return;
    *(buffer) = queue->front->data;
    
    if (queue->front == queue->rear) {
        free(queue->front);

        queue->front = queue->rear = NULL;
    } else {
        struct frame_queue_node *node = queue->front;
        queue->front = queue->front->next;
        free(node);
    }
}