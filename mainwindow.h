#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ImageProc.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    v4l2_device_list camera_list;
    v4l2_device_node *selected_camera;
    type_pixformats *selected_pixformat;
    type_framesizes *selected_framesize;
    struct v4l2_frmivalenum *selected_ivals;

    void frameRecvCallback(const struct v4l2_device *device);

public slots:
    void deviceChanged(int index);
    void formatChanged(int index);
    void framesizeChanged(int index);
    void fpsChanged(int index);
    void openDevice();

};

#endif // MAINWINDOW_H
