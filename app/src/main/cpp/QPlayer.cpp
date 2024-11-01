//
// Created by A on 2024-10-25.
//

#include "QPlayer.h"
#include "qlog.h"

QPlayer::QPlayer(const char *data_source, JNICallbackHelper *helper) {
//  this->data_source = data_source;
//  如果被释放，会造成悬空指针

//深拷贝，C层字符串会自动加\0， 所以长度要加1
    this->data_source = new char[strlen(data_source) + 1];//为了补 \0
    strcpy(this->data_source, data_source);

    this->helper = helper;
    pthread_mutex_init(&seek_mutex, nullptr);
}

QPlayer::~QPlayer() {
    delete data_source;
    delete helper;
    pthread_mutex_destroy(&seek_mutex);
}

void *task_prepare(void *args) {// 此函数和QPlayer对象无关，无法访问QPlayer的私有变量
//    avformat_open_input(0, this->data_source)
    auto *player = static_cast<QPlayer *>(args);
    player->prepare_();

    return nullptr;
}

void QPlayer::prepare() {
    pthread_create(&pid_prepare, nullptr, task_prepare, this);
}

void QPlayer::prepare_() {
    // 因为FFMpeg是纯C的面向过程，所以要定义大量的Context贯彻环境
    formatContext = avformat_alloc_context();
    AVDictionary *dictionary = nullptr;
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    /**
     * 1. AVFormatContext *
     * 2. 路径
     * 3. AVInputFormat *fmt  MAC/Win摄像头麦克风等，安卓用不到
     * 4. 各种设置项：如 http超时时间，打开rtmp超时等
     */
    int r = avformat_open_input(&formatContext, data_source, nullptr, &dictionary);
    // 释放
    av_dict_free(&dictionary);

    if (r) {
        qlogd("onPrepareError %d", r)
        if (helper) {
            helper->onPrepareError(THREAD_CHILD, 1);
        }
        avformat_close_input(&formatContext);
        return;
    }
    /**
     * 第二步：查找媒体中心的音视频流信息
     */
    r = avformat_find_stream_info(formatContext, nullptr);
    if (r < 0) {
        //通过JNI回调到Java
        if (helper) {
            helper->onPrepareError(THREAD_CHILD, 2);
        }
        avformat_close_input(&formatContext);
        return;
    }
    this->duration = formatContext->duration / AV_TIME_BASE;

    /**
     * 第三步：根据留信息，流个数，用循环查找
     */
    AVCodecContext *codecContext = nullptr;
    for (int stream_index = 0; stream_index < formatContext->nb_streams; ++stream_index) {
        /**
         * 第四步：获取媒体流（视频，音频）
         */
        AVStream *stream = formatContext->streams[stream_index];
        /**
         * 第五步：从上面的流中获取编解码参数
         */
        AVCodecParameters *parameters = stream->codecpar;
        /**
         * 第六步：根据上面的参数获取编码器
         */
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (!codec) {
            if (helper) {
                helper->onPrepareError(THREAD_CHILD, 3);
            }
        }
        /**
         * 第七步：获取编解码器上下文
         */
        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            //通过JNI回调到Java
            if (helper) {
                helper->onPrepareError(THREAD_CHILD, 4);
            }
            avcodec_free_context(&codecContext);
            avformat_close_input(&formatContext);
            return;
        }
        /**
         * 第八步：他目前是一张白纸
         */
        r = avcodec_parameters_to_context(codecContext, parameters);
        if (r < 0) {
            //通过JNI回调到Java
            if (helper) {
                helper->onPrepareError(THREAD_CHILD, 6);
            }
            avcodec_free_context(&codecContext);
            avformat_close_input(&formatContext);
            return;
        }
        /**
         * 第九步：打开解码器
         */
        r = avcodec_open2(codecContext, codec, nullptr);
        if (r) {
            if (helper) {
                helper->onPrepareError(THREAD_CHILD, 7);
            }
            avcodec_free_context(&codecContext);
            avformat_close_input(&formatContext);
            return;
        }
        //音视频同步
        AVRational time_base = stream->time_base;
        /**
         * 第十步：从编解码器参数中获取流类型 codec_type
         */
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            audio_channel = new AudioChannel(stream_index, codecContext, time_base);
            if (duration != 0) {//非直播
                audio_channel->setJNICallbackHelper(helper);
            }
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }
            auto fps = av_q2d(stream->avg_frame_rate);
            video_channel = new VideoChannel(stream_index, codecContext, time_base, fps);
            video_channel->setRenderCallback(renderCallback);
            if (duration != 0) {//非直播
                video_channel->setJNICallbackHelper(helper);
            }
        }
    }
    /**
     * 第十一步：如果流中没有音频，也没有视频
     */
    if (!audio_channel && !video_channel) {
        //JNI回调到Java
        if (helper) {
            helper->onPrepareError(THREAD_CHILD, 8);
        }
        if (codecContext) {
            avcodec_free_context(&codecContext);
        }
        avformat_close_input(&formatContext);
        return;
    }
    /**
     * 第十二步：准备成功，通知上层
     */
    if (helper) {
        qlogd("准备成功，通知上层")
        helper->onPrepared(THREAD_CHILD);
    }
}

void *task_start(void *args) {
    auto *player = static_cast<QPlayer *>(args);
    player->start_();
    return nullptr;
}

void QPlayer::start() {
    isPlaying = true;
    if (video_channel) {
        video_channel->setAudioChannel(audio_channel);
        video_channel->start();
    }
    if (audio_channel) {
        audio_channel->start();
    }
    //把音视频压缩包加入队列
    pthread_create(&pid_start, nullptr, task_start, this);
}

void QPlayer::start_() {//子线程
    while (isPlaying) {
        if (video_channel && video_channel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        if (audio_channel && audio_channel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        //VPacket 可能是音频也可能是视频（压缩包）
        AVPacket *packet = av_packet_alloc();
        int r = av_read_frame(formatContext, packet);
        if (!r) {
            // AudioChannel
            // VideoChannel
            //把AVPacket加入到队列，
            if (video_channel && video_channel->stream_index == packet->stream_index) {
                video_channel->packets.offer(packet);
            } else if (audio_channel && audio_channel->stream_index == packet->stream_index) {
                audio_channel->packets.offer(packet);
            }

        } else if (r == AVERROR_EOF) {
            //文件播放完成
            if (video_channel->packets.empty() && audio_channel->packets.empty()) {
                break;
            }
        } else {
            break;
        }

    }
    isPlaying = false;
    video_channel->stop();
    audio_channel->stop();
}

void QPlayer::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}


double QPlayer::getDuration() {
    return duration;
}

void QPlayer::seek(double play_progress) {
    if (play_progress < 0 || play_progress > duration) {
        return;
    }
    if (!audio_channel || !video_channel) {
        return;
    }
    if (!formatContext) {
        return;
    }
    pthread_mutex_lock(&seek_mutex);
    // -1代表默认，ffmpegZ自动选择音频还是视频
    int r = av_seek_frame(formatContext, -1, play_progress * AV_TIME_BASE,
                          AVSEEK_FLAG_BACKWARD);
    if (r < 0) {
        return;
    }
    if (audio_channel) {
        audio_channel->packets.setWork(0);
        audio_channel->frames.setWork(0);
        audio_channel->packets.clear();
        audio_channel->frames.clear();
        audio_channel->packets.setWork(1);
        audio_channel->frames.setWork(1);
    }
    if (video_channel) {
        video_channel->packets.setWork(0);
        video_channel->frames.setWork(0);
        video_channel->packets.clear();
        video_channel->frames.clear();
        video_channel->packets.setWork(1);
        video_channel->frames.setWork(1);
    }
    //拖动进度条需要暂停
    pthread_mutex_unlock(&seek_mutex);
}

void *task_stop(void *args) {
    auto *player = static_cast<QPlayer *>(args);
    player->stop_(player);
    return nullptr;
}

void QPlayer::stop_(QPlayer *player) {
    isPlaying = false;
    pthread_join(pid_prepare, nullptr);
    pthread_join(pid_start, nullptr);
    //等待线程结束再释放
    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
    DELETE(audio_channel)
    DELETE(video_channel)
    DELETE(player)
}

void QPlayer::stop() {
    helper = nullptr;
    if (audio_channel) {
        audio_channel->jnihelper = nullptr;
    }
    if (video_channel) {
        video_channel->jnihelper = nullptr;
    }
    pthread_create(&pid_stop, nullptr, task_stop, this);
}


