
#ifndef _DLL_VIDEO_CAPTURE_H
#define _DLL_VIDEO_CAPTURE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct{
	char *id;
	char *name;
	char *description;
	void *internalObject;
} VideoCaptureDeviceInfo;

typedef struct {
	uint32_t mediaType;
	uint32_t bitPerPixel;
	uint32_t frameWidth;
	uint32_t frameHeight;
	uint32_t exposure;
	uint32_t zoom;
} VideoCaptureStatus;

#define MEDIATYPE_UNKNOWN 1
#define MEDIATYPE_RGB24 2
#define MEDIATYPE_YUYV 3
#define MEDIATYPE_UYVY 4;
#define MEDIATYPE_NV12 5
#define MEDIATYPE_NV21 6;
#define MEDIATYPE_YV21 7;

#define ERROR_RENDER_STREAM -13
#define ERROR_GRABBER_FILTER -12
#define ERROR_MEDIA_CONTROL -11
#define ERROR_DEVICE_OPEN -10

#define EVENT_FRAME_UPDATE 10

typedef void *VideoCaptureDevice;
typedef void(*OnDeviceEvent)(VideoCaptureDevice device,long event, unsigned char *pBuffer, long bufferLen);




typedef struct {
	int(*StartQueryDevice)();
	int(*NextDevice)(VideoCaptureDeviceInfo **info);
	int(*CloseQueryDevice)();
	int(*OpenDevice)(VideoCaptureDeviceInfo *info, VideoCaptureDevice *device);
	int(*StartDevice)(VideoCaptureDevice *device);
	int(*StopDevice)(VideoCaptureDevice *device);
	int(*CloseDevice)(VideoCaptureDevice *device);
	int(*SetCallback)(OnDeviceEvent callback);
	int(*GetCallback)(OnDeviceEvent *callback);
	int(*GetDeviceStatus)(VideoCaptureDevice *device,VideoCaptureStatus *status);
}VideoCaptureFunc;

extern int video_capture_QueryInterface(VideoCaptureFunc **result);

#ifdef __cplusplus
}
#endif

#endif