#include <jni.h>
#include <string>

#include <android/log.h>

#define TAG "QQQit"

extern "C" {
#include "libavutil/avutil.h"
};

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
extern "C" JNIEXPORT jstring JNICALL
Java_com_qitian_c_learning_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ \n";
    hello.append("当前FFMpeg的版本是: ");
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}