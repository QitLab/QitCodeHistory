//
// Created by A on 2024-10-25.
//

#include "QPlayer.h"

QPlayer::QPlayer(const char *data_source, JNICallbackHelper *helper) {
//  this->data_source = data_source;
//  如果被释放，会造成悬空指针

//深拷贝，C层字符串会自动加\0， 所以长度要加1
    this->data_source = new char[strlen(data_source) + 1];//为了补 \0
    strcpy(this->data_source, data_source);

    this->helper = helper;
}

QPlayer::~QPlayer() {
    if (data_source) {
        delete data_source;
    }

    if (helper) {
        delete helper;
    }
}

void *task_prepare(void *args) {// 此函数和QPlayer对象无关，无法访问QPlayer的私有变量
//    avformat_open_input(0, this->data_source)
    auto *player = static_cast<QPlayer *>(args);
    player->prepare_();

    return 0;
}

void QPlayer::prepare() {
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

void QPlayer::prepare_() {
    // 因为FFMpeg是纯C的面向过程，所以要定义大量的Context贯彻环境
    formatContext = avformat_alloc_context();
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    /**
     * 1. AVFormatContext *
     * 2. 路径
     * 3. AVInputFormat *fmt  MAC/Win摄像头麦克风等，安卓用不到
     * 4. 各种设置项：如 http超时时间，打开rtmp超时等
     */
    int r = avformat_open_input(&formatContext, data_source, 0, &dictionary);
    // 释放
    av_dict_free(&dictionary);

    if (r) {
        logd("avformat_open_input : %d",r)
        //通过JNI回调到Java
        return;
    }
    /**
     * 第二步：查找媒体中心的音视频流信息
     */
    r = avformat_find_stream_info(formatContext, 0);
    if (r < 0) {
        //通过JNI回调到Java
        logd("avformat_find_stream_info : %d",r)
        return;
    }

    /**
     * 第三步：根据留信息，流个数，用循环查找
     */
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        /**
         * 第四步：获取媒体流（视频，音频）
         */
        AVStream *stream = formatContext->streams[i];
        /**
         * 第五步：从上面的流中获取编解码参数
         */
        AVCodecParameters *parameters = stream->codecpar;
        /**
         * 第六步：根据上面的参数获取编码器
         */
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        /**
         * 第七步：获取编解码器上下文
         */
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            //通过JNI回调到Java
            logd("avcodec_alloc_context3 : %d",r)

            return;
        }
        /**
         * 第八步：他目前是一张白纸
         */
        r = avcodec_parameters_to_context(codecContext, parameters);
        if (r < 0) {
            //通过JNI回调到Java
            logd("avcodec_parameters_to_context : %d",r)

            return;
        }
        /**
         * 第九步：打开解码器
         */
        r = avcodec_open2(codecContext, codec, 0);
        if (r) {
            //通过JNI回调到Java
            logd("avcodec_open2 : %d",r)

            return;
        }
        /**
         * 第十步：从编解码器参数中获取流类型 codec_type
         */
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            audio_channel = new AudioChannel();
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            video_channel = new VideoChannel();
        }
    }
    /**
     * 第十一步：如果流中没有音频，也没有视频
     */
    if (!audio_channel && !video_channel) {
        //JNI回调到Java
        logd("audio_channel : %d",r)

        return;
    }
    /**
     * 第十二步：准备成功，通知上层
     */
    if (helper) {
        logd("准备成功，通知上层")
        helper->onPrepared(THREAD_CHILD);
    }
}
