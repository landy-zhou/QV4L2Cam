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

typedef struct v4l2_device{
	char dev_path[16];
    char dev_name[32];
	int fd;
    buffer *buf;
    int buf_size;
	v4l2_device *next;
	v4l2_device *prev;
	pthread_mutex_t lock;
    device_status status;
}v4l2_device_node;

typedef struct{
	v4l2_device_node *head;
	v4l2_device_node *tail;
    int length;
}v4l2_device_list;


static char            dev_name[16];
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;

int camerabase = -1;

int *rgb = NULL;
int *ybuf = NULL;

int yuv_tbl_ready=0;
int y1192_tbl[256];
int v1634_tbl[256];
int v833_tbl[256];
int u400_tbl[256];
int u2066_tbl[256];

int errnoexit(const char *s);

int xioctl(int fd, int request, void *arg);

int checkCamerabase(void);
int opendevice(int videoid);
int initdevice(void);
int initmmap(void);
int startcapturing(void);

int readframeonce(void);
int readframe(void);
void processimage (const void *p);

int stopcapturing(void);
int uninitdevice(void);
int closedevice(void);

void yuyv422toABGRY(unsigned char *src);

jint Java_com_camera_simplewebcam_CameraPreview_prepareCamera( JNIEnv* env,jobject thiz, jint videoid);
jint Java_com_camera_simplewebcam_CameraPreview_prepareCameraWithBase( JNIEnv* env,jobject thiz, jint videoid, jint videobase);
void Java_com_camera_simplewebcam_CameraPreview_processCamera( JNIEnv* env,jobject thiz);
void Java_com_camera_simplewebcam_CameraPreview_stopCamera(JNIEnv* env,jobject thiz);
void Java_com_camera_simplewebcam_CameraPreview_pixeltobmp( JNIEnv* env,jobject thiz,jobject bitmap);                                                  
