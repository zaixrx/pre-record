#include <libavcodec/codec.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

#include "types.h"

void encode_yuyv_to_h264(struct frame_queue *queue, int width, int height, FILE *output_file) {
    AVCodecContext *ctx = NULL;
    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;
    int ret;

    // Find H.264 encoder
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        perror("Codec not found\n");
        return;
    }

    // Create codec context
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        perror("Could not allocate codec context\n");
        return;
    }

    // Set codec parameters
    ctx->bit_rate = 400000;
    ctx->width = width;
    ctx->height = height;
    ctx->time_base = (AVRational){1, 30}; // 30 FPS
    ctx->framerate = (AVRational){30, 1};
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // Open codec
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        perror("Could not open codec\n");
        return;
    }

    // Allocate packet and frame
    pkt = av_packet_alloc();
    frame = av_frame_alloc();
    if (!pkt || !frame) {
        perror("Could not allocate packet or frame\n");
        return;
    }

    // Set up frame parameters
    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;
    if (av_frame_get_buffer(frame, 32) < 0) {
        perror("Could not allocate frame buffer\n");
        return;
    }

    // Create scaling context (YUYV422 → YUV420P)
    struct SwsContext *sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_YUYV422,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, NULL, NULL, NULL);

    if (!sws_ctx) {
        perror("Could not create SwsContext\n");
        return;
    }

    int i = 0;
    struct frame_buffer buffer = {0};
    dequeue_frame(queue, &buffer);

    while (!is_frame_queue_empty(queue)) {
        const uint8_t *src_data[1] = {(uint8_t *)buffer.data};
        int src_linesize[1] = {2 * width}; // YUYV422 line size

        // Convert frame
        sws_scale(sws_ctx, src_data, src_linesize, 0, height, frame->data, frame->linesize);

        // Encode frame
        frame->pts = i;
        ret = avcodec_send_frame(ctx, frame);
        if (ret < 0) {
            perror("Error sending frame to encoder\n");
            break;
        }

        // Retrieve encoded packets
        while (ret >= 0) {
            ret = avcodec_receive_packet(ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0) {
                perror("Error encoding frame\n");
                return;
            }

            // Write encoded data to file
            fwrite(pkt->data, 1, pkt->size, output_file);
            av_packet_unref(pkt);
        }

        dequeue_frame(queue, &buffer);
        i++;
    }

    // Flush the encoder
    avcodec_send_frame(ctx, NULL);
    while (avcodec_receive_packet(ctx, pkt) == 0) {
        fwrite(pkt->data, 1, pkt->size, output_file);
        av_packet_unref(pkt);
    }

    // Clean up
    sws_freeContext(sws_ctx);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
}
