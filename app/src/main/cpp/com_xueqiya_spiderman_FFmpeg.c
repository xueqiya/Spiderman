#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "com_xueqiya_spiderman_FFmpeg.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavfilter/avfilter.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/imgutils.h"
#include "include/libavutil/avutil.h"
#include "include/libavutil/frame.h"

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, "FFmpeg", FORMAT, ##__VA_ARGS__);
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", FORMAT, ##__VA_ARGS__);

JNIEXPORT jstring JNICALL
Java_com_xueqiya_spiderman_FFmpeg_getVersion(JNIEnv *env, jclass clazz) {
    const unsigned version = avutil_version();
    char version_str[32];
    snprintf(version_str, sizeof(version_str), "%u", version);
    LOGD("avcodec_configuration: %s", version_str);
    return (*env)->NewStringUTF(env, version_str);
}

JNIEXPORT void JNICALL
Java_com_xueqiya_spiderman_FFmpeg_playVideo(JNIEnv *env, jclass clazz, jstring url, jobject surface) {
    const char *videoUrl = (*env)->GetStringUTFChars(env, url, 0);

    // 初始化 FFmpeg
    avformat_network_init();

    AVFormatContext *formatContext = NULL;
    int code = avformat_open_input(&formatContext, videoUrl, NULL, NULL);
    if (code != 0) {
        LOGE("Failed to open video file, code=%d, msg=%s", code, av_err2str(code));
        return;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("Failed to find stream information")
        return;
    }

    // 查找视频流
    int videoStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        LOGE("No video stream found")
        return;
    }

    AVStream *videoStream = formatContext->streams[videoStreamIndex];
    AVCodecParameters *codecParameters = videoStream->codecpar;
    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);

    if (!codec) {
        LOGE("Codec not found")
        return;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        LOGE("Failed to copy codec parameters to context")
        return;
    }

    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        LOGE("Failed to open codec")
        return;
    }

    // 获取 Surface
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (!nativeWindow) {
        LOGE("Failed to get native window")
        return;
    }

    // 创建 SwsContext，用于将解码后的YUV数据转换为RGB
    struct SwsContext *swsContext = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
            SWS_BICUBIC, NULL, NULL, NULL
    );

    if (!swsContext) {
        LOGE("Failed to create sws context")
        return;
    }

    // 创建 AVFrame 用于视频解码
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);

    AVPacket packet;

    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            int response = avcodec_send_packet(codecContext, &packet);
            if (response < 0) {
                LOGE("Error sending packet to decoder")
                break;
            }

            while (response >= 0) {
                response = avcodec_receive_frame(codecContext, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    LOGE("Error receiving frame from decoder")
                    return;
                }

                // 转换帧为RGB格式
                sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, rgbFrame->data, rgbFrame->linesize);

                // 渲染帧
                // 将RGB数据绘制到 Surface 上
                ANativeWindow_Buffer windowBuffer;
                if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
                    LOGE("Failed to lock the window")
                    continue;
                }

                uint8_t *dst = (uint8_t *)windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;

                for (int y = 0; y < codecContext->height; y++) {
                    for (int x = 0; x < codecContext->width; x++) {
                        int r = rgbFrame->data[0][y * rgbFrame->linesize[0] + x * 3 + 0];
                        int g = rgbFrame->data[0][y * rgbFrame->linesize[0] + x * 3 + 1];
                        int b = rgbFrame->data[0][y * rgbFrame->linesize[0] + x * 3 + 2];

                        dst[y * dstStride + x * 4 + 0] = b;  // Blue
                        dst[y * dstStride + x * 4 + 1] = g;  // Green
                        dst[y * dstStride + x * 4 + 2] = r;  // Red
                        dst[y * dstStride + x * 4 + 3] = 255;  // Alpha (fully opaque)
                    }
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }

        av_packet_unref(&packet);
    }

    // 释放资源
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_freep(&buffer);
    sws_freeContext(swsContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    ANativeWindow_release(nativeWindow);
}