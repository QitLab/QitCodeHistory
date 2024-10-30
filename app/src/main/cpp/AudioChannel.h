//
// Created by A on 2024-10-26.
//

#ifndef C__LEARNING_AUDIOCHANNEL_H
#define C__LEARNING_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int stream_index, AVCodecContext * codecContext);
    virtual ~AudioChannel();

    void stop();
};


#endif //C__LEARNING_AUDIOCHANNEL_H
