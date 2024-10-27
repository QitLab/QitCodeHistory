#include <jni.h>
#include <string>

#include <android/log.h>
#include "QPlayer.h"
#include "JNICallbackHelper.h"

#define TAG "QQQit"

extern "C" {
#include "ffmpeg/include/libavutil/avutil.h"
};

#define logd(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
extern "C" JNIEXPORT jstring JNICALL
Java_com_qitian_c_learning_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ \n";
    hello.append("当前FFMpeg的版本是: ");
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}

QPlayer *player =0;
JavaVM *vm = 0;

jint JNI_OnLoad(JavaVM *vm, void *args) {
        ::vm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_prepareNative(JNIEnv *env, jobject thiz, jstring data_source) {
    const char *data_source_ = env->GetStringUTFChars(data_source, 0);
    JNICallbackHelper *helper = new JNICallbackHelper(vm, env, thiz);
    player = new QPlayer(data_source_, helper);
    player->prepare();
    env->ReleaseStringUTFChars(data_source, data_source_);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_startNative(JNIEnv *env, jobject thiz) {
    if(player){
//        player.start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_stopNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_releaseNative(JNIEnv *env, jobject thiz) {

}