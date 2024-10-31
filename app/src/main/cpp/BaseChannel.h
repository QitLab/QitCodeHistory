//
// Created by A on 2024-10-29.
//

#ifndef C__LEARNING_BASECHANNEL_H
#define C__LEARNING_BASECHANNEL_H
extern "C" {
#include "libavcodec/avcodec.h"
};

#include "safe_queue.h"
#include "qlog.h"

class BaseChannel {
private:
public:
    int stream_index;// 音频 或 视频的下标
    SafeQueue<AVPacket *> packets;//压缩的数据包
    SafeQueue<AVFrame *> frames;//原始的数据包
    bool isPlaying;//是否正在播放
    AVCodecContext *codecContext = 0;//解码器上下文

    BaseChannel(int stream_index, AVCodecContext *codecContext) : stream_index(stream_index),
                                                                  codecContext(codecContext) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    ~BaseChannel(){
        packets.clear();
        frames.clear();
    }

    /**
     * 释放队列中所有AVPacket
     * @param p
     */
    static void releaseAVPacket(AVPacket ** p){
        if(p){
            av_packet_free(p);
            *p = 0;
        }
    }
    /**
     * 释放队列中所有AVFrame
     * @param f
     */
    static void releaseAVFrame(AVFrame ** f){
        if(f){
            av_frame_free(f);
            *f = 0;
        }
    }


};

#endif //C__LEARNING_BASECHANNEL_H
