#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QTimer>
#include "ImageProc.h"
#include "readthread.h"

class PaintWidget : public QWidget
{
    Q_OBJECT

private:
    ReadThread *frameReader;
    QTimer *fpsTimer;
    char *rgb_buf;
    char *yuyv_buf;
    int fps;
    int fpsCount;
    bool updataFlag;

public:
    explicit PaintWidget(QWidget *parent = 0);
    ~PaintWidget();

    PaintWidget(ReadThread *pthread);

    void paintEvent(QPaintEvent *);

public slots:
    void updateFPS();

};

#endif // PAINTWIDGET_H
