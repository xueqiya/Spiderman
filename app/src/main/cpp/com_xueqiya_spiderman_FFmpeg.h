#include <jni.h>
/* Header for class com_xueqiya_spiderman_FFmpeg */

#ifndef _Included_com_xueqiya_spiderman_FFmpeg
#define _Included_com_xueqiya_spiderman_FFmpeg
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL
Java_com_xueqiya_spiderman_FFmpeg_getVersion(JNIEnv *env, jclass clazz);

JNIEXPORT void JNICALL
Java_com_xueqiya_spiderman_FFmpeg_playVideo(JNIEnv *env, jclass clazz, jstring url, jobject surface);

#ifdef __cplusplus
}
#endif
#endif