package com.qitian.c.learning

import android.util.Log

class QPlayer {
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    lateinit var onPreparedListener: OnPreparedListener

    var dataSource: String = ""

    fun prepare() {
        prepareNative(dataSource)
    }


    fun start() {
        startNative()
    }


    fun stop() {
        stopNative()
    }


    fun release() {
        releaseNative()
    }

    fun onPrepared() {
        Log.d("QQQit", "asdasdasad")
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared()
        }
    }

    open class OnPreparedListener {
        open fun onPrepared() {

        }
    }


    external fun prepareNative(dataSource: String)
    external fun startNative()
    external fun stopNative()
    external fun releaseNative()
}