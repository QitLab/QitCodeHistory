#include <jni.h>
#include <string>

#include "QPlayer.h"
#include "JNICallbackHelper.h"
#include "android/native_window_jni.h"

extern "C" {
#include "ffmpeg/include/libavutil/avutil.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_qitian_c_learning_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ \n";
    hello.append("当前FFMpeg的版本是: ");
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}

QPlayer *player{};
JavaVM *vm{};
ANativeWindow *window{};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

jint JNI_OnLoad(JavaVM *jvm, void *args) {
    ::vm = jvm;
    return JNI_VERSION_1_6;
}

//渲染工作
void renderFrame(uint8_t *src_data, int width, int height, int src_line_src) {
    pthread_mutex_lock(&mutex);
//先释放之前的显示窗口
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
//    设置窗口的大小
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
//    自带缓冲区buff
    ANativeWindow_Buffer window_buffer;
//    如果在渲染时被锁住，无法渲染，需要释放锁，避免死锁
    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&mutex);
        return;
    }
//    开始真正渲染，rgba字节对齐
//    填充window_buff，画面就出来了
    auto *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_linesize = window_buffer.stride * 4;
//
    for (int i = 0; i < window_buffer.height; ++i) {
//        memcpy(dst_data+i*1704, src_data+i * 1704,1704);会花屏崩溃
//ANativeWindow_Buffer16字节对齐，1704无法以64位对齐
        memcpy(dst_data + i * dst_linesize, src_data + i * src_line_src, dst_linesize);


    }
    ANativeWindow_unlockAndPost(window);//解锁并刷新buffer数据
    pthread_mutex_unlock(&mutex);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_prepareNative(JNIEnv *env, jobject thiz, jstring data_source) {
    const char *data_source_ = env->GetStringUTFChars(data_source, nullptr);
    auto *helper = new JNICallbackHelper(vm, env, thiz);
    player = new QPlayer(data_source_, helper);
    player->setRenderCallback(renderFrame);
    player->prepare();
    env->ReleaseStringUTFChars(data_source, data_source_);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_startNative(JNIEnv *env, jobject thiz) {
    if (player) {
        player->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_stopNative(JNIEnv *env, jobject thiz) {
    if (player) {
        player->stop();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    pthread_mutex_unlock(&mutex);
    DELETE(player)
    DELETE(vm)
    DELETE(window)
}

extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_setSurfaceNative(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);
//先释放之前的显示窗口
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_qitian_c_learning_QPlayer_getDurationNative(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->getDuration();
    }
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_qitian_c_learning_QPlayer_seekNative(JNIEnv *env, jobject thiz, jdouble play_progress) {
    if (player) {
        player->seek(play_progress);
    }
}