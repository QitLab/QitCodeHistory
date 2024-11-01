//
// Created by A on 2024-10-26.
//

#ifndef C__LEARNING_VIDEOCHANNEL_H
#define C__LEARNING_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

typedef void(*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
private:
    pthread_t pid_video_decode{};
    pthread_t pid_video_play{};
    RenderCallback renderCallback{};

    int fps{};
    AudioChannel *audio_channel{};
public:
    VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base, double fps);

    virtual ~VideoChannel();

    void start();

    void setAudioChannel(AudioChannel *audio_channel);

    void video_decode();

    void video_play();

    void stop();

    void setRenderCallback(RenderCallback renderCallback);
};


#endif //C__LEARNING_VIDEOCHANNEL_H
