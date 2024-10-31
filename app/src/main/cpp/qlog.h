//
// Created by A on 2024-10-31.
//

#ifndef C__LEARNING_QLOG_H
#define C__LEARNING_QLOG_H
#include <android/log.h>
#define TAG "QQQit"
#define qlogd(...) __android_log_print(ANDROID_LOG_DEBUG, TAG , __VA_ARGS__);
#endif //C__LEARNING_QLOG_H
