/*
Copyright (c) 2020, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.camera;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;

import android.util.Range;
import android.util.Size;
import android.util.SizeF;
import android.view.Surface;
import android.view.TextureView;
import android.widget.Toast;

import java.nio.ShortBuffer;
import java.util.ArrayList;
import java.util.List;
import org.codeaurora.snapcam.R;

public class TofCameraActivity extends Activity implements TextureView.SurfaceTextureListener {
    private static final String TAG = "TofCamera";

    public static final String TOF_ACTION = "android.media.action.TOF_CAMERA";
    private static final short RANG_MIN = 0;
    private static final short RANG_MAX =400;
    private static final short RANG_DIFF = RANG_MAX - RANG_MIN;
    private static final float CONFIDENCE = 0.1f;
    private static final Size PREVIEW_SIZE = new Size(640,480);

    private TextureView mTextureView;
    private CameraManager mCameraManager;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mSession;
    private String mDepthId;
    private Size mPreviewSize;
    private ImageReader mPreviewReader;
    private CaptureRequest.Builder mPreviewRequest;
    private HandlerThread mProcessThread;
    private Handler mProcessHandler;
    private Matrix mDrawMatrix;
    private volatile boolean mPreviewReady = false;
    private Handler mRenderHandler = new renderHandler();


    private class renderHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.obj != null && msg.obj instanceof Bitmap){
                Bitmap bitmap = (Bitmap)msg.obj;
                Canvas canvas = mTextureView.lockCanvas();
                canvas.drawBitmap(bitmap,mDrawMatrix,null);
                mTextureView.unlockCanvasAndPost(canvas);
                bitmap.recycle();
            }
        }
    }

    private ImageReader.OnImageAvailableListener mTofImageListener =
            new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(ImageReader imageReader) {
                    Image image = imageReader.acquireNextImage();
                    if(mPreviewReady) {
                        ShortBuffer buffer = image.getPlanes()[0].getBuffer().asShortBuffer();
                        Bitmap bitmap = depthToRGBA(buffer,image.getWidth()/2,image.getHeight()/2);
                        if (mDrawMatrix == null) {
                            Log.d(TAG, "image width=" + image.getWidth() + " height=" + image.getHeight());
                            Log.d(TAG, "image stride=" + image.getPlanes()[0].getRowStride());
                            mDrawMatrix = getBitmapTransform(
                                    mTextureView, image.getWidth() / 2, image.getHeight() / 2);
                            Log.d(TAG, "buffer size=" + image.getPlanes()[0].getBuffer().capacity());
                        }
                        Message message = mRenderHandler.obtainMessage(1,bitmap);
                        mRenderHandler.sendMessage(message);
                    }
                    image.close();
                }
            };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tof);
        mCameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        mTextureView = (TextureView) findViewById(R.id.tof_preview);
        mTextureView.setSurfaceTextureListener(this);
        mProcessThread = new HandlerThread("Process Thread");
        mProcessThread.start();
        mProcessHandler = new Handler(mProcessThread.getLooper());
    }

    @Override
    protected void onResume() {
        super.onResume();
        setupDepthCamera();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mSession != null) {
            mSession.close();
            mSession = null;
        }
        if (mCameraDevice != null){
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mProcessHandler = null;
        mProcessThread.quitSafely();
    }

    private void setupDepthCamera()  {
        try{
            if (mCameraManager != null) {
                for (String cameraId : mCameraManager.getCameraIdList()) {
                    Log.d(TAG,"found camera id="+cameraId);
                    CameraCharacteristics characteristics =
                            mCameraManager.getCameraCharacteristics(cameraId);
                    int[] caps = characteristics.get(
                            CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
                    for (int i : caps) {
                        if (i == CameraMetadata.REQUEST_AVAILABLE_CAPABILITIES_DEPTH_OUTPUT) {
                            mDepthId = cameraId;
                            Log.d(TAG, "found depth camera id=" + mDepthId);
                        }
                    }
                }
            }
            if (mDepthId != null) {
                SizeF sensor_size = mCameraManager.getCameraCharacteristics(mDepthId).get(
                        CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);
                Log.d(TAG,"sensor_size="+sensor_size.toString());
                Rect active_array_size = mCameraManager.getCameraCharacteristics(mDepthId).get(
                        CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
                Log.d(TAG,"active_array_size="+active_array_size.toString());
                Size pixel_array_size = mCameraManager.getCameraCharacteristics(mDepthId).get(
                        CameraCharacteristics.SENSOR_INFO_PIXEL_ARRAY_SIZE);
                Log.d(TAG,"pixel_array_size="+pixel_array_size.toString());
                mPreviewSize = pixel_array_size;
                mPreviewReader = ImageReader.newInstance(mPreviewSize.getWidth(),
                        mPreviewSize.getHeight(),
                        ImageFormat.DEPTH16, 8);
                mPreviewReader.setOnImageAvailableListener(mTofImageListener, mProcessHandler);
                openCamera(mDepthId);
            } else {
                Toast.makeText(getApplicationContext(), "Can't find depth camera",
                        Toast.LENGTH_LONG).show();
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void openCamera(String cameraId) throws CameraAccessException {
        CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
            @Override
            public void onOpened(CameraDevice cameraDevice) {
                Log.d(TAG, "onOpened " + mDepthId);
                mCameraDevice = cameraDevice;
                final CameraCaptureSession.CaptureCallback captureCallback = new CameraCaptureSession.CaptureCallback() {
                    @Override
                    public void onCaptureStarted(CameraCaptureSession session,
                                                 CaptureRequest request, long timestamp,
                                                 long frameNumber) {
                        super.onCaptureStarted(session, request, timestamp, frameNumber);
                    }

                    @Override
                    public void onCaptureProgressed(CameraCaptureSession session,
                                                    CaptureRequest request,
                                                    CaptureResult partialResult) {
                        super.onCaptureProgressed(session, request, partialResult);
                    }

                    @Override
                    public void onCaptureFailed(CameraCaptureSession session,
                                                CaptureRequest request,
                                                CaptureFailure failure) {
                        super.onCaptureFailed(session, request, failure);
                        Log.d(TAG, "onCaptureFailed");
                    }
                };
                CameraCaptureSession.StateCallback callback = new CameraCaptureSession.StateCallback() {

                    @Override
                    public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                        Log.d(TAG, "onConfigured");
                        mSession = cameraCaptureSession;
                        try {
                            mSession.setRepeatingRequest(
                                    mPreviewRequest.build(), captureCallback, null);
                        } catch (CameraAccessException e) {
                            e.printStackTrace();
                        }
                    }

                    @Override
                    public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                        Toast.makeText(getApplicationContext(),
                                "onConfigureFailed " + mDepthId, Toast.LENGTH_LONG);
                    }
                };
                try {
                    mPreviewRequest = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                    Range<Integer> fpsRange = new Range<>(15, 30);
                    mPreviewRequest.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, fpsRange);
                    mPreviewRequest.addTarget(mPreviewReader.getSurface());
                    List<Surface> outputSurfaces = new ArrayList<>();
                    outputSurfaces.add(mPreviewReader.getSurface());
                    mCameraDevice.createCaptureSession(outputSurfaces,callback,null);
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onDisconnected(CameraDevice cameraDevice) {
                mCameraDevice = null;
                mSession = null;
            }

            @Override
            public void onError(CameraDevice cameraDevice, int i) {
                Toast.makeText(getApplicationContext(),
                        "onError " + mDepthId, Toast.LENGTH_LONG).show();
            }
        };
        mCameraManager.openCamera(cameraId, stateCallback, null);
    }

    private Bitmap depthToRGBA(ShortBuffer buffer,int width,int height) {
        int[] argb = new int[width * height];

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = y * width + x;
                short depthSample = buffer.get(index);
                short depthRange = (short) (depthSample & 0x1FFF);
                short depthConfidence = (short) ((depthSample >> 13) & 0x7);
                float depthPercentage = depthConfidence == 0 ? 1.f : (depthConfidence - 1) / 7.f;
                int value = 0;
                if (depthPercentage > CONFIDENCE) {
                    if (depthRange > RANG_MAX) {
                        depthRange = RANG_MAX;
                    } else if (depthRange < RANG_MIN){
                        depthRange = RANG_MIN;
                    }
                    value = depthRange - RANG_MIN;
                    value = value / RANG_DIFF * 255;
                    value = 255 - value;
                }
                argb[index] = Color.argb(255,value,0,0);
            }
        }

        Bitmap bitmap = Bitmap.createBitmap(argb,width,height, Bitmap.Config.ARGB_8888);

        return bitmap;
    }

    private Matrix getBitmapTransform(TextureView view,int width,int height) {
        Matrix matrix = new Matrix();

        int centerX = view.getWidth() / 2;
        int centerY = view.getHeight() / 2;

        RectF bufferRect = new RectF(0, 0, width, height);
        RectF viewRect = new RectF(0, 0, view.getWidth(), view.getHeight());
        matrix.setRectToRect(bufferRect, viewRect, Matrix.ScaleToFit.CENTER);
        matrix.postRotate(90, centerX, centerY);

        return matrix;
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int i, int i1) {
        mPreviewReady = true;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int i, int i1) {
        mPreviewReady = true;
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        mPreviewReady = false;
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

    }
}
