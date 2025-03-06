package com.xueqiya.spiderman;

import android.view.Surface;

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

    public static native void playVideo(String url, Surface surface);
}
