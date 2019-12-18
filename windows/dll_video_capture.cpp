
#include "dll_video_capture.h"
#include "comutil.h"
#include <windows.h>
#include <dshow.h>
#include <combaseapi.h>



class CSampleGrabberCB;
EXTERN_C const IID IID_ISampleGrabberCB;
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB(
		double SampleTime,
		IMediaSample *pSample) = 0;

	virtual HRESULT STDMETHODCALLTYPE BufferCB(
		double SampleTime,
		BYTE *pBuffer,
		long BufferLen) = 0;
};


EXTERN_C const IID IID_ISampleGrabber;
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject){
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown){
			*ppvObject = this;
			return S_OK;
		}
		else{
			return S_FALSE;
		}
	}
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(
		BOOL OneShot) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetMediaType(
		const AM_MEDIA_TYPE *pType) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
		AM_MEDIA_TYPE *pType) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
		BOOL BufferThem) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
		/* [out][in] */ long *pBufferSize,
		/* [out] */ long *pBuffer) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
		/* [retval][out] */ IMediaSample **ppSample) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetCallback(
		ISampleGrabberCB *pCallback,
		long WhichMethodToCallback) = 0;
};

EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;



static OnDeviceEvent userCallback = NULL;
struct _s_VideoCaptureDevice{
	ICaptureGraphBuilder2* m_capture;
	IGraphBuilder* m_graph;
	IMediaControl* m_control;
	ISampleGrabber* m_grabber;
	CSampleGrabberCB *m_callback;
	IAMCameraControl *m_camctl;
};

class CSampleGrabberCB : public ISampleGrabberCB
{
public:
	struct _s_VideoCaptureDevice *attached;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject){
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown){
			*ppvObject = this;
			return S_OK;
		}
		else{
			return S_FALSE;
		}
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void){
		return 1;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void){
		delete this;
		return 0;
	}
	virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample){
		AM_MEDIA_TYPE *type;
		pSample->GetMediaType(&type);
		CoTaskMemFree(type);
		if (userCallback!=NULL){
			AM_MEDIA_TYPE *type;
			pSample->GetMediaType(&type);
			BYTE *pBuffer;
			pSample->GetPointer(&pBuffer);
			userCallback(attached, EVENT_FRAME_UPDATE, pBuffer, pSample->GetSize());
		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen){
		return S_OK;
	}
};


#pragma comment(lib, "strmiids.lib")

IEnumMoniker* pClassEnum = NULL;

int vcStartQueryVideoCaptureDevice(){
	HRESULT hr;
	ICreateDevEnum* pDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&pDevEnum);
	if (!pDevEnum){
		return 1;
	}
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (!pClassEnum){
		pDevEnum->Release();
		return 1;
	}
	pDevEnum->Release();
	return 0;
}

int vcReleaseVideoCaptureDeviceInfo(VideoCaptureDeviceInfo *info){
	if (info->pMoniker){
		((IMoniker *)(info->pMoniker))->Release();
		info->pMoniker = 0;
	}
	if (info->id){
		delete info->id;
		info->id = NULL;
	}
	if (info->name){
		delete info->name;
		info->name = NULL;
	}
	if (info->description){
		delete info->description;
		info->description = NULL;
	}
	return 0;
}

int vcNextVideoCaptureDevice(VideoCaptureDeviceInfo *info){
	HRESULT hr;
	IMoniker *pMoniker = NULL;
	ULONG cFetched = 0;
	pClassEnum->Next(1, &pMoniker, &cFetched);
	IPropertyBag* prop = NULL;
	if (pMoniker) {
		vcReleaseVideoCaptureDeviceInfo(info);
		info->pMoniker = pMoniker;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&prop);
		if (prop) {
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&prop);
			VARIANT var;

			VariantInit(&var);
			prop->Read(L"DevicePath", &var, 0);
			wchar_t *lpWideChar = ((wchar_t *)var.bstrVal);
			int buffReq = WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, NULL, 0, NULL, NULL);
			info->id = new char[buffReq];
			WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, info->id, buffReq, NULL, NULL);
			VariantClear(&var);

			VariantInit(&var);
			prop->Read(L"FriendlyName", &var, 0);
			lpWideChar = ((wchar_t *)var.bstrVal);
			buffReq = WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, NULL, 0, NULL, NULL);
			info->name = new char[buffReq];
			WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, info->name, buffReq, NULL, NULL);
			VariantClear(&var);

			VariantInit(&var);
			prop->Read(L"Description", &var, 0);
			lpWideChar = ((wchar_t *)var.bstrVal);
			buffReq = WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, NULL, 0, NULL, NULL);
			info->description = new char[buffReq];
			WideCharToMultiByte(CP_UTF8, NULL, lpWideChar, -1, info->description, buffReq, NULL, NULL);
			VariantClear(&var);

			prop->Release();
			prop = NULL;
			return 0;
		}
	}
	return -1;
}



int vcCloseQueryVideoCaptureDevice(){
	pClassEnum->Release();
	pClassEnum = NULL;
	return 0;
}


int vcOpenVideoCaptureDevice(VideoCaptureDeviceInfo *info, VideoCaptureDevice *device){
	HRESULT hr;
	struct _s_VideoCaptureDevice *dev = new _s_VideoCaptureDevice();
	*device = (VideoCaptureDevice)dev;
	IBaseFilter* filterSource = NULL;
	hr = ((IMoniker *)(info->pMoniker))->BindToObject(0, 0, IID_IBaseFilter, (void **)&filterSource);
	IGraphBuilder* graph = NULL;
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void**)&graph);
	if (graph) {
		ICaptureGraphBuilder2* capture = NULL;
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void**)&capture);
		if (capture) {
			IMediaControl* control = NULL;
			hr = graph->QueryInterface(IID_IMediaControl, (LPVOID*)&control);
			if (control) {
				IBaseFilter* filterGrabber = NULL;
				hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&filterGrabber);
				if (filterGrabber) {
					IBaseFilter* filterNullRenderer = NULL;
					hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&filterNullRenderer);
					if (filterNullRenderer) {
						hr = capture->SetFiltergraph(graph);
						if (SUCCEEDED(hr)) {
							hr = graph->AddFilter(filterSource, L"Video Capture");
							if (SUCCEEDED(hr)) {
								hr = graph->AddFilter(filterGrabber, L"Sample Grabber");
								if (SUCCEEDED(hr)) {
									ISampleGrabber* grabber = NULL;
									hr = filterGrabber->QueryInterface(IID_ISampleGrabber, (void**)&grabber);
									if (grabber) {
										hr = graph->AddFilter(filterNullRenderer, L"Null Renderer");
										if (SUCCEEDED(hr)) {
											hr = capture->RenderStream(NULL, &MEDIATYPE_Video, filterSource, filterGrabber, filterNullRenderer);
											if (SUCCEEDED(hr)) {
												dev->m_callback = new CSampleGrabberCB();
												dev->m_callback->attached = dev;
												hr = grabber->SetCallback(dev->m_callback, 0);
												
												if (SUCCEEDED(hr)){
													dev->m_capture = capture;
													dev->m_graph = graph;
													dev->m_control = control;
													dev->m_grabber = grabber;
													filterSource->QueryInterface(IID_IAMCameraControl,(void **) &dev->m_camctl);
													if (!SUCCEEDED(hr)){
														dev = NULL;
														return ERROR_RENDER_STREAM;
													}
												}
												else{
													dev = NULL;
													return ERROR_RENDER_STREAM;
												}
											}
											else{
												dev = NULL;
												return ERROR_RENDER_STREAM;
											}
										}
										else{
											dev = NULL;
											return ERROR_RENDER_STREAM;
										}
									}
									else{
										dev = NULL;
										return ERROR_RENDER_STREAM;
									}
								}
								else{
									dev = NULL;
									return ERROR_RENDER_STREAM;
								}
							}
							else{
								dev = NULL;
								return ERROR_RENDER_STREAM;
							}
						}
						else{
							dev = NULL;
							return ERROR_RENDER_STREAM;;
						}
					}
					else{
						dev = NULL;
						return ERROR_RENDER_STREAM;
					}
				}
				else{
					dev = NULL;
					return ERROR_GRABBER_FILTER;
				}
			}
			else{
				dev = NULL;
				return ERROR_MEDIA_CONTROL;
			}
		}
		else{
			dev = NULL;
			return ERROR_DEVICE_OPEN;
		}
	}
	else{
		dev = NULL;
		return ERROR_DEVICE_OPEN;
	}
	return S_OK;
}

int vcDisposeVideoCaptureDevice(VideoCaptureDevice *device){
	struct _s_VideoCaptureDevice *dev = (struct _s_VideoCaptureDevice *)(*device);
	if (dev->m_callback){
		dev->m_callback->Release();
		dev->m_callback = 0;
	}
	if (dev->m_capture){
		dev->m_capture->Release();
		dev->m_capture = 0;
	}
	if (dev->m_grabber){
		dev->m_grabber->Release();
		dev->m_grabber = 0;
	}
	if (dev->m_graph){
		dev->m_graph->Release();
		dev->m_graph = 0;
	}
	return 0;
}
int vcStartVideoCaptureDevice(VideoCaptureDevice *device){
	struct _s_VideoCaptureDevice *dev = (struct _s_VideoCaptureDevice *)(*device);
	return SUCCEEDED(dev->m_control->Run());
}
int vcStopVideoCaptureDevice(VideoCaptureDevice *device){
	struct _s_VideoCaptureDevice *dev = (struct _s_VideoCaptureDevice *)(*device);
	return SUCCEEDED(dev->m_control->Stop());
}
int vcCloseVideoCaptureDevice(VideoCaptureDevice *device){
	vcStopVideoCaptureDevice(device);
	return vcDisposeVideoCaptureDevice(device);
}
int vcSetCallback(OnDeviceEvent callback){
	userCallback = callback;
	return 0;
}
int vcGetCallback(OnDeviceEvent *callback){
	*callback = userCallback;
	return 0;
}

int vcGetDeviceStatus(VideoCaptureDevice *device, VideoCaptureStatus *status){
	struct _s_VideoCaptureDevice *dev = (struct _s_VideoCaptureDevice *)(*device);
	AM_MEDIA_TYPE type;
	dev->m_grabber->GetConnectedMediaType(&type);
	VIDEOINFOHEADER *header = (VIDEOINFOHEADER *)type.pbFormat;
	status->frameWidth = header->bmiHeader.biWidth;
	status->frameHeight = header->bmiHeader.biHeight;
	if (type.subtype == MEDIASUBTYPE_RGB24){
		status->mediaType = MEDIATYPE_RGB24;
	}
	else if (type.subtype == MEDIASUBTYPE_YUYV || type.subtype == MEDIASUBTYPE_YUY2){
		status->mediaType = MEDIATYPE_YUYV;
	}
	else if (type.subtype == MEDIASUBTYPE_UYVY){
		status->mediaType = MEDIATYPE_UYVY;
	}
	else if (type.subtype == MEDIASUBTYPE_NV12){
		status->mediaType = MEDIATYPE_NV12;
	}
	else{
		status->mediaType = MEDIATYPE_UNKNOWN;
	}
	long flag;
	if (dev->m_camctl){
		dev->m_camctl->Get(CameraControl_Exposure, &status->exposure, &flag);
		dev->m_camctl->Get(CameraControl_Zoom, &status->zoom, &flag);
	}
	return 0;
}

extern "C"{
	static VideoCaptureLibrary *video_capture_Interface;

	__declspec(dllexport)
		int video_capture_QueryInterface_Impl(VideoCaptureLibrary **result){
		if (video_capture_Interface){
			*result = video_capture_Interface;
			return 0;
		}
		CoInitialize(0);
		video_capture_Interface = new VideoCaptureLibrary();
		video_capture_Interface->ptvbl = new VideoCaptureFunc();
		video_capture_Interface->ptvbl->StartQueryDevice = vcStartQueryVideoCaptureDevice;
		video_capture_Interface->ptvbl->NextDevice = vcNextVideoCaptureDevice;
		video_capture_Interface->ptvbl->CloseQueryDevice = vcCloseQueryVideoCaptureDevice;
		video_capture_Interface->ptvbl->ReleaseDeviceInfo = vcReleaseVideoCaptureDeviceInfo;
		video_capture_Interface->ptvbl->OpenDevice = vcOpenVideoCaptureDevice;
		video_capture_Interface->ptvbl->CloseDevice = vcCloseVideoCaptureDevice;
		video_capture_Interface->ptvbl->StartDevice = vcStartVideoCaptureDevice;
		video_capture_Interface->ptvbl->StopDevice = vcStopVideoCaptureDevice;
		video_capture_Interface->ptvbl->SetCallback = vcSetCallback;
		video_capture_Interface->ptvbl->GetCallback = vcGetCallback;
		video_capture_Interface->ptvbl->GetDeviceStatus = vcGetDeviceStatus;
		*result = video_capture_Interface;
		return 0;
	}
	__declspec(dllexport)
		int video_capture_ReleaseInterface_Impl(VideoCaptureLibrary **result){
		delete video_capture_Interface->ptvbl;
		delete video_capture_Interface;
		video_capture_Interface = NULL;
		return 0;
	}

}
