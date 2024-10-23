#include <jni.h>
#include <string>

#include <android/log.h>
#define TAG "QQQit"

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
extern "C" JNIEXPORT jstring JNICALL
Java_com_qitian_c_learning_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}