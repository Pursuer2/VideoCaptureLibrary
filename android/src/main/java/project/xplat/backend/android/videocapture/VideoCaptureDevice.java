package project.xplat.backend.android.videocapture;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.view.TextureView.SurfaceTextureListener;

public class VideoCaptureDevice implements SurfaceTextureListener{
	public Camera cam;
	public SurfaceTexture previewTex;
	public int glTex;
	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture arg0, int arg1, int arg2) {
	}
	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture arg0) {
		return false;
	}
	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture arg0, int arg1,
			int arg2) {
	}
	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture arg0) {
	}
}
