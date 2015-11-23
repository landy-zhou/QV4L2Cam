#ifndef __IMAGEPROC_H__
#define __IMAGEPROC_H__
//#include <jni.h>
//#include <android/log.h>
//#include <android/bitmap.h>

#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>
#include <linux/usbdevice_fs.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  LOG_TAG    "WebCam"
#define  LOGI(...)  printf(__VA_ARGS__)//__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  printf(__VA_ARGS__)//__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define IMG_WIDTH 1280
#define IMG_HEIGHT 720

typedef struct{
    void *ptr;
    size_t length;
}buffer;

typedef enum{
    CLOSE=0,OPEN=1
}device_status;

typedef struct{
    struct v4l2_frmsizeenum frm_size;
    size_t frm_ivals_len;
    struct v4l2_frmivalenum frm_ivals[10];
}type_framesizes;

typedef struct{
    struct v4l2_fmtdesc fmt;
    size_t frm_sizes_len;
    type_framesizes frm_sizes[10];
}type_pixformats;

struct v4l2_device;
typedef int (* image_proc_callback)(const struct v4l2_device *device,struct v4l2_buffer *buf);

typedef struct v4l2_device{
    char dev_path[16];
    char dev_name[32];
    int fd;
    type_pixformats fmt_supported[10];
    int fmt_size;
    struct v4l2_format cur_setup_fmt;
    buffer *buf;
    int buf_size;
    image_proc_callback image_proc_cb;  //callback function for image processing
    struct v4l2_device *next;
    struct v4l2_device *prev;
    pthread_mutex_t lock;
    device_status status;
}v4l2_device_node;

typedef struct{
    v4l2_device_node *head;
    v4l2_device_node *tail;
    int length;
}v4l2_device_list;

//local functions
static int v4l2_device_list_init(v4l2_device_list *list);
static int v4l2_device_list_add(v4l2_device_list *list,v4l2_device_node *device);
static int v4l2_device_list_destory(v4l2_device_list *list);
static int errno_info(const char *str);
static int xioctl(int fd, int request, void *arg);
static void process_image(const v4l2_device_node *device,struct v4l2_buffer *buf);
static int enum_frame_sizes(int fd,type_pixformats *pixfmt);
static int enum_frame_intervals(int fd,type_pixformats *pixfmt,type_framesizes *fsize);

//export functions
/* find v4l2 devices */
int find_devices(v4l2_device_list *v4l2_cameras);
/* enumerate the device */
int enum_device(v4l2_device_node *device);
/* init & activate the device */
int setup_device(v4l2_device_node *device,struct v4l2_format *fmt);
int register_proc_cb(v4l2_device_node *device,image_proc_callback func);
int close_device(v4l2_device_node *device);
int start_capture(v4l2_device_node *device);
int stop_capture(v4l2_device_node *device);
int read_frameonce(v4l2_device_node *device,char *data,int *length);

//export util functions
int yuyv422_to_abgr(unsigned char *dst,unsigned char *src,int height,int width);

#ifdef __cplusplus
}
#endif


#endif //__IMAGEPROC_H__
