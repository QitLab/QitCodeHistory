#include <jni.h>
#include <string>

#include <android/log.h>

//由于libgetndk文件夹下是.c，所以需要加上extern "C"，按C的方式编译
extern "C"{
#include "libgetndk/getutil.h"
}
//libgetndk文件夹下是.cpp，不需要extern "C"
#include "libcount/countutil.h."

#define TAG "QQQit"

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
extern "C" JNIEXPORT jstring JNICALL
Java_com_qitian_c_learning_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    printf("get1_action:%s", get1_action())
    printf("count_add:%f", add(9.5f, 6.3f))
    return env->NewStringUTF(hello.c_str());
}