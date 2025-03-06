package com.xueqiya.spiderman

import android.os.Bundle
import android.util.Log
import android.view.SurfaceView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val versionView = findViewById<TextView>(R.id.text)
        val surfaceView = findViewById<SurfaceView>(R.id.surface)

        versionView.text = FFmpeg.getVersion()
        versionView.setOnClickListener {
            val videoUrl = "http://vjs.zencdn.net/v/oceans.mp4"
            val surface = surfaceView.holder.surface
            if (surface == null) {
                Log.e("MainActivity", "Surface is null")
                return@setOnClickListener
            }
            FFmpeg.playVideo(videoUrl, surface)
        }
    }
}