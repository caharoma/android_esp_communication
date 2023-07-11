package com.example.controller

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.DisplayMetrics
import android.view.MotionEvent
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import java.io.BufferedReader
import java.io.DataOutputStream
import java.io.InputStreamReader
import java.io.PrintWriter
import java.lang.Math.abs
import java.net.InetSocketAddress
import java.net.Socket
import kotlin.concurrent.thread
import kotlin.math.roundToInt

class MainActivity : AppCompatActivity() {

    private var width = 0
    private var touchplace = 0f
    private lateinit var textShow: TextView
    private lateinit var testText: TextView
    lateinit var soutput: PrintWriter



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val displaymetrics = DisplayMetrics()
        windowManager.defaultDisplay.getMetrics(displaymetrics)
        width = displaymetrics.widthPixels

        textShow = findViewById<TextView>(R.id.txt)
        textShow.text = "0"
        testText = findViewById<TextView>(R.id.testText)
        testText.text = "0"
        /*baseElement.setOnTouchListener { _, motionEvent ->
            val mX = motionEvent.x
            val point = mX-(width/2)
            textShow.text = "$mX / $width / $point"
            true
        }*/
        var ip = intent.getStringExtra("ip")
        var port  = intent.getIntExtra("port", 6886)
        thread{
            var socket = Socket(ip, port)
            soutput = PrintWriter(socket.getOutputStream(), true)
            var sinput = BufferedReader(InputStreamReader(socket.getInputStream()))

            var x = 0

            fun Tosend() {
                    var m_int = touchplace.roundToInt()
                    var m = ""
                    if (abs(m_int).toString().length == 1) {
                        m = "00" + abs(m_int).toString()
                    }
                    if (abs(m_int).toString().length == 2) {
                        m = "0" + abs(m_int).toString()
                    }
                    if (abs(m_int).toString().length == 3) {
                        m = abs(m_int).toString()
                    }

                    if (m_int > 0) {
                        runOnUiThread {
                            testText.text = "+" + m
                        }
                        soutput.println("+" + m)

                    } else {
                        runOnUiThread {
                            testText.text = "-" + m
                        }
                        soutput.println("-" + m)

                    }
                }

            val senderHandler = Handler(Looper.getMainLooper())

            senderHandler.post(object : Runnable{
                override fun run(){
                    thread{Tosend()}
                    senderHandler.postDelayed(this, 50)
                }
            })
        }
    }

    private fun touchMove(){
        textShow.text = "${touchplace.roundToInt()}"

        //soutput.println(touchplace)
        /*thread {
            soutput.println(touchplace.roundToInt())
        }*/
    }

    private fun touchUp(){
        touchplace = 0f
        textShow.text = "${touchplace.roundToInt()}"

        /*thread {
            soutput.println(touchplace.roundToInt())
        }*/
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        val x = if (event == null) {
             0f
        }
        else{
            event?.x
        }
        touchplace = ((x-(width/2))*100)/(width/2)

        when (event?.action){
            MotionEvent.ACTION_MOVE -> touchMove()
            MotionEvent.ACTION_UP -> touchUp()
            MotionEvent.ACTION_DOWN ->touchMove()
        }
        return true
    }

}