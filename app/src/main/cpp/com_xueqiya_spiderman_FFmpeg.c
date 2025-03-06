#include <android/log.h>
#include "com_xueqiya_spiderman_FFmpeg.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "./include/libavcodec/avcodec.h"
#include "./include/libavformat/avformat.h"
#include "./include/libavfilter/avfilter.h"

JNIEXPORT jstring JNICALL Java_com_xueqiya_spiderman_FFmpeg_run(JNIEnv *env, jclass){
    const char *conf = avcodec_configuration();
    __android_log_print(ANDROID_LOG_INFO, "JNI", "avcodec_configuration: %s", conf);
    return (*env)->NewStringUTF(env, conf);
}