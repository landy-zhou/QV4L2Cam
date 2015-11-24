#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QTextStream>
#include <QPaintEvent>
#include <QBoxLayout>
#include "paintwidget.h"

PaintWidget::PaintWidget(QWidget *parent) : QWidget(parent)
{}

PaintWidget::PaintWidget(ReadThread *pthread)
{
    //setMinimumHeight(FRAME_HEIGHT);
    //setMinimumWidth(FRAME_WIDTH);
    frameReader = pthread;
    fpsCount = 0;
    fps = 0;
    updataFlag = false;
    fpsTimer = new QTimer;
    fpsTimer->setInterval(1000);
    connect(fpsTimer,SIGNAL(timeout()),this,SLOT(updateFPS()));
    fpsTimer->start();

    int width = frameReader->v4l2_device->cur_setup_fmt.fmt.pix.width;
    int height = frameReader->v4l2_device->cur_setup_fmt.fmt.pix.height;
    rgb_buf = new char[width*height*3];

    setWindowTitle(frameReader->v4l2_device->dev_name);
    setMinimumWidth(width);
    setMinimumHeight(height);
    //QHBoxLayout *layout = new QHBoxLayout;
    //this->setLayout(layout);
}

PaintWidget::~PaintWidget()
{
    delete fpsTimer;
    delete frameReader;
}

void PaintWidget::paintEvent(QPaintEvent *)
{
    if(frameReader->isRunning())
    {
        int width = frameReader->v4l2_device->cur_setup_fmt.fmt.pix.width;
        int height = frameReader->v4l2_device->cur_setup_fmt.fmt.pix.height;

        yuyv422_to_abgr(rgb_buf,(char *)frameReader->v4l2_device->buf[frameReader->ready_buf_index].ptr,\
                height,width);

        QImage frame((unsigned char*)rgb_buf,width,height,QImage::Format_RGB888);
        QImage frame_bgr = frame.rgbSwapped();
        QPainter painter(this);
        QPoint target(0, 0);
        painter.drawImage(target, frame_bgr);

        //display framerate
        painter.setPen(QColor(255, 0, 0, 127));
        QString fpsStr("Frame Rate: ");
        QTextStream(&fpsStr) << fps;
        painter.drawText(20,30,fpsStr);

        fpsCount++;
    }
}

void PaintWidget::updateFPS()
{
    fps = fpsCount;
    fpsCount = 0;
}
