//
// Created by A on 2024-10-26.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext): BaseChannel(stream_index, codecContext) {

}
AudioChannel::~AudioChannel() {}

void AudioChannel::stop() {

}
