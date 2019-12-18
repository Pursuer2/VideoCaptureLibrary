package xplat.backend.android.videocapture;
import android.hardware.*;
import android.hardware.Camera.Parameters;
import android.opengl.GLES20;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;

import java.io.*;
import java.nio.IntBuffer;

public class VideoCaptureC{

	public VideoCaptureC(){}
	public static int enumIndex;
	public static int enumCnt;
	public static void startQueryDevice(){
		enumIndex=0;
		enumCnt=Camera.getNumberOfCameras();
	}
	public static VideoCaptureDeviceInfo nextDevice(){
		VideoCaptureDeviceInfo info=new VideoCaptureDeviceInfo();
		if(enumIndex>=enumCnt){
			return null;
		}
		Camera.CameraInfo info2=new Camera.CameraInfo();
		Camera.getCameraInfo(enumIndex,info2);
		info.id=String.valueOf(enumIndex);
		info.name=info2.toString();
		if(info2.facing==Camera.CameraInfo.CAMERA_FACING_BACK){
			info.description="back";
		}else{
			info.description="front";
		}
		enumIndex++;
		return info;
	}
	public static void closeDeviceQuery(){
		enumCnt=0;
		enumIndex=0;
	}
	public static VideoCaptureDevice openDevice(VideoCaptureDeviceInfo info){
		VideoCaptureDevice dev=new VideoCaptureDevice();
		dev.cam=Camera.open(Integer.parseInt(info.id));
		dev.previewTex=new SurfaceTexture(0);
		try {
			dev.cam.setPreviewTexture(dev.previewTex);
			dev.cam.setPreviewCallback(new PreviewCB(dev));
		} catch (IOException e) {
			closeDevice(dev);
			return null;
		}
		return dev;
	}
	public static void startDevice(VideoCaptureDevice dev){
		dev.cam.startPreview();
	}
	public static void stopDevice(VideoCaptureDevice dev){
		dev.cam.stopPreview();
	}
	public static void closeDevice(VideoCaptureDevice dev){
		if(dev.previewTex!=null){
			dev.previewTex.release();
		}
		dev.cam.release();
	}
	public static VideoCaptureDeviceStatus getDeviceStatus(VideoCaptureDevice dev){
		VideoCaptureDeviceStatus status=new VideoCaptureDeviceStatus();
		Parameters param = dev.cam.getParameters();
		int format=param.getPreviewFormat();
		if(format==ImageFormat.NV21){
			status.mediaType=6;
		}else if(format==ImageFormat.YV12){
			status.mediaType=7;
		}else{
			status.mediaType=1;
		}
		status.frameWidth=param.getPreviewSize().width;
		status.frameHeight=param.getPreviewSize().height;
		return status;
	}
	
	public static class PreviewCB implements Camera.PreviewCallback {
		public VideoCaptureDevice dev;
		public PreviewCB(VideoCaptureDevice dev) {
			this.dev=dev;
		}
		@Override
		public void onPreviewFrame(byte[] p1, Camera p2) {
			jniPreviewCallback(dev,p1);
		}
		
	}
	public static void jniPreviewCallback(VideoCaptureDevice cam,byte[] buffer){
		try {
			FileOutputStream file=new FileOutputStream(new File("/sdcard/log"),true);
			file.write(buffer);
			file.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
}
