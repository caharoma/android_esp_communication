package com.example.controller

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import java.net.Socket
import kotlin.concurrent.thread

class MainActivity2 : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main2)
        findViewById<EditText>(R.id.PortField).setText("6886")

        val button = findViewById<Button>(R.id.ConnectButton)
        button.setOnClickListener {
            var ip: String = findViewById<EditText>(R.id.IpField).text.toString()
            var port: String = findViewById<EditText>(R.id.PortField).text.toString()

            thread {
                try {
                    var socket = Socket(ip, port.toInt())

                    if (socket.isConnected) {
                        val intent = Intent(this@MainActivity2, MainActivity::class.java)
                        intent.putExtra("ip", ip)
                        intent.putExtra("port", port.toInt())
                        socket.close()

                        runOnUiThread {
                            //setContentView(R.layout.activity_main)
                            startActivity(intent)
                        }
                    }
                }
                catch (e: Exception){
                    runOnUiThread {
                        findViewById<EditText>(R.id.IpField).setText("Can't connect to server")
                    }
                }
            }
        }
    }
}