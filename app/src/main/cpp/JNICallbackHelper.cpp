//
// Created by A on 2024-10-26.
//

#include "JNICallbackHelper.h"

JNICallbackHelper::JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job) {
    this->vm = vm;
    this->env = env;
//    this.job = job;//jobject不能跨线程，不能跨越函数，必须全局引用
    this->job = env->NewGlobalRef(job);

    jclass clazz = env->GetObjectClass(job);

    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_prepare_error = env->GetMethodID(clazz, "onPrepareError", "(I)V");
}

JNICallbackHelper::~JNICallbackHelper() {
    vm = nullptr;
    env->DeleteGlobalRef(job);
    job = nullptr;
    env = nullptr;
}

void JNICallbackHelper::onPrepared(int thread_mode) {
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(job, jmd_prepared);
    } else if (thread_mode == THREAD_CHILD) {
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, nullptr);
        env_child->CallVoidMethod(job, jmd_prepared);
        vm->DetachCurrentThread();
    }
}

void JNICallbackHelper::onPrepareError(int thread_mode, int error_code) {
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(job, jmd_prepare_error, error_code);
    } else if (thread_mode == THREAD_CHILD) {
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, nullptr);
        env_child->CallVoidMethod(job, jmd_prepare_error, error_code);
        vm->DetachCurrentThread();
    }
}
