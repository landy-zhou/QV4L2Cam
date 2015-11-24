#include "readthread.h"
#include <iostream>

ReadThread::ReadThread(v4l2_device_node *device)
{
    //rgb_buf = new unsigned char[FRAME_WIDTH*FRAME_HEIGHT*3];
    v4l2_device = device;
}

ReadThread::~ReadThread()
{
    //delete rgb_buf;
    stop_capture(v4l2_device);
    close_device(v4l2_device);
}

void ReadThread::run()
{
    start_capture(v4l2_device);
    while(1){
        //loop to read 1 frame
        ready_buf_index = read_frameonce(v4l2_device,yuyv_buf,&buf_len);
        if(0 <= ready_buf_index)
            emit frameReceived();
    }
}

/*
void PrinterThread::startManager()
{
    if(isRunning()){
        emit stopReadFrame();
        terminate();
        wait();
        //emit changeStartButtonText("START");
        //emit frameReceived(); //display logo
    }else{
        start();
        emit changeStartButtonText("STOP");
        emit startReadFrame();
    }
}
*/

int ReadThread::getRGBData(char *buf)
{
    //buf = rgb_buf;
    return 0;
}
