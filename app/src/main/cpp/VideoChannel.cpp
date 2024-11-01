//
// Created by A on 2024-10-26.
//

#include "VideoChannel.h"
#include "BaseChannel.h"

void dropAVFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

void dropAVPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *packet = q.front();
        if (packet->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&packet);
            q.pop();
        } else {
            break;
        }
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base,
                           double fps) : BaseChannel(
        stream_index, codecContext, time_base), fps(fps) {
    frames.setSyncCallback(dropAVFrame);
    packets.setSyncCallback(dropAVPacket);
}

void *task_video_decode(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_decode();
    return nullptr;
}

void *task_video_play(void *args) {
    auto *video_channel = static_cast<VideoChannel *>(args);
    video_channel->video_play();
    return nullptr;
}

void VideoChannel::start() {
    isPlaying = true;
    //队列开始工作
    packets.setWork(1);
    frames.setWork(1);
    //第一个线程，取出队列压缩包，进行解码，解码后的原始包再push到队列中
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);
    //第二个线程：从队里取出原始包，播放
    pthread_create(&pid_video_play, nullptr, task_video_play, this);

}

void VideoChannel::setAudioChannel(AudioChannel *channel) {
    this->audio_channel = channel;
}

void VideoChannel::video_decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        if (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
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
            //B帧会参考前后，等待P帧出来
            continue;
        } else if (r != 0) {
            if (avFrame) {
                releaseAVFrame(&avFrame);
            }
            break;
        }

        frames.offer(avFrame);
    }
    releaseAVPacket(&packet);
}

void VideoChannel::video_play() {
    // 原始包YUV -> android是RGB，需要libswscale
    AVFrame *frame = nullptr;
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
            nullptr, nullptr, nullptr//特效不需要
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
        //音视频同步
        //首先增加fps的间隔
        auto extra_delay = frame->repeat_pict / (2.0 * fps);
        auto fps_delay = 1.0 / fps;//根据fps得到每一帧消耗的时间
        auto real_delay = extra_delay + fps_delay;
        auto video_time = static_cast<double>(frame->best_effort_timestamp) * av_q2d(time_base);
        auto audio_time = audio_channel->audio_time;
        auto time_diff = video_time - audio_time;
//        qlogd("time_diff : %f", time_diff)
        if (time_diff > 0) {
            if (time_diff > 1) {
                //说明二者差距很大
                av_usleep(static_cast<int>(real_delay * 2 * 1000 * 1000));
            } else {
                //差距不大
                av_usleep(static_cast<int>((real_delay + time_diff) * 1000 * 1000));
            }
        } else if (time_diff < 0) {
            //音频 > 视频，要丢帧。 I帧不能丢
            // 从 frames和packets丢弃
            if (fabs(time_diff) <= 0.03) {
                //多线程安全
                frames.sync();
                continue;
            }
        } else {

        }

        //ANativeWindows
        renderCallback(dst_data[0],
                       codecContext->width,
                       codecContext->height,
                       dst_linesize[0]
        );
        releaseAVFrame(&frame);
    }
    releaseAVFrame(&frame);
    isPlaying = false;
    av_free(&dst_data);
    sws_freeContext(sws_context);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::stop() {
    pthread_join(pid_video_decode, nullptr);
    pthread_join(pid_video_play, nullptr);
    isPlaying = false;
    packets.setWork(0);
    frames.setWork(0);
    packets.clear();
    frames.clear();
}

VideoChannel::~VideoChannel() {
    DELETE(audio_channel)
}

