package com.xueqiya.spiderman;

public class FFmpeg {

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("swscale");
        System.loadLibrary("avutil");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("swresample");
        System.loadLibrary("avdevice");
        System.loadLibrary("ffmpeg");
    }

    public static native String getVersion();
}
