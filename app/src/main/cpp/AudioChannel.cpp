//
// Created by A on 2024-10-26.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base)
        : BaseChannel(
        stream_index, codecContext, time_base) {
    //音频三要素： 采样率、位深、声道数
    // 音频压缩数据包是AAC， 三要素一般是：44100、32b、2
    // 一般没有32位的，AAC为了运算效率32位浮点运算效率高，所以需要重采样，变为手机参数
    // 手机的参数一般三要素一般是：44100、16b、2
    //缓冲区大小定义
    //初始化缓冲区
    out_sample_rate = 44100;
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);//获取声道数，2
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); //每个sample 16b
    //三要素相乘就是所需buffer
    out_buffer_size = out_sample_rate * out_channels * out_sample_size;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffer_size));

    //音频重采样上下文
    swr_context = swr_alloc_set_opts(
            0,
            //下面是输出环节
            AV_CH_LAYOUT_STEREO,//声道布局，双声道
            AV_SAMPLE_FMT_S16,//采样大小 16b
            out_sample_rate,//采样率 41600
            //下面是输入环节
            codecContext->channel_layout,
            codecContext->sample_fmt,
            _tolower(codecContext->sample_rate), 0, 0
    );
    swr_init(swr_context);
}

AudioChannel::~AudioChannel() {}

void *task_audio_decode(void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    audio_channel->audio_decode();
    return 0;
}

void *task_audio_play(void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    audio_channel->audio_play();
    return 0;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    int pcm_size = audio_channel->getPCM();
    //添加数据到缓冲队列
    (*bq)->Enqueue(
            bq,
            audio_channel->out_buffers,//PCM数据
            pcm_size//PCM数据大小,需要计算
    );
}


void AudioChannel::start() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);

    //第一个线程，取出队列压缩包，进行解码，解码后的原始包再push到队列中
    pthread_create(&pid_audio_decode, 0, task_audio_decode, this);
    //第二个线程：从队里取出原始包，播放
    pthread_create(&pid_audio_play, 0, task_audio_play, this);

}

void AudioChannel::audio_decode() {
    AVPacket *packet = 0;
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
        if (r) {
            break;
        }
        AVFrame *avFrame = av_frame_alloc();
        r = avcodec_receive_frame(codecContext, avFrame);
        //音频也有帧的概念
        if (r == AVERROR(EAGAIN)) {
            //B帧会参考前后，等待P帧出来
            continue;
        } else if (r != 0) {
            //错误
            if (avFrame) {
                releaseAVFrame(&avFrame);
            }
            break;
        }

        frames.offer(avFrame);

        //FFMpeg内部会缓存一会packet,所以这里要把我们的释放掉
        releaseAVPacket(&packet);
    }
    releaseAVPacket(&packet);
}

void AudioChannel::audio_play() {
    SLresult r;
//    1. 创建引擎并获取引擎接口
//      1.1 创建引擎对象
    r = slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("创建引擎 slCreateEngine error")
        return;
    }
//      1.2 初始化引擎
    r = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);//SL_BOOLEAN_FALSE 延时等待到创建成功
    if (SL_RESULT_SUCCESS != r) {
        qlogd("创建引擎 Realize error")
        return;
    }
//      1.3 获取引擎接口
    r = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("创建引擎 GetInterface error")
        return;
    }
    if (engineInterface) {
        qlogd("创建引擎接口成功")
    } else {
        qlogd("创建引擎接口失败")
        return;
    }
//    2. 设置混音器
//      2.1 创建混音器
    r = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObj, 0, 0, 0);//后面三个参数是特效，不需要
    if (SL_RESULT_SUCCESS != r) {
        qlogd("创建混音器 CreateOutputMix error")
        return;
    }
//      2.2 初始化混音器
    r = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("创建混音器 Realize error")
        return;
    }
//    3. 创建播放器
//      3.1 创建buffer缓存队列
    SLDataLocator_AndroidSimpleBufferQueue local_buffer_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, //数据格式为pcm
                                   2, //双声道
                                   SL_SAMPLINGRATE_44_1,//采样率44100
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//采样格式, 每个采样点使用 16 位固定点数来进行编码
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//数据大小，每个采样点占用 16 位（2 字节）的空间
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//双声道
                                   SL_BYTEORDER_LITTLEENDIAN};//小端模式
    SLDataSource audioSrc = {&local_buffer_queue, &format_pcm};
//      3.2 配置音轨
//          设置混音器
    SLDataLocator_OutputMix local_output_mix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
    SLDataSink audioSnk = {&local_output_mix, NULL};
//          设置需要开放的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
//      3.3 创建播放器
    r = (*engineInterface)->CreateAudioPlayer(
            engineInterface,//引擎接口
            &bpPlayerObj,//播放器
            &audioSrc,//音频配置
            &audioSnk,//混音器
            1,//开放的参数个数
            ids,//buffer
            req//上面的buffer需要开放出去
    );
//      3.4 初始化播放器
    r = (*bpPlayerObj)->Realize(bpPlayerObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("初始化播放器 Realize error")
        return;
    }
//      3.5 获取播放器接口
    r = (*bpPlayerObj)->GetInterface(bpPlayerObj, SL_IID_PLAY, &bpPlayerInterface);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("获取播放器接口 GetInterface error")
        return;
    }
    qlogd("创建播放器成功")
//    4. 设置播放回调
//      4.1 获取播放队列接口
    r = (*bpPlayerObj)->GetInterface(bpPlayerObj, SL_IID_BUFFERQUEUE, &bpPlayerBufferQueue);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("获取播放队列 GetInterface error")
        return;
    }
//      设置回调
    r = (*bpPlayerBufferQueue)->RegisterCallback(bpPlayerBufferQueue, bqPlayerCallback,
                                                 this);//this是给callback的参数
    if (SL_RESULT_SUCCESS != r) {
        qlogd("设置回调 RegisterCallback error")
        return;
    }
//    5. 设置播放器状态为播放
    r = (*bpPlayerInterface)->SetPlayState(bpPlayerInterface, SL_PLAYSTATE_PLAYING);
    if (SL_RESULT_SUCCESS != r) {
        qlogd("设置播放器状态为播放 SetPlayState error")
        return;
    }
//    6. 手动激活回调
    bqPlayerCallback(bpPlayerBufferQueue, this);
//    7. 释放
}


int AudioChannel::getPCM() {
    int pcm_data_size = 0;
    //获取PCM数据
    //PCM数据在frames队列中，是32位的，待重采样
    AVFrame *frame = 0;
    while (isPlaying) {
        int r = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        if (!r) {
            continue;
        }
        //开始重采样
        int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_context, frame->sample_rate) + frame->nb_samples,
                out_sample_rate,
                frame->sample_rate,
                AV_ROUND_UP
        );
        //返回每个通道转换后输出的样本数
        int samples_per_channel = swr_convert(
                //下面是输出
                swr_context,
                &out_buffers,//输出的buffer
                dst_nb_samples,//输出的单通道样本数
                //下面是输入
                (const uint8_t **) frame->data,//未重采样的输入数据
                frame->nb_samples//输入的样本数
        );
        pcm_data_size = samples_per_channel * out_sample_size * out_channels;
        //音视频同步
        //时间基Timebase的时间戳
        audio_time = frame->best_effort_timestamp * av_q2d(time_base);
        break;
    }
    return pcm_data_size;
}

void AudioChannel::stop() {

}
