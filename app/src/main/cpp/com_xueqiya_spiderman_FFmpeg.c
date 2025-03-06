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
    int result = 0;
    const char *path = (*env)->GetStringUTFChars(env, url, 0);
    AVFormatContext *format_context = avformat_alloc_context();
    // 打开视频文件
    result = avformat_open_input(&format_context, path, NULL, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not open video file");
        return;
    }
    result = avformat_find_stream_info(format_context, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not find video file stream info");
        return;
    }

    // 查找视频编码器
    int video_stream_index = -1;
    for (int i = 0; i < format_context->nb_streams; i++) {
        // 匹配视频流
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
        }
    }
    // 没找到视频流
    if (video_stream_index == -1) {
        LOGE("Player Error : Can not find video stream");
        return;
    }

    // 初始化视频编码器上下文
    AVCodecContext *video_codec_context = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(video_codec_context,
                                  format_context->streams[video_stream_index]->codecpar);
    // 初始化视频编码器
    AVCodec *video_codec = avcodec_find_decoder(video_codec_context->codec_id);
    if (video_codec == NULL) {
        LOGE("Player Error : Can not find video codec");
        return;
    }

    result = avcodec_open2(video_codec_context, video_codec, NULL);
    if (result < 0) {
        LOGE("Player Error : Can not find video stream");
        return;
    }

    // 获取视频的宽高
    int videoWidth = video_codec_context->width;
    int videoHeight = video_codec_context->height;
    // R4 初始化 Native Window 用于播放视频
    ANativeWindow *native_window = ANativeWindow_fromSurface(env, surface);
    if (native_window == NULL) {
        LOGE("Player Error : Can not create native window");
        return;
    }
    // 通过设置宽高限制缓冲区中的像素数量，而非屏幕的物理显示尺寸。
    // 如果缓冲区与物理屏幕的显示尺寸不相符，则实际显示可能会是拉伸，或者被压缩的图像
    result = ANativeWindow_setBuffersGeometry(native_window, videoWidth, videoHeight,
                                              WINDOW_FORMAT_RGBA_8888);
    if (result < 0) {
        LOGE("Player Error : Can not set native window buffer");
        ANativeWindow_release(native_window);
        return;
    }

    // 定义绘图缓冲区
    ANativeWindow_Buffer window_buffer;
    // 声明数据容器 有3个
    // R5 解码前数据容器 Packet 编码数据
    AVPacket *packet = av_packet_alloc();
    // R6 解码后数据容器 Frame 像素数据 不能直接播放像素数据 还要转换
    AVFrame *frame = av_frame_alloc();
    // R7 转换后数据容器 这里面的数据可以用于播放
    AVFrame *rgba_frame = av_frame_alloc();
    // 数据格式转换准备
    // 输出 Buffer
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, videoWidth, videoHeight, 1);
    // R8 申请 Buffer 内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
    av_image_fill_arrays(rgba_frame->data, rgba_frame->linesize, out_buffer, AV_PIX_FMT_RGBA,
                         videoWidth, videoHeight, 1);
    // R9 数据格式转换上下文
    struct SwsContext *data_convert_context = sws_getContext(
            videoWidth, videoHeight, video_codec_context->pix_fmt,
            videoWidth, videoHeight, AV_PIX_FMT_RGBA,
            SWS_BICUBIC, NULL, NULL, NULL);

    // 开始读取帧
    //读取帧
    while (av_read_frame(format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {

            /***
            * 很多教程写的是这个函数，这是旧版FFmpeg里面的写法，新版中已经移除了这个函数
            * avcodec_decode_video2(context, frame, &count, packet)
            * 用下面两个函数替代
            * avcodec_send_packet(context, packet)
            * avcodec_receive_frame(context, frame)
            ***/

            //视频解码
            int ret = avcodec_send_packet(video_codec_context, packet);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                continue;
            }

            ret = avcodec_receive_frame(video_codec_context, frame);
            if (ret < 0 && ret != AVERROR_EOF) {
                continue;
            }

            sws_scale(data_convert_context, (const uint8_t *const *) frame->data, frame->linesize,
                      0, video_codec_context->height,
                      rgba_frame->data, rgba_frame->linesize);

            if (ANativeWindow_lock(native_window, &window_buffer, NULL) < 0) {
                continue;
            } else {
                //将图像绘制到界面上，注意这里pFrameRGBA一行的像素和windowBuffer一行的像素长度可能不一致
                //需要转换好，否则可能花屏
                uint8_t *dst = (uint8_t *) window_buffer.bits;
                for (int h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * window_buffer.stride * 4,
                           out_buffer + h * rgba_frame->linesize[0],
                           rgba_frame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(native_window);
            }
        }
        av_packet_unref(packet);
    }
    //释放内存
    sws_freeContext(data_convert_context);
    av_free(packet);
    av_free(rgba_frame);
    avcodec_close(video_codec_context);
    avformat_close_input(&format_context);
}