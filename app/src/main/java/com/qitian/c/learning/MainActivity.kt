package com.qitian.c.learning

import android.annotation.SuppressLint
import android.graphics.Color
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.SeekBar
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.qitian.c.learning.databinding.ActivityMainBinding
import java.io.File
import java.util.Locale


class MainActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener {

    private lateinit var binding: ActivityMainBinding

    private var isTouch = false
    var duration = 0.0

    val player = QPlayer()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        binding.seekBar.apply {
            setOnSeekBarChangeListener(this@MainActivity)
        }

        player.setSurfaceView(binding.surfaceView)
        player.dataSource =
//            File("${Environment.getExternalStorageDirectory()}${File.separator}demo.mp4").absolutePath
        "rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid"
        player.onPreparedListener = object : QPlayer.OnPreparedListener {
            @SuppressLint("SetTextI18n")
            override fun onPrepared() {
                runOnUiThread {
                    duration = player.getDuration()
                    if (duration > 0) {
                        binding.tvTime.text =
                            String.format(
                                Locale.CHINA,
                                "00:00/%d:%02d",
                                (duration / 60).toInt(),
                                (duration % 60).toInt()
                            )
                        binding.seekLayout.visibility = View.VISIBLE
                    }
                    binding.tvState.setTextColor(Color.GREEN)
                    binding.tvState.text = "准备成功，即将开始播放"
                }
                player.start()
            }

            @SuppressLint("SetTextI18n")
            override fun onError(msg: String) {
                runOnUiThread {
                    binding.tvState.setTextColor(Color.RED)
                    binding.tvState.text = "error : $msg"
                }
            }

            override fun onPlayProgress(progress: Double) {
                if (!isTouch) {//如果是拖动的
                    runOnUiThread {
                        binding.tvTime.text =
                            String.format(
                                Locale.CHINA,
                                "%02d:%02d/%02d:%02d",
                                (progress / 60).toInt(),
                                (progress % 60).toInt(),
                                (duration / 60).toInt(),
                                (duration % 60).toInt()
                            )
                        binding.seekBar.progress = (progress * 10000 / duration).toInt()
                    }
                }


            }
        }

    }

    override fun onResume() {
        super.onResume()
        player.prepare()
    }

    override fun onStop() {
        super.onStop()
        player.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }

    /**
     * A native method that is implemented by the 'learning' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
        if (fromUser) {
            Log.i("WWWWW", "--1")
            val curProgress = progress / 10000f * duration
            binding.tvTime.text =
                String.format(
                    Locale.CHINA,
                    "%02d:%02d/%02d:%02d",
                    (curProgress / 60).toInt(),
                    (curProgress % 60).toInt(),
                    (duration / 60).toInt(),
                    (duration % 60).toInt()
                )
        }
    }

    override fun onStartTrackingTouch(seekBar: SeekBar?) {
        isTouch = true
    }

    override fun onStopTrackingTouch(seekBar: SeekBar?) {
        isTouch = false
        val seekBarProgress = seekBar?.progress ?: return
        val playProgress = seekBarProgress * duration / 10000f
        player.seek(playProgress)
    }


}