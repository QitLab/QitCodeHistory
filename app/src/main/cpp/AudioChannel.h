//
// Created by A on 2024-10-26.
//

#ifndef C__LEARNING_AUDIOCHANNEL_H
#define C__LEARNING_AUDIOCHANNEL_H


#include "BaseChannel.h"

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"

extern "C" {
#include "libswresample/swresample.h"//对音频进行数据转换，重采样
};

class AudioChannel : public BaseChannel {
private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
public:
    int out_sample_rate;
    int out_channels;
    int out_sample_size;
    int out_buffer_size;
    uint8_t *out_buffers = 0;
    SwrContext *swr_context = 0;
public:
    SLObjectItf engineObj = 0;//引擎
    SLEngineItf engineInterface = 0;//引擎接口
    SLObjectItf outputMixObj = 0;//混音器
    SLObjectItf bpPlayerObj = 0;//播放器
    SLPlayItf bpPlayerInterface = 0;//播放器接口
    SLAndroidSimpleBufferQueueItf bpPlayerBufferQueue = 0;//播放器队列接口
public:
    AudioChannel(int stream_index, AVCodecContext *codecContext);

    virtual ~AudioChannel();

    void start();
    void audio_decode();
    int getPCM();
    void audio_play();
    void stop();
};


#endif //C__LEARNING_AUDIOCHANNEL_H
