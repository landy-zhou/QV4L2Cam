#include "ImageProc.h"

/*local functions*/
static int v4l2_device_list_init(v4l2_device_list *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return 0;
}

static int v4l2_device_list_add(v4l2_device_list *list,v4l2_device_node *device)
{
    if((NULL == list->head) && (NULL==list->tail))//first node
    {
        device->next = NULL;
        device->prev = NULL;
        list->head = device;
        list->tail = device;
        list->length += 1;
    }
    else{
        device->next = NULL;
        device->prev = list->tail;
        list->tail->next = device;
        list->tail = list->tail->next;
        list->length += 1;
    }
    return list->length ;
}

static int v4l2_device_list_destory(v4l2_device_list *list)
{
    while(NULL != list->tail)
    {
        v4l2_device_node *pdevice = list->tail;
        list->tail = list->tail->prev;
        free(pdevice);
        list->length -= 1;
    }
    return 0;
}


static int errno_info(const char *str)
{
    LOGE("%s error %d, %s\n", str, errno, strerror (errno));
	return -1;
}

static int xioctl(int fd, int request, void *arg)
{
	int ret;

	do ret = ioctl (fd, request, arg);
	while (-1 == ret && EINTR == errno);

	return ret;
}

/* find v4l2 devices */
int find_devices(v4l2_device_list *v4l2_cameras)
{
	struct stat st;
    int i,fd,ret;
    char dev_path[16];
    struct v4l2_capability cap;
    v4l2_device_node *pdevice_node;
    //init the device list
    v4l2_device_list_init(v4l2_cameras);

    for(i=0 ; i<64 ; i++){
        sprintf(dev_path,"/dev/video%d",i);
        //device file status
        if (-1 == stat (dev_path, &st)) {
            LOGE("Cannot identify '%s': %d, %s\n", dev_path, errno, strerror (errno));
            continue;
        }
        //is a char device ?
        if(!S_ISCHR (st.st_mode)){
            LOGE("%s was not a device file\n",dev_path);
            continue;
        }
        //try to open the device
        fd = open (dev_path, O_RDWR, 0);
        if (0 > fd){
            LOGE("Cannot open '%s': %d, %s\n", dev_path, errno, strerror (errno));
            close(fd);
            continue;
        }
        //query the capabilities
        if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)){
            if (EINVAL == errno)
                LOGE("%s is no V4L2 device\n", dev_path);
            else
                LOGE("ioctl VIDIOC_QUERYCAP failed\n");
            close(fd);
            continue;
        }
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
            LOGE("%s is no video capture device\n", dev_path);
            close(fd);
            continue;
        }
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            LOGE("%s does not support streaming i/o\n", dev_path);
            close(fd);
            continue;
        }
        //prepare and add the device node to list
        pdevice_node = (v4l2_device_node*)malloc(sizeof(v4l2_device_node));
        strncpy(pdevice_node->dev_path,dev_path,sizeof dev_path);
        strncpy(pdevice_node->dev_name,cap.card,sizeof cap.card);
        pdevice_node->image_proc_cb = NULL;
        pdevice_node->fd = fd;
        close(fd);
        LOGI("find v4l2 device: %s,%s\n", pdevice_node->dev_path,pdevice_node->dev_name);
        ret = v4l2_device_list_add(v4l2_cameras,pdevice_node);
    }
    return ret;
}

/* enumerate the device */
int enum_device(v4l2_device_node *device)
{
    int fd,i;
    //try to open the device
    fd = open (device->dev_path, O_RDWR, 0);
    if (0 > fd){
        LOGE("Cannot open '%s': %d, %s\n", device->dev_path, errno, strerror (errno));
        close(fd);
        return -1;
    }
    device->fmt_size = 0;
    for(i=0;i<10;i++){ 			//enum pixel format
        type_pixformats *pixfmt = &device->fmt_supported[i];
        memset(pixfmt,0,sizeof(type_pixformats));
        pixfmt->fmt.index = i;
        pixfmt->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if(0 == ioctl(fd,VIDIOC_ENUM_FMT,&pixfmt->fmt)){
            device->fmt_size++;  //increase the supported format size
            int ret = enum_frame_sizes(fd,pixfmt);  //enum supported frame size
            if (ret != 0)
                printf("  Unable to enumerate frame sizes.\n");
        }else if(errno == EINVAL){
            printf("Device %s format enumerate finished\n",device->dev_path);
            break;
        }else{
            return errno_info("VIDIOC_ENUM_FMT");
        }
    }
    //close the device
    close(fd);
    return 0;
}

static int enum_frame_sizes(int fd,type_pixformats *pixfmt)
{
    int j;
    pixfmt->frm_sizes_len = 0;
    for(j=0;j<10;j++){			//enum supported frame size
        int ret;
        type_framesizes *fsize = &pixfmt->frm_sizes[j];
        memset(fsize,0,sizeof(type_framesizes));
        fsize->frm_size.index = j;
        fsize->frm_size.pixel_format = pixfmt->fmt.pixelformat;
        ret = ioctl(fd,VIDIOC_ENUM_FRAMESIZES,&fsize->frm_size);
        if(0==ret){
            if (fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                pixfmt->frm_sizes_len++;
                printf("{ discrete: width = %u, height = %u }\n",
                       fsize->frm_size.discrete.width, fsize->frm_size.discrete.height);
                ret = enum_frame_intervals(fd,pixfmt,fsize);
                if (ret != 0)
                    printf("Unable to enumerate frame sizes.\n");
            } else if (fsize->frm_size.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
                printf("{ continuous: min { width = %u, height = %u } .. "
                       "max { width = %u, height = %u } }\n",
                       fsize->frm_size.stepwise.min_width, fsize->frm_size.stepwise.min_height,
                       fsize->frm_size.stepwise.max_width, fsize->frm_size.stepwise.max_height);
                printf("Refusing to enumerate frame intervals.\n");
                break;
            } else if (fsize->frm_size.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                printf("{ stepwise: min { width = %u, height = %u } .. "
                       "max { width = %u, height = %u } / "
                       "stepsize { width = %u, height = %u } }\n",
                       fsize->frm_size.stepwise.min_width, fsize->frm_size.stepwise.min_height,
                       fsize->frm_size.stepwise.max_width, fsize->frm_size.stepwise.max_height,
                       fsize->frm_size.stepwise.step_width, fsize->frm_size.stepwise.step_height);
                printf("Refusing to enumerate frame intervals.\n");
                break;
            }
        }else if(EINVAL == errno){
            break;
        }else{
            return errno_info("VIDIOC_ENUM_FRAMESIZES");
        }
    }
    return 0;
}

static int enum_frame_intervals(int fd,type_pixformats *pixfmt,type_framesizes *fsize)
{
    int k;
    fsize->frm_ivals_len = 0;
    for(k=0;k<10;k++){
        int ret;
        struct v4l2_frmivalenum *fivals = &fsize->frm_ivals[k];
        memset(fivals,0,sizeof(struct v4l2_frmivalenum));
        fivals->index = k;
        fivals->pixel_format = pixfmt->fmt.pixelformat;
        fivals->width = fsize->frm_size.discrete.width;
        fivals->height = fsize->frm_size.discrete.height;
        ret = ioctl(fd,VIDIOC_ENUM_FRAMEINTERVALS,fivals);
        if(0==ret){
            if (fivals->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                fsize->frm_ivals_len++;
                printf("%u/%u, ",
                       fivals->discrete.numerator, fivals->discrete.denominator); //输出分数
            } else if (fivals->type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
                printf("{min { %u/%u } .. max { %u/%u } }, ",
                       fivals->stepwise.min.numerator, fivals->stepwise.min.numerator,
                       fivals->stepwise.max.denominator, fivals->stepwise.max.denominator);
                break;
            } else if (fivals->type == V4L2_FRMIVAL_TYPE_STEPWISE) {
                printf("{min { %u/%u } .. max { %u/%u } / "
                       "stepsize { %u/%u } }, ",
                       fivals->stepwise.min.numerator, fivals->stepwise.min.denominator,
                       fivals->stepwise.max.numerator, fivals->stepwise.max.denominator,
                       fivals->stepwise.step.numerator, fivals->stepwise.step.denominator);
                break;
            }
        }else if(EINVAL == errno){
            break;
        }else{
            return errno_info("VIDIOC_ENUM_FRAMEINTERVALS");
        }
    }
    return 0;
}

/* clean v4l2 devices */
int clean_devices(v4l2_device_list *v4l2_cameras)
{
    if(NULL != v4l2_cameras)
        return v4l2_device_list_destory(v4l2_cameras);
    else
        return -1;
}

/* init & activate the device */
int setup_device(v4l2_device_node *device,struct v4l2_format *fmt)
{
    unsigned int min;
    struct v4l2_requestbuffers buf_req;
    unsigned int i;

    memset (&buf_req, 0, sizeof(struct v4l2_requestbuffers));
    memcpy(&device->cur_setup_fmt,fmt,sizeof(struct v4l2_format));

    device->fd = open (device->dev_path, O_RDWR, 0);
    if (0 > device->fd){
        LOGE("Cannot open '%s': %d, %s", device->dev_path, errno, strerror (errno));
        close(device->fd);
        return -1;
    }

    if(-1 == xioctl(device->fd, VIDIOC_S_FMT, fmt))
        return errno_info("VIDIOC_S_FMT");

    min = fmt->fmt.pix.width * 2; //YUYV
    if(fmt->fmt.pix.bytesperline < min)
        fmt->fmt.pix.bytesperline = min;
    min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
    if(fmt->fmt.pix.sizeimage < min)
        fmt->fmt.pix.sizeimage = min;

    //request v4l2 kernel buffers
    buf_req.count               = 3;
    buf_req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf_req.memory              = V4L2_MEMORY_MMAP;
    if(-1 == xioctl(device->fd, VIDIOC_REQBUFS, &buf_req)){
        if (EINVAL == errno) {
            LOGE("%s does not support memory mapping\n", device->dev_name);
            return -1;
        } else{
            return errno_info("VIDIOC_REQBUFS\n");
        }

    }
    if (buf_req.count < 2){
        LOGE("Insufficient buffer memory on %s", device->dev_name);
        return -1;
    }
    //device->buf = calloc(buf_req.count,sizeof(buffer));
    //if(!device->buf){
    //    LOGE("device->buf calloc failed\n");
     //   return -1;
  //  }
    //mmap the buffers to userspace
    for(i=0; i<buf_req.count; ++i){
        struct v4l2_buffer buf;
        memset (&buf, 0, sizeof buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;
        if (-1 == xioctl(device->fd, VIDIOC_QUERYBUF, &buf))
        {
            return errno_info("VIDIOC_QUERYBUF");
        }
        device->buf[i].length = buf.length;
        device->buf[i].ptr = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, device->fd, buf.m.offset);
        if (MAP_FAILED == device->buf[i].ptr){
            return errno_info ("mmap");
        }
    }
    device->buf_size = i;

    return 0;
}

int register_proc_cb(v4l2_device_node *device,image_proc_callback func)
{
    if(NULL == func)
        return -1;
    device->image_proc_cb = func;
    return 0;
}

int close_device(v4l2_device_node *device)
{
    int i;
    for(i=0; i<device->buf_size; i++){
        if(-1 == munmap(device->buf[i].ptr,device->buf[i].length))
            errno_info ("munmap");
    }
    close(device->fd);
    free(device->buf);
    return 0;
}

int start_capture(v4l2_device_node *device)
{
    int i;
    enum v4l2_buf_type type;

    for(i = 0; i < device->buf_size; ++i){
        struct v4l2_buffer buf;
        memset (&buf, 0, sizeof buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;
        if (-1 == xioctl(device->fd, VIDIOC_QBUF, &buf))
            return errno_info("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl (device->fd, VIDIOC_STREAMON, &type))
        return errno_info ("VIDIOC_STREAMON");

    return 0;
}

int stop_capture(v4l2_device_node *device)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl (device->fd, VIDIOC_STREAMOFF, &type))
        return errno_info ("VIDIOC_STREAMOFF");

    return 0;
}

int read_frameonce(v4l2_device_node *device,char *data,int *length)
{
    fd_set fds;
    struct timeval timeout;
    struct v4l2_buffer buf;
    int ret;

    memset (&buf, 0, sizeof buf);
    FD_ZERO (&fds);
    FD_SET (device->fd, &fds);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    ret = select (device->fd + 1, &fds, NULL, NULL, &timeout);

    if(0 < ret){ 		//select return success
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (-1 == xioctl (device->fd, VIDIOC_DQBUF, &buf)){
            switch(errno){
            case EAGAIN:
                return 0;
            case EIO:
            default:
                return errno_info("VIDIOC_DQBUF");
            }
        }

        if(buf.index >= device->buf_size){
            LOGE("buf index error\n");
            return -1;
        }
        //hook the callback function
        process_image(device, &buf);
        data = device->buf[buf.index].ptr;
        *length = buf.length;

        if (-1 == xioctl (device->fd,VIDIOC_QBUF, &buf))
                return errno_info("VIDIOC_QBUF");

    }else if(0 == ret){
        LOGE("select timeout");
        return -1;
    }else{
        return errno_info ("select");
    }
    return buf.index;
}

static void process_image(const v4l2_device_node *device,struct v4l2_buffer *buf)
{
    //LOGI("%2x %2x %2x %2x",*(char*)(p),*(char*)(p+1),*(char*)(p+2),*(char*)(p+3));
    if(device->image_proc_cb)
        device->image_proc_cb(device,buf);
}


static int yuv_tbl_ready=0;
static int y1192_tbl[256];
static int v1634_tbl[256];
static int v833_tbl[256];
static int u400_tbl[256];
static int u2066_tbl[256];
int yuyv422_to_abgr(char *dst,char *src,int height,int width)
{
    int i;
    int frame_size = width*height*2;

    if(!dst){
        return -1;
    }
    int *lrgb = (int *)dst;

    if(yuv_tbl_ready==0){
        for(i=0 ; i<256 ; i++){
            y1192_tbl[i] = 1192*(i-16);
            if(y1192_tbl[i]<0){
                y1192_tbl[i]=0;
            }

            v1634_tbl[i] = 1634*(i-128);
            v833_tbl[i] = 833*(i-128);
            u400_tbl[i] = 400*(i-128);
            u2066_tbl[i] = 2066*(i-128);
        }
        yuv_tbl_ready=1;
    }

    for(i=0 ; i<frame_size ; i+=4){
        unsigned char y1, y2, u, v;
        y1 = src[i];
        u = src[i+1];
        y2 = src[i+2];
        v = src[i+3];

        int y1192_1=y1192_tbl[y1];
        int r1 = (y1192_1 + v1634_tbl[v])>>10;
        int g1 = (y1192_1 - v833_tbl[v] - u400_tbl[u])>>10;
        int b1 = (y1192_1 + u2066_tbl[u])>>10;

        int y1192_2=y1192_tbl[y2];
        int r2 = (y1192_2 + v1634_tbl[v])>>10;
        int g2 = (y1192_2 - v833_tbl[v] - u400_tbl[u])>>10;
        int b2 = (y1192_2 + u2066_tbl[u])>>10;

        r1 = r1>255 ? 255 : r1<0 ? 0 : r1;
        g1 = g1>255 ? 255 : g1<0 ? 0 : g1;
        b1 = b1>255 ? 255 : b1<0 ? 0 : b1;
        r2 = r2>255 ? 255 : r2<0 ? 0 : r2;
        g2 = g2>255 ? 255 : g2<0 ? 0 : g2;
        b2 = b2>255 ? 255 : b2<0 ? 0 : b2;

        *lrgb++ = 0xff000000 | b1<<16 | g1<<8 | r1;
        *lrgb++ = 0xff000000 | b2<<16 | g2<<8 | r2;

    }
    return 0;
}

