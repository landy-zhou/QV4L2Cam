#ifndef PRINTERTHREAD_H
#define PRINTERTHREAD_H

#include <QThread>
#include "ImageProc.h"

class ReadThread : public QThread
{
    Q_OBJECT
private:
    //unsigned char *rgb_buf;

public:
    v4l2_device_node *v4l2_device;
    int ready_buf_index;

    char *yuyv_buf;
    int buf_len;
    int width;
    int height;

    ReadThread();
    ReadThread(v4l2_device_node *device);
    ~ReadThread();

    int getRGBData(char *);

    signals:
    void frameReceived();
    //void changeStartButtonText(const QString &);
    //void stopReadFrame();
    //void startReadFrame();

private slots:
    //void startManager();

protected:
    void run();

};

#endif // PRINTERTHREAD_H
