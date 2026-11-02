#pragma once

#include <android/log.h>

#define IMG_TAG "ImGhost"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, IMG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, IMG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, IMG_TAG, __VA_ARGS__)
