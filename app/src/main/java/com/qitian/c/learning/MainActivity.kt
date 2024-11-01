package com.qitian.c.learning

import android.graphics.Color
import android.os.Bundle
import android.os.Environment
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.qitian.c.learning.databinding.ActivityMainBinding
import java.io.File


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    val player  = QPlayer()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()
        player.setSurfaceView(binding.surfaceView)
        player.dataSource =
            File("${Environment.getExternalStorageDirectory()}${File.separator}demo.mp4").absolutePath
//        "rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid"
        player.onPreparedListener = object : QPlayer.OnPreparedListener {
            override fun onPrepared(){
                runOnUiThread {
                    binding.tvState.setTextColor(Color.GREEN)
                    binding.tvState.text = "准备成功，即将开始播放"
                }
                player.start()
            }

            override fun onError(msg: String) {
                runOnUiThread {
                    binding.tvState.setTextColor(Color.RED)
                    binding.tvState.text = "error : "+msg
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


}