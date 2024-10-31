//
// Created by A on 2024-10-26.
//

#ifndef C__LEARNING_JNICALLBACKHELPER_H
#define C__LEARNING_JNICALLBACKHELPER_H


#include <jni.h>
#include "utils.h"

class JNICallbackHelper {
private:
    JavaVM *vm{};
    JNIEnv *env{};
    jobject job{};
    jmethodID jmd_prepared{};
    jmethodID jmd_prepare_error{};

public:
    JNICallbackHelper(JavaVM *pVm, JNIEnv *pEnv, jobject job);

    virtual ~JNICallbackHelper();

    void onPrepared(int thread_mode);

    void onPrepareError(int thread_mode, int error_code);
};


#endif //C__LEARNING_JNICALLBACKHELPER_H
