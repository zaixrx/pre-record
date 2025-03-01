#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "types.h"

#define WIDTH 640
#define HEIGHT 480
#define PIXEL_FORMAT V4L2_PIX_FMT_YUYV

static int fd = -1;
static uint8_t *buffer;
static size_t buffer_size;

int initialize_cam() {
    fd = open("/dev/video0", O_RDWR);
    if (fd == -1) {
        perror("Failed to open /dev/video0");
        return -1;
    }

    struct v4l2_format fmt = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .fmt.pix.width = WIDTH,
        .fmt.pix.height = HEIGHT,
        .fmt.pix.pixelformat = PIXEL_FORMAT,
        .fmt.pix.field = V4L2_FIELD_NONE
    };

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Failed to set format");
        close(fd);
        return -1;
    }

    buffer_size = fmt.fmt.pix.sizeimage;
    buffer = malloc(buffer_size * sizeof(int8_t));
    if (!buffer) {
        perror("Failed to allocate user buffer");
        close(fd);
        return -1;
    }

    struct v4l2_requestbuffers req = {
        .count = 1,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_USERPTR
    };

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Failed to request buffer");
        free(buffer);
        close(fd);
        return -1;
    }

    struct v4l2_buffer buf = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_USERPTR,
        .index = 0,
        .m.userptr = (unsigned long) buffer,
        .length = buffer_size
    };

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Failed to queue buffer");
        free(buffer);
        close(fd);
        return -1;
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Failed to start streaming");
        free(buffer);
        close(fd);
        return -1;
    }

    return 0;
}

int get_frame(struct frame_buffer *dest_buffer) {
    struct v4l2_buffer buf = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_USERPTR
    };

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Failed to dequeue buffer");
        return -1;
    }

    dest_buffer->size = buffer_size;
    dest_buffer->data = malloc(buffer_size * sizeof(int8_t));

    memcpy(dest_buffer->data, buffer, buffer_size);

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Failed to queue buffer");
        return -1;
    }

    return 0;
}

int dispose_cam() {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    free(buffer);
    close(fd);

    return 0;
}