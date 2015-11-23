#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->comboBox_device->clear();
    ui->comboBox_format->clear();
    ui->comboBox_size->clear();
    ui->comboBox_fps->clear();

    int n_cameras = find_devices(&camera_list);
    if( 0 < n_cameras ){
        v4l2_device_node *cur_camera = camera_list.head;
        while(cur_camera){
            ui->comboBox_device->addItem(cur_camera->dev_name,(long long)cur_camera);
            cur_camera = cur_camera->next;
        }
    }
    //get current selected camera devices
    selected_camera = (v4l2_device_node *)ui->comboBox_device->currentData().toInt();

    if(0 == enum_device(selected_camera)){
        int i;
        //add formats to combo item
        for(i=0;i<selected_camera->fmt_size;i++){
            ui->comboBox_format->addItem((const char*)(selected_camera->fmt_supported[i].fmt.description),\
                                         (long long)&selected_camera->fmt_supported[i]);
        }
        //get current selected pixel format
        selected_pixformat = (type_pixformats *)ui->comboBox_format->currentData().toInt();
        //add framesize items to combo
        for(i=0;i<selected_pixformat->frm_sizes_len;i++){
            ui->comboBox_size->addItem(QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.width) \
                                       +" x " \
                                       + QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.height),\
                                       (long long)&selected_pixformat->frm_sizes[i]);
        }
        selected_framesize = (type_framesizes*)ui->comboBox_size->currentData().toInt();
        //add framerate items to combo
        for(i=0;i<selected_framesize->frm_ivals_len;i++){
            ui->comboBox_fps->addItem(QString::number(selected_framesize->frm_ivals[i].discrete.denominator/\
                                                      selected_framesize->frm_ivals[i].discrete.numerator),\
                                      (long long)&selected_framesize->frm_ivals[i]);
        }
        selected_ivals = (struct v4l2_frmivalenum *)ui->comboBox_fps->currentData().toInt();
    }

    //connect signals & slots
    connect(ui->comboBox_device,SIGNAL(currentIndexChanged(int)),this,SLOT(deviceChanged(int)));
    connect(ui->comboBox_format,SIGNAL(currentIndexChanged(int)),this,SLOT(formatChanged(int)));
    connect(ui->comboBox_size,SIGNAL(currentIndexChanged(int)),this,SLOT(framesizeChanged(int)));
    connect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));

}

void MainWindow::openDevice()
{
    struct v4l2_format *fmt;
    memset (fmt, 0, sizeof(struct v4l2_format));

    fmt->type = selected_pixformat->fmt.type;
    fmt->fmt.pix.width = selected_framesize->frm_size.discrete.width;
    fmt->fmt.pix.height = selected_framesize->frm_size.discrete.height;
    fmt->fmt.pix.pixelformat = selected_pixformat->fmt.pixelformat;
    fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;
    if(0 == setup_device(selected_camera,fmt))
    {
        register_proc_cb(selected_camera,frameRecvCallback);
    }

}

void MainWindow::deviceChanged(int index)
{
    //get current selected camera devices
    selected_camera = (v4l2_device_node *)ui->comboBox_device->currentData().toInt();

    if(0 == enum_device(selected_camera)){
        int i;

        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_format,SIGNAL(currentIndexChanged(int)),this,SLOT(formatChanged(int)));
        ui->comboBox_format->clear();
        connect(ui->comboBox_format,SIGNAL(currentIndexChanged(int)),this,SLOT(formatChanged(int)));

        //add formats to combo item
        for(i=0;i<selected_camera->fmt_size;i++){
            ui->comboBox_format->addItem((const char*)(selected_camera->fmt_supported[i].fmt.description),\
                                         (long long)&selected_camera->fmt_supported[i]);
        }
        //get current selected pixel format
        selected_pixformat = (type_pixformats *)ui->comboBox_format->currentData().toInt();

        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_size,SIGNAL(currentIndexChanged(int)),this,SLOT(framesizeChanged(int)));
        ui->comboBox_size->clear();
        connect(ui->comboBox_size,SIGNAL(currentIndexChanged(int)),this,SLOT(framesizeChanged(int)));

        //add framesize items to combo
        for(i=0;i<selected_pixformat->frm_sizes_len;i++){
            ui->comboBox_size->addItem(QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.width) \
                                       +" x " \
                                       + QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.height),\
                                       (long long)&selected_pixformat->frm_sizes[i]);
        }
        selected_framesize = (type_framesizes*)ui->comboBox_size->currentData().toInt();

        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));
        ui->comboBox_fps->clear();
        connect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));

        //add framerate items to combo
        for(i=0;i<selected_framesize->frm_ivals_len;i++){
            ui->comboBox_fps->addItem(QString::number(selected_framesize->frm_ivals[i].discrete.denominator/\
                                                      selected_framesize->frm_ivals[i].discrete.numerator),\
                                      (long long)&selected_framesize->frm_ivals[i]);
        }
        selected_ivals = (struct v4l2_frmivalenum *)ui->comboBox_fps->currentData().toInt();
    }
}

void MainWindow::formatChanged(int index)
{
    //get current selected camera devices
    selected_camera = (v4l2_device_node *)ui->comboBox_device->currentData().toInt();

    if(0 == enum_device(selected_camera)){
        int i;
        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_size,SIGNAL(currentIndexChanged(int)),this,SLOT(framesizeChanged(int)));
        ui->comboBox_size->clear();
        connect(ui->comboBox_size,SIGNAL(currentIndexChanged(int)),this,SLOT(framesizeChanged(int)));

        //get current selected pixel format
        selected_pixformat = (type_pixformats *)ui->comboBox_format->currentData().toInt();
        //add framesize items to combo
        for(i=0;i<selected_pixformat->frm_sizes_len;i++){
            ui->comboBox_size->addItem(QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.width) \
                                       +" x " \
                                       + QString::number(selected_pixformat->frm_sizes[i].frm_size.discrete.height),\
                                       (long long)&selected_pixformat->frm_sizes[i]);
        }

        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));
        ui->comboBox_fps->clear();
        connect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));

        selected_framesize = (type_framesizes*)ui->comboBox_size->currentData().toInt();
        //add framerate items to combo
        for(i=0;i<selected_framesize->frm_ivals_len;i++){
            ui->comboBox_fps->addItem(QString::number(selected_framesize->frm_ivals[i].discrete.denominator/\
                                                      selected_framesize->frm_ivals[i].discrete.numerator),\
                                      (long long)&selected_framesize->frm_ivals[i]);
        }
        selected_ivals = (struct v4l2_frmivalenum *)ui->comboBox_fps->currentData().toInt();
    }
}

void MainWindow::framesizeChanged(int index)
{
    //get current selected camera devices
    selected_camera = (v4l2_device_node *)ui->comboBox_device->currentData().toInt();

    if(0 == enum_device(selected_camera)){
        int i;
        //disconnect first to avoid QT go crash
        disconnect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));
        ui->comboBox_fps->clear();
        connect(ui->comboBox_fps,SIGNAL(currentIndexChanged(int)),this,SLOT(fpsChanged(int)));

        //get current selected pixel format
        selected_pixformat = (type_pixformats *)ui->comboBox_format->currentData().toInt();
        selected_framesize = (type_framesizes*)ui->comboBox_size->currentData().toInt();
        //add framerate items to combo
        for(i=0;i<selected_framesize->frm_ivals_len;i++){
            ui->comboBox_fps->addItem(QString::number(selected_framesize->frm_ivals[i].discrete.denominator/\
                                                      selected_framesize->frm_ivals[i].discrete.numerator),\
                                      (long long)&selected_framesize->frm_ivals[i]);
        }
        selected_ivals = (struct v4l2_frmivalenum *)ui->comboBox_fps->currentData().toInt();
    }
}

void MainWindow::fpsChanged(int index)
{
    //get current selected camera devices
    selected_camera = (v4l2_device_node *)ui->comboBox_device->currentData().toInt();

    if(0 == enum_device(selected_camera)){
        int i;
        //get current selected pixel format
        selected_pixformat = (type_pixformats *)ui->comboBox_format->currentData().toInt();
        selected_framesize = (type_framesizes*)ui->comboBox_size->currentData().toInt();
        selected_ivals = (struct v4l2_frmivalenum *)ui->comboBox_fps->currentData().toInt();
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}
