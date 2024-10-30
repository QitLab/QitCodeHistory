//
// Created by A on 2024-10-26.
//

#include "VideoChannel.h"
#include "BaseChannel.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

VideoChannel::~VideoChannel() {}

void *task_video_decode(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_decode();
    return 0;
}

void *task_video_play(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_play();
    return 0;
}

void VideoChannel::start() {
    isPlaying = 1;
    //队列开始工作
    packets.setWork(1);
    frames.setWork(1);
    //第一个线程，取出队列压缩包，进行编码，编码后的原始包再push到队列中
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    //第二个线程：从队里取出原始包，播放
    pthread_create(&pid_video_play, 0, task_video_play, this);

}

void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int r = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        if (!r) {
            continue;
        }

        //新旧差别大
        r = avcodec_send_packet(codecContext, packet);
        releaseAVPacket(&packet);
        if (r) {
            break;
        }
        AVFrame *avFrame = av_frame_alloc();
        r = avcodec_receive_frame(codecContext, avFrame);
        if (r == AVERROR(EAGAIN)) {
            continue;
        } else if (r != 0) {
            break;
        }

        frames.offer(avFrame);
    }
    releaseAVPacket(&packet);
}

void VideoChannel::video_play() {
    // 原始包YUV -> android是RGB，需要libswscale
    AVFrame *frame = 0;
    uint8_t *dst_data[4];//RGBA
    int dst_linesize[4];
    av_image_alloc(
            dst_data,
            dst_linesize,
            codecContext->width,
            codecContext->height,
            AV_PIX_FMT_RGBA,
            1
    );
    SwsContext *sws_context = sws_getContext(
            codecContext->width,
            codecContext->height,
            codecContext->pix_fmt,
            codecContext->width,
            codecContext->height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL, NULL, NULL//特效不需要
    );
    while (isPlaying) {
        int r = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        if (!r) {
            continue;
        }
        sws_scale(sws_context,
                  frame->data,
                  frame->linesize,
                  0,
                  codecContext->height,
                  dst_data,
                  dst_linesize
        );
        //ANativeWindows
        renderCallback(dst_data[0],
                       codecContext->width,
                       codecContext->height,
                       dst_linesize[0]
        );
        releaseAVFrame(&frame);
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_free(&dst_data);
    sws_freeContext(sws_context);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::stop() {

}

