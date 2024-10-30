//
// Created by A on 2024-10-25.
//

#ifndef C__LEARNING_QPLAYER_H
#define C__LEARNING_QPLAYER_H

#include "cstring"
#include "pthread.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JNICallbackHelper.h"
#include "utils.h"

extern "C" {
#include "libavformat/avformat.h"
};

#include <android/log.h>
#define logd(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
#define TAG "QQQit"

class QPlayer {
private:
    char *data_source = 0;
    pthread_t pid_prepare;
    pthread_t pid_start;
    AVFormatContext *formatContext = 0;
    AudioChannel *audio_channel = 0;
    VideoChannel *video_channel;
    JNICallbackHelper *helper = 0;
    bool isPlaying;//是否播放
    RenderCallback renderCallback;

public:
    QPlayer(const char *data_source, JNICallbackHelper *helper);
    ~QPlayer();

    void prepare();
    void prepare_();


    void start();
    void start_();

    void setRenderCallback(RenderCallback renderCallback);
};


#endif //C__LEARNING_QPLAYER_H
