

#include "jni.h"
#include "dll_video_capture.h"
#include <string.h>
#include <stdlib.h>

#ifndef dll_video_capture_malloc
#define dll_video_capture_malloc malloc
#define dll_video_capture_free free
#endif


JNIEnv *jnienv;
const char *nameVideoCaptureC = "project/xplat/backend/android/videocapture/VideoCaptureC";
jclass clsVideoCaptureC;
const char *nameVideoCaptureInfo = "project/xplat/backend/android/videocapture/VideoCaptureDeviceInfo";
const char *sigString = "Ljava/lang/String;";
const char *sigInt = "I";
jclass clsVideoCaptureInfo;
const char *nameVideoCaptureStatus = "project/xplat/backend/android/videocapture/VideoCaptureDeviceStatus";
jclass clsVideoCaptureStatus;
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved){
	vm->GetEnv((void**)&jnienv, JNI_VERSION_1_6);
	clsVideoCaptureC=jnienv->FindClass(nameVideoCaptureC);
	clsVideoCaptureInfo = jnienv->FindClass(nameVideoCaptureInfo);
	clsVideoCaptureStatus = jnienv->FindClass(nameVideoCaptureStatus);
	return JNI_VERSION_1_6;
}

static OnDeviceEvent userCallback;

void Java_project_xplat_backend_android_videocapture_VideoCaptureC_jniPreviewCallback(JNIEnv *env, jobject thisobj, jobject camdev, jbyteArray cambuff){
	
	struct _s_VideoCaptureDevice **pDev;
	jbyte *buff = env->GetByteArrayElements(cambuff, NULL);
	jint buffLen = env->GetArrayLength(cambuff);
	if (userCallback != NULL){
		userCallback(camdev, EVENT_FRAME_UPDATE, (unsigned char *)buff, buffLen);
	}
	env->ReleaseByteArrayElements(cambuff, buff, JNI_ABORT);
}



const char *nameStartQuery = "startQueryDevice";
const char *sigStartQuery = "()";
jmethodID idStartQuery = NULL;
static int vcStartQueryDevice(){
	if (idStartQuery == NULL){
		idStartQuery = jnienv->GetMethodID(clsVideoCaptureC, sigStartQuery, sigStartQuery);
	}
	if (idStartQuery != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idStartQuery);
		return 0;
	}
	else{
		return -1;
	}
}
const char *nameNextDevice = "nextDevice";
const char *sigNextDevice = "()Lproject/xplat/backend/android/videocapture/VideoCaptureDeviceInfo;";
jmethodID idNextDevice = NULL;
const char *nameInfoId = "id";
const char *nameInfoName = "name";
const char *nameInfoDes = "description";
jfieldID idInfoId = NULL;
jfieldID idInfoName = NULL;
jfieldID idInfoDes = NULL;
static int vcReleaseDeviceInfo(VideoCaptureDeviceInfo *info);

VideoCaptureDeviceInfo vcdInfo;
static int vcNextDevice(VideoCaptureDeviceInfo **info){
	if (idNextDevice == NULL){
		idNextDevice = jnienv->GetMethodID(clsVideoCaptureC, nameNextDevice, sigNextDevice);
	}
	if (idNextDevice != NULL){
		vcReleaseDeviceInfo(&vcdInfo);
		jobject jinfo=jnienv->CallStaticObjectMethod(clsVideoCaptureC, idNextDevice);
		if (idInfoId == NULL){
			idInfoId = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoId,sigString);
			idInfoName = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoName, sigString);
			idInfoDes = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoDes, sigString);
		}

		jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoId);
		vcdInfo.id = (char *)jnienv->GetStringUTFChars(jstr,NULL);
		
		jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoName);
		vcdInfo.name = (char *)jnienv->GetStringUTFChars(jstr, NULL);

		jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoDes);
		vcdInfo.description = (char *)jnienv->GetStringUTFChars(jstr, NULL);

		jnienv->NewGlobalRef(jinfo);
		vcdInfo.internalObject = jinfo;
		*info=&vcdInfo;
		return 0;
	}else{
		*info=NULL;
		return -1;
	}
}
static int vcReleaseDeviceInfo(VideoCaptureDeviceInfo *info){
	jobject jinfo = (jobject)info->internalObject;
	if (jinfo == NULL){
		return -1;
	}
	if (info->id != NULL){
		jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoId);
		jnienv->ReleaseStringUTFChars(jstr, info->id);
	}
	if (info->name != NULL){
		jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoName);
		jnienv->ReleaseStringUTFChars(jstr, info->name);
	}
	if (info->description != NULL){
		jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoDes);
		jnienv->ReleaseStringUTFChars(jstr, info->description);
	}
	jnienv->DeleteGlobalRef(jinfo);
	info->internalObject = NULL;
	return 0;
}

const char *nameCloseQuery = "closeDeviceQuery";
const char *sigCloseQuery = sigStartQuery;
jmethodID idCloseQuery = NULL;
static int vcCloseQueryDevice(){
	if(vcdInfo.internalObject!=NULL){
		vcReleaseDeviceInfo(&vcdInfo);
	}
	if (idCloseQuery == NULL){
		idCloseQuery = jnienv->GetMethodID(clsVideoCaptureC, nameCloseQuery, sigCloseQuery);
	}
	if (idCloseQuery != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idCloseQuery);
		return 0;
	}
	return -1;
}

const char *nameOpenDevice = "openDevice";
const char *sigOpenDevice = "(Lproject/xplat/backend/android/videocapture/VideoCaptureDeviceInfo;)Lproject/xplat/backend/android/videocapture/VideoCaptureDevice;";
jmethodID idOpenDevice = NULL;
static int vcOpenDevice(VideoCaptureDeviceInfo *info, VideoCaptureDevice *device){
	if (idOpenDevice == NULL){
		idOpenDevice = jnienv->GetMethodID(clsVideoCaptureC,nameOpenDevice,sigOpenDevice);
	}
	if (idOpenDevice != NULL){
		jobject camdev = jnienv->CallStaticObjectMethod(clsVideoCaptureC, idOpenDevice, info->internalObject);
		jnienv->NewGlobalRef(camdev);
		*device = camdev;
		return 0;
	}
	*device = NULL;
	return ERROR_DEVICE_OPEN;
}

const char *nameStartDevice = "startDevice";
const char *sigStartDevice = "(Lproject/xplat/backend/android/videocapture/VideoCaptureDevice;)";
jmethodID idStartDevice = NULL;
static int vcStartDevice(VideoCaptureDevice *device){
	if (idStartDevice == NULL){
		idStartDevice = jnienv->GetMethodID(clsVideoCaptureC, nameStartDevice, sigStartDevice);
	}
	if (idStartDevice != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idStartDevice, *device);
		return 0;
	}
	return -1;
}
const char *nameStopDevice = "stopDevice";
const char *sigStopDevice = sigStartDevice;
jmethodID idStopDevice = NULL;
static int vcStopDevice(VideoCaptureDevice *device){
	if (idStopDevice == NULL){
		idStopDevice = jnienv->GetMethodID(clsVideoCaptureC, nameStopDevice, sigStopDevice);
	}
	if (idStopDevice != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idStopDevice, *device);
		return 0;
	}
	return -1;
}

const char *nameCloseDevice = "closeDevice";
const char *sigCloseDevice = sigStartDevice;
jmethodID idCloseDevice = NULL;
static int vcCloseDevice(VideoCaptureDevice *device){
	if (idCloseDevice == NULL){
		idCloseDevice = jnienv->GetMethodID(clsVideoCaptureC, nameCloseDevice, sigCloseDevice);
	}
	if (idCloseDevice != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idCloseDevice, *device);
		jnienv->DeleteGlobalRef((jobject)*device);
		return 0;
	}
	return -1;
}
const char *nameGetDeviceStatus = "getDeviceStatus";
const char *sigGetDeviceStatus = "(Lproject/xplat/backend/android/videocapture/VideoCaptureDevice;)Lproject/xplat/backend/android/videocapture/VideoCaptureDeviceStatus;";
jmethodID idGetDeviceStatus = NULL;
const char *nameFrameWidth = "frameWidth";
const char *nameFrameHeight = "frameHeight";
const char *nameMediaType = "mediaType";
jfieldID idFrameWidth;
jfieldID idFrameHeight;
jfieldID idMediaType;
static int vcGetDeviceStatus(VideoCaptureDevice *device, VideoCaptureStatus *status){
	if (idGetDeviceStatus == NULL){
		idGetDeviceStatus = jnienv->GetMethodID(clsVideoCaptureStatus, nameGetDeviceStatus, sigGetDeviceStatus);
	}
	if (idGetDeviceStatus != NULL){
		jobject jstatus=jnienv->CallStaticObjectMethod(clsVideoCaptureC, idGetDeviceStatus, *device);
		if (idFrameWidth == NULL){
			idFrameWidth=jnienv->GetFieldID(clsVideoCaptureStatus, nameFrameWidth, sigInt);
			idFrameHeight = jnienv->GetFieldID(clsVideoCaptureStatus, nameFrameHeight, sigInt);
			idMediaType = jnienv->GetFieldID(clsVideoCaptureStatus, nameMediaType, sigInt);
		}
		status->frameWidth = jnienv->GetIntField(jstatus, idFrameWidth);
		status->frameHeight = jnienv->GetIntField(jstatus, idFrameHeight);
		status->mediaType = jnienv->GetIntField(jstatus, idMediaType);
		status->bitPerPixel = 0;
		status->zoom = 0xFFFF;
		status->exposure = 0xFFFF;
		return 0;
	}
	return -1;
}

static int vcSetCalback(OnDeviceEvent callback){
	userCallback = callback;
	return 0;
}
static int vcGetCallback(OnDeviceEvent *callback){
	*callback = userCallback;
	return 0;
}
extern "C"{
	static VideoCaptureFunc video_capture_Interface;
	extern int video_capture_QueryInterface(VideoCaptureFunc **result){
		video_capture_Interface.StartQueryDevice = vcStartQueryDevice;
		video_capture_Interface.NextDevice = vcNextDevice;
		video_capture_Interface.CloseQueryDevice = vcCloseQueryDevice;
		video_capture_Interface.OpenDevice = vcOpenDevice;
		video_capture_Interface.CloseDevice = vcCloseDevice;
		video_capture_Interface.StartDevice = vcStartDevice;
		video_capture_Interface.StopDevice = vcStopDevice;
		video_capture_Interface.SetCallback = vcSetCalback;
		video_capture_Interface.GetCallback = vcGetCallback;
		video_capture_Interface.GetDeviceStatus = vcGetDeviceStatus;
		*result = &video_capture_Interface;
		return 0;
	}
	int video_capture_ReleaseInterface(VideoCaptureFunc **result){
		return 0;
	}

}
