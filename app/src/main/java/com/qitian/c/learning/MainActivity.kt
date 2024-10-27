package com.qitian.c.learning

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

        player.dataSource = File("${Environment.getExternalStorageDirectory()}${File.separator}demo.mp4").absolutePath
        player.onPreparedListener = object : QPlayer.OnPreparedListener() {
            override fun onPrepared(){
                runOnUiThread {
                    Toast.makeText(this@MainActivity,"准备成功，即将开始播放",Toast.LENGTH_SHORT).show()
                }
                player.start()
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