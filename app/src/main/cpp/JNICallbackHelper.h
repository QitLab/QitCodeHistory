//
// Created by A on 2024-10-26.
//

#ifndef C__LEARNING_JNICALLBACKHELPER_H
#define C__LEARNING_JNICALLBACKHELPER_H


#include <jni.h>
#include "utils.h"

class JNICallbackHelper {
private:
    JavaVM *vm = 0;
    JNIEnv *env = 0;
    jobject job;
    jmethodID  jmd_prepared;

public:
    JNICallbackHelper(JavaVM *pVm, JNIEnv *pEnv, jobject job);
    virtual ~JNICallbackHelper();

    void onPrepared(int thread_mode);
};


#endif //C__LEARNING_JNICALLBACKHELPER_H
