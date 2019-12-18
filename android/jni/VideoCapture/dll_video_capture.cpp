

#include "jni.h"
#include "dll_video_capture.h"
#include <string.h>



JNIEnv *jnienv;
char *nameVideoCaptureC = "xplat/backend/android/videocapture/VideoCaptureC";
jclass clsVideoCaptureC;
char *nameVideoCaptureInfo = "xplat/backend/android/videocapture/VideoCaptureDeviceInfo";
char *sigString = "Ljava/lang/String;";
char *sigInt = "I";
jclass clsVideoCaptureInfo;
char *nameVideoCaptureStatus = "xplat/backend/android/videocapture/VideoCaptureDeviceStatus";
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

void Java_xplat_backend_android_videocapture_VideoCaptureC_PreviewCB_jniPreviewCallback(JNIEnv *env, jobject thisobj, jobject camdev, jbyteArray cambuff){
	
	struct _s_VideoCaptureDevice **pDev;
	jbyte *buff = env->GetByteArrayElements(cambuff, NULL);
	jint buffLen = env->GetArrayLength(cambuff);
	if (userCallback != NULL){
		userCallback(camdev, EVENT_FRAME_UPDATE, (unsigned char *)buff, buffLen);
	}
	env->ReleaseByteArrayElements(cambuff, buff, JNI_ABORT);
}



char *nameStartQuery = "startQueryDevice";
char *sigStartQuery = "()";
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
char *nameNextDevice = "nextDevice";
char *sigNextDevice = "()Lxplat/backend/android/videocapture/VideoCaptureDeviceInfo;";
jmethodID idNextDevice = NULL;
char *nameInfoId = "id";
char *nameInfoName = "name";
char *nameInfoDes = "description";
jfieldID idInfoId = NULL;
jfieldID idInfoName = NULL;
jfieldID idInfoDes = NULL;
static int vcReleaseDeviceInfo(VideoCaptureDeviceInfo *info);
static int vcNextDevice(VideoCaptureDeviceInfo *info){
	if (idNextDevice == NULL){
		idNextDevice = jnienv->GetMethodID(clsVideoCaptureC, nameNextDevice, sigNextDevice);
	}
	if (idNextDevice != NULL){
		vcReleaseDeviceInfo(info);
		jobject jinfo=jnienv->CallStaticObjectMethod(clsVideoCaptureC, idNextDevice);
		if (idInfoId == NULL){
			idInfoId = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoId,sigString);
			idInfoName = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoName, sigString);
			idInfoDes = jnienv->GetFieldID(clsVideoCaptureInfo, nameInfoDes, sigString);
		}
		if (idInfoId != NULL){
			jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoId);
			info->id = (char *)jnienv->GetStringUTFChars(jstr,NULL);
		}
		if (idInfoName != NULL){
			jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoName);
			info->name = (char *)jnienv->GetStringUTFChars(jstr, NULL);
		}
		if (idInfoDes != NULL){
			jstring jstr = (jstring)jnienv->GetObjectField(jinfo, idInfoDes);
			info->description = (char *)jnienv->GetStringUTFChars(jstr, NULL);
		}
		jnienv->NewGlobalRef(jinfo);
		info->pMoniker = jinfo;
		return 0;
	}
	return -1;
}
static int vcReleaseDeviceInfo(VideoCaptureDeviceInfo *info){
	jobject jinfo = (jobject)info->pMoniker;
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
	info->pMoniker = NULL;
	return 0;
}

char *nameCloseQuery = "closeDeviceQuery";
char *sigCloseQuery = sigStartQuery;
jmethodID idCloseQuery = NULL;
static int vcCloseQueryDevice(){
	if (idCloseQuery == NULL){
		idCloseQuery = jnienv->GetMethodID(clsVideoCaptureC, nameCloseQuery, sigCloseQuery);
	}
	if (idCloseQuery != NULL){
		jnienv->CallStaticVoidMethod(clsVideoCaptureC, idCloseQuery);
		return 0;
	}
	return -1;
}

char *nameOpenDevice = "openDevice";
char *sigOpenDevice = "(Lxplat/backend/android/videocapture/VideoCaptureDeviceInfo;)Lxplat/backend/android/videocapture/VideoCaptureDevice;";
jmethodID idOpenDevice = NULL;
static int vcOpenDevice(VideoCaptureDeviceInfo *info, VideoCaptureDevice *device){
	if (idOpenDevice == NULL){
		idOpenDevice = jnienv->GetMethodID(clsVideoCaptureC,nameOpenDevice,sigOpenDevice);
	}
	if (idOpenDevice != NULL){
		jobject camdev = jnienv->CallStaticObjectMethod(clsVideoCaptureC, idOpenDevice, info->pMoniker);
		jnienv->NewGlobalRef(camdev);
		*device = camdev;
		return 0;
	}
	*device = NULL;
	return ERROR_DEVICE_OPEN;
}

char *nameStartDevice = "startDevice";
char *sigStartDevice = "(Lxplat/backend/android/videocapture/VideoCaptureDevice;)";
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
char *nameStopDevice = "stopDevice";
char *sigStopDevice = sigStartDevice;
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

char *nameCloseDevice = "closeDevice";
char *sigCloseDevice = sigStartDevice;
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
char *nameGetDeviceStatus = "getDeviceStatus";
char *sigGetDeviceStatus = "(Lxplat/backend/android/videocapture/VideoCaptureDevice;)Lxplat/backend/android/videocapture/VideoCaptureDeviceStatus;";
jmethodID idGetDeviceStatus = NULL;
char *nameFrameWidth = "frameWidth";
char *nameFrameHeight = "frameHeight";
char *nameMediaType = "mediaType";
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
		status->bitPerPixel = NULL;
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
	static VideoCaptureLibrary *video_capture_Interface;
	int video_capture_QueryInterface_Impl(VideoCaptureLibrary **result){
		if (video_capture_Interface){
			*result = video_capture_Interface;
			return 0;
		}
		video_capture_Interface = new VideoCaptureLibrary();
		video_capture_Interface->ptvbl = new VideoCaptureFunc();
		video_capture_Interface->ptvbl->StartQueryDevice = vcStartQueryDevice;
		video_capture_Interface->ptvbl->NextDevice = vcNextDevice;
		video_capture_Interface->ptvbl->CloseQueryDevice = vcCloseQueryDevice;
		video_capture_Interface->ptvbl->ReleaseDeviceInfo = vcReleaseDeviceInfo;
		video_capture_Interface->ptvbl->OpenDevice = vcOpenDevice;
		video_capture_Interface->ptvbl->CloseDevice = vcCloseDevice;
		video_capture_Interface->ptvbl->StartDevice = vcStartDevice;
		video_capture_Interface->ptvbl->StopDevice = vcStopDevice;
		video_capture_Interface->ptvbl->SetCallback = vcSetCalback;
		video_capture_Interface->ptvbl->GetCallback = vcGetCallback;
		video_capture_Interface->ptvbl->GetDeviceStatus = vcGetDeviceStatus;
		*result = video_capture_Interface;
		return 0;
	}
	int video_capture_ReleaseInterface(VideoCaptureLibrary **result){
		delete video_capture_Interface->ptvbl;
		delete video_capture_Interface;
		video_capture_Interface = NULL;
		return 0;
	}

}
