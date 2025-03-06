#include <android/log.h>
#include "com_xueqiya_spiderman_FFmpeg.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "./include/libavcodec/avcodec.h"
#include "./include/libavformat/avformat.h"
#include "./include/libavfilter/avfilter.h"
#include "include/libavutil/avutil.h"

JNIEXPORT jstring JNICALL Java_com_xueqiya_spiderman_FFmpeg_getVersion(JNIEnv *env, jclass){
    const unsigned version = avutil_version();
    char version_str[32];
    snprintf(version_str, sizeof(version_str), "%u", version);
    __android_log_print(ANDROID_LOG_DEBUG, "JNI", "avcodec_configuration: %s", version_str);
    return (*env)->NewStringUTF(env, version_str);
}