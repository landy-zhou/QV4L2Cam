#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int n_cameras = find_devices(&camera_list);
    if( 0 < n_cameras ){
        v4l2_device_node *cur_camera = camera_list.head;
        while(cur_camera){
            ui->comboBox_device->addItem(cur_camera->dev_name);
            cur_camera = cur_camera->next;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
