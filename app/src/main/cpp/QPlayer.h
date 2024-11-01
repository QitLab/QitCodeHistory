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
#include "libavutil/time.h"
}

class QPlayer {
private:
    char *data_source{};
    pthread_t pid_prepare{};
    pthread_t pid_start{};
    pthread_t pid_stop{};
    AVFormatContext *formatContext{};
    AudioChannel *audio_channel{};
    VideoChannel *video_channel{};
    JNICallbackHelper *helper{};
    bool isPlaying{};//是否播放
    RenderCallback renderCallback{};

    double duration{};

    pthread_mutex_t  seek_mutex{};

public:
    QPlayer(const char *data_source, JNICallbackHelper *helper);

    ~QPlayer();

    void prepare();

    void prepare_();


    void start();

    void start_();

    void setRenderCallback(RenderCallback renderCallback);

    double getDuration();

    void seek(double play_progress);

    void stop();

    void stop_(QPlayer * player);
};


#endif //C__LEARNING_QPLAYER_H
