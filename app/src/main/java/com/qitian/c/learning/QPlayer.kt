package com.qitian.c.learning

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

class QPlayer : SurfaceHolder.Callback {
    companion object {
        enum class FFMpegPrepareError(val errorCode: Int, val msg: String) {
            //打不开视频
            FFMPEG_OPEN_URL_FAILD(1, "打不开视频"),

            //找不到流
            FFMPEG_FIND_STREAMS_FAILED(2, "找不到流媒体"),

            //找不到解码器
            FFMPEG_FIND_DECODER_FAILED(3, "找不到解码器"),

            //无法根据解码器创建上下文
            FFMPEG_ALLOC_CODEC_CONTEXT_FAILED(4, "无法根据解码器创建上下文"),

            //根据流信息配置上下文参数失败
            FFMPEG_CODEC_CONTEXT_PARAMETERS_FAILED(6, "根据流信息配置上下文参数失败"),

            //打开解码器失败
            FFMPEG_OPEN_DECODER_FAILED(7, "打开解码器失败"),

            //没有音视频
            FFMPEG_NO_MEDIA(8, "没有音视频");

            companion object {
                fun getMsgByErrorCode(errorCode: Int): String {
                    return values().first { it.errorCode == errorCode }.msg
                }
            }
        }

        init {
            System.loadLibrary("native-lib")
        }
    }

    var onPreparedListener: OnPreparedListener? = null
    var surfaceHolder: SurfaceHolder? = null
    var dataSource: String = ""

    fun getDuration() = getDurationNative()
    fun seek(playProgress: Double) = seekNative(playProgress)

    fun prepare() = prepareNative(dataSource)
    fun start() = startNative()
    fun stop() = stopNative()
    fun release() = releaseNative()

    fun onPrepared() {
        onPreparedListener?.onPrepared()
    }

    fun onPrepareError(errorCode: Int) {
        onPreparedListener?.onError(FFMpegPrepareError.getMsgByErrorCode(errorCode))
    }

    fun onPlayProgress(progress: Double) {
        onPreparedListener?.onPlayProgress(progress)
    }

    interface OnPreparedListener {
        fun onPrepared()
        fun onError(msg: String)
        fun onPlayProgress(progress: Double)
    }


    override fun surfaceCreated(holder: SurfaceHolder) {

    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        setSurfaceNative(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }

    fun setSurfaceView(surfaceView: SurfaceView) {
        if (this.surfaceHolder != null) {
            surfaceHolder?.removeCallback(this)
        }
        surfaceHolder = surfaceView.holder
        surfaceHolder?.addCallback(this)
    }

    private external fun prepareNative(dataSource: String)
    private external fun startNative()
    private external fun stopNative()
    private external fun releaseNative()
    private external fun setSurfaceNative(surface: Surface)
    private external fun getDurationNative(): Double
    private external fun seekNative(playProgress: Double)

}