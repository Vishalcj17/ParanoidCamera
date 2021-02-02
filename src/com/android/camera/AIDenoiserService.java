/*
Copyright (c) 2020 The Linux Foundation. All rights reserved.

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

import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.camera2.TotalCaptureResult;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicResize;
import android.renderscript.Type;
import android.util.Log;
import android.os.SystemClock;

import java.io.ByteArrayOutputStream;
import java.io.File;

import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.IBinder;
import com.android.camera.aide.AideUtil;
import com.android.camera.aide.SwmfnrUtil;

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.location.Location;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageWriter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;
import android.renderscript.ScriptIntrinsicYuvToRGB;

import com.android.camera.CameraActivity;
import com.android.camera.CaptureModule;
import com.android.camera.Exif;
import com.android.camera.MediaSaveService;
import com.android.camera.PhotoModule;
import com.android.camera.SettingsManager;
import com.android.camera.deepportrait.DPImage;
import com.android.camera.exif.ExifInterface;
import com.android.camera.exif.Rational;
import com.android.camera.ui.RotateTextToast;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import android.util.Size;
import java.util.TimeZone;
import java.util.concurrent.Semaphore;

import com.android.camera.util.CameraUtil;
import com.android.camera.util.PersistUtil;
import com.android.camera.util.VendorTagUtil;
import com.android.camera.imageprocessor.ZSLQueue;
import com.android.camera.aide.AideUtil.*;
import com.android.camera.aide.SwmfnrUtil.*;

import com.android.camera.imageprocessor.PostProcessor;

import static com.android.camera.imageprocessor.PostProcessor.addExifTags;

public class AIDenoiserService extends Service {

    private static class IntegerLock {
        private int value;

        public IntegerLock(int value) {
            this.value = value;
        }

        public synchronized int get(){
            return value;
        }

        public synchronized int incrementAndGet(int incrBy) {
            value += incrBy;
            notifyAll();
            return value;
        }

        public synchronized void set(int value) {
            this.value = value;
            notifyAll();
        }

        public synchronized void waitUntilIs(int targetValue) {
            while (value != targetValue) {
                try {
                    wait();
                } catch(InterruptedException e) {
                    // ignore
                }
            }
        }
    }

    private static final int MAX_REQUIRED_IMAGE_NUM = 11;
    private AideUtil mAideUtil;
    private SwmfnrUtil mSwmfnrUtil;
    private static final String TAG = "snapcam_aideservice";
    private ZSLQueue mMfnrQueue;
    private ImageHandlerTask mImageHandlerTask;
    private Thread mThread;
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    ZSLQueue.ImageItem mItem;
    private final IBinder mBinder = new LocalBinder();
    private ByteBuffer mMfnrOut;
    private ByteBuffer mAideOut;
    int mStrideY;
    int mStrideC;
    int mWidth;
    int mHeight;
    int[] mOutRoi = new int[4];
    CameraActivity mActivity;
    private float mGain;
    private IntegerLock imagesNum = new IntegerLock(0);
    private boolean isDoingMfnr = false;

    class LocalBinder extends Binder {
        public AIDenoiserService getService() {
            return AIDenoiserService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG,"onBind");
        mAideUtil = new AideUtil();
        mSwmfnrUtil = new SwmfnrUtil();
        mImageHandlerTask = new ImageHandlerTask();
        mHandlerThread = new HandlerThread("AideThread");
        mHandlerThread.start();
        mHandler = new ProcessorHandler(mHandlerThread.getLooper());
        if(SwmfnrUtil.isSwmfnrSupported()) {
            createMfnr();
        }
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        return 0;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG,"service onDestory");
        mImageHandlerTask = null;
        if (mHandlerThread != null) {
            mHandlerThread.quitSafely();
            try {
                mHandlerThread.join();
            } catch (InterruptedException e) {
            }
            mHandlerThread = null;
            mHandler = null;
        }
        if(mMfnrQueue != null) {
            mMfnrQueue.onClose();
            mMfnrQueue = null;
        }
        if(SwmfnrUtil.isSwmfnrSupported()) destoryMfnr();
    }

    @Override
    public void onCreate() {
    }

    public void createMfnr() {
        mMfnrQueue = new ZSLQueue(null);
        long sessionId = mSwmfnrUtil.nativeMfnrCreate();
        Log.i(TAG,"createMfnr, sessionId:" + sessionId);
    }

    public void configureMfnr(int width, int height, MfnrTunableParams mfnrParams){
        Log.i(TAG,"configureMfnr, width" + width + ",height:" + height + ",mfnrParams:" + mfnrParams);
        int allocateResult = mSwmfnrUtil.nativeMfnrAllocate(width, height);
        Log.i(TAG,"configureMfnr, allocateResult" + allocateResult);
        Log.i(TAG,"configureMfnr, param, enableGyroRefinement" + mfnrParams.enableGyroRefinement + ",imageRegDesc:"+ mfnrParams.imageRegDesc + ",imageRegMode:" + mfnrParams.imageRegMode + ",deghostingStrength:" + mfnrParams.deghostingStrength + ",localMotionSmoothingStrength:" + mfnrParams.localMotionSmoothingStrength
             + "dtfSpatialYStrength:" + mfnrParams.dtfSpatialYStrength + ",dtfSpatialChromaStrength:" + mfnrParams.dtfSpatialChromaStrength + ",sharpnessStrength:" + mfnrParams.sharpnessStrength + ",spatioTemporalDenoiseBalanceStrength" +mfnrParams.spatioTemporalDenoiseBalanceStrength + ",sharpnessScoreThreshold:" + mfnrParams.sharpnessScoreThreshold);
        int configureResult = mSwmfnrUtil.nativeMfnrConfigure(mfnrParams.enableGyroRefinement, mfnrParams.imageRegDesc, mfnrParams.imageRegMode, mfnrParams.deghostingStrength, mfnrParams.localMotionSmoothingStrength,
            mfnrParams.dtfSpatialYStrength, mfnrParams.dtfSpatialChromaStrength, mfnrParams.sharpnessStrength, mfnrParams.spatioTemporalDenoiseBalanceStrength, mfnrParams.sharpnessScoreThreshold);
        Log.i(TAG,"configureMfnr, configureResult" + configureResult);
    }

    public boolean isDoingMfnr(){
        return isDoingMfnr;
    }

    public int getFrameNumbers(float realGain){
        int numFrames = 3;
        if (realGain <= 2.0f){
            numFrames = 3;
        } else if (realGain <= 4.0f) {
            numFrames = 4;
        } else if (realGain <= 8.0f){
            numFrames = 5;
        } else if (realGain <= 16.0f){
            numFrames = 6;
        } else if (realGain <= 32.0f) {
            numFrames = 7;
        } else {
            numFrames = 8;
        }
        Log.i(TAG,"numFrames: " + numFrames);
        return numFrames;
    }

    public void setGain(float gain){
        mGain = gain;
    }

    public void startMfnrProcess(CameraActivity activity, float imageGain, boolean isAIDEenabled) {
        mActivity = activity;
        mMfnrOut = ByteBuffer.allocateDirect(mStrideY * mHeight *3/2);
        Log.i(TAG,"mWidth:" + mWidth + ",mHeight:" + mHeight + ",strideY:" + mStrideY +",strideC:" + mStrideC);
        int processResult = mSwmfnrUtil.nativeMfnrRegisterAndProcess(getFrameNumbers(imageGain), mStrideY, mStrideC, mWidth, mHeight,
            mMfnrOut.array(), mOutRoi, imageGain, isAIDEenabled);
        mSwmfnrUtil.nativeMfnrDeAllocate();
        mSwmfnrUtil.nativeReleaseImage();
        isDoingMfnr = false;
        //param include the output
        Log.i(TAG, " processResult:" + processResult);
        for (int i =0;i < 4;i++){
            Log.i(TAG, " output roi:" + mOutRoi[i]);
        }
    }

    public void startAideProcess(long expTimeInNs, int iso, float denoiseStrength, int rGain, int bGain, int gGain){
        Log.i(TAG,"startAideProcess, expTimeInNs：" + expTimeInNs + ",iso:" + iso + ",denoiseStrength:" +denoiseStrength + ",rGain:" + rGain + "rGain:" + bGain + ",gGain:" + gGain );
        int[] inputFrameDim = {mWidth, mHeight, mStrideY};
        int[] outputFrameDim = {mWidth, mHeight, mStrideY};
        int result = mAideUtil.nativeAIDenoiserEngineCreate(inputFrameDim, outputFrameDim);
        mAideOut = ByteBuffer.allocateDirect(mStrideY*mHeight *3/2);

        mAideUtil.nativeAIDenoiserEngineProcessFrame(mMfnrOut.array(), mAideOut.array(), expTimeInNs,
            iso, denoiseStrength, rGain, bGain, gGain, mOutRoi);

        mAideUtil.nativeAIDenoiserEngineAbort();
        mAideUtil.nativeAIDenoiserEngineDestroy();
        Log.i(TAG,"aide process finished");
    }

    public byte[] generateImage(CameraActivity activity,boolean isMfnr, int orientation, Size pictureSize, Rect rect, TotalCaptureResult captureResult, int quality){

        Log.d(TAG,"src mstrideY="+mStrideY+" mStrideC="+mStrideC);
        int dataLength = mStrideY * mHeight * 3 /2;
        byte[] srcImage = new byte[dataLength];
        int offset = mStrideY * mHeight;

        if(isMfnr){
            srcImage = mMfnrOut.array();
        }else{
            srcImage = mAideOut.array();
        }
        if(rect.width() > (mOutRoi[2]-mOutRoi[0]) || rect.height() > mOutRoi[3]-mOutRoi[1]){
            rect = new Rect(mOutRoi[0],mOutRoi[1],mOutRoi[2],mOutRoi[3]);
        }
        Log.d(TAG,"cropYuvImage, rect:" + rect.toString());
        if((rect.left&1)==1) {
            rect.left = rect.left - 1;
        }
        if((rect.right&1)==1) {
            rect.right = rect.right - 1;
        }
        if((rect.top&1)==1) {
            rect.top = rect.top - 1;
        }
        if((rect.bottom&1)==1) {
            rect.bottom = rect.bottom - 1;
        }
        Log.d(TAG,"cropYuvImage, rect:" + rect.toString());
        srcImage = cropYuvImage(srcImage,mStrideY, mWidth, mHeight, rect);
        mActivity.getMediaSaveService().addRawImage(srcImage,"aftercrop","yuv");
        srcImage = nv21ToUpscaleJpeg(srcImage,orientation,rect.width(),rect.height(),
                pictureSize.getWidth(), pictureSize.getHeight(),captureResult, quality);
        Log.d(TAG,"test done");
        System.gc();
        return srcImage;
    }

    public void destoryMfnr(){
        int destoryResult = mSwmfnrUtil.nativeMfnrDestroy();
        Log.i(TAG, " destoryResult:" + destoryResult);
    }

    public ImageHandlerTask getImageHandler() {
        return mImageHandlerTask;
    }

    class ImageHandlerTask implements Runnable, ImageReader.OnImageAvailableListener {

        @Override
        public void onImageAvailable(ImageReader reader) {
            try {
                Log.i(TAG,"onImageAvailable");
                isDoingMfnr = true;
                Image image = reader.acquireNextImage();
                mWidth = image.getWidth();
                mHeight = image.getHeight();
                mStrideY = image.getPlanes()[0].getRowStride();
                mStrideC = image.getPlanes()[2].getRowStride();
                ByteBuffer dataY= image.getPlanes()[0].getBuffer();
                ByteBuffer dataUV = image.getPlanes()[2].getBuffer();
                dataY.rewind();
                dataUV.rewind();
                mSwmfnrUtil.nativeRegisterImage(dataY,mHeight*mStrideY,dataUV,mHeight*mStrideY/2);
                image.close();
                imagesNum.incrementAndGet(1);
            } catch (IllegalStateException e) {
                Log.e(TAG, "Max images has been already acquired. ");
            }
        }

        @Override
        public void run() {
        }
    }

    public void resetImagesNum(){
        imagesNum.set(0);
    }

    public void wantImagesNum(int num) {
        imagesNum.waitUntilIs(num);
    }

    class ProcessorHandler extends Handler {
        boolean isRunning;

        public ProcessorHandler(Looper looper) {
            super(looper);
            isRunning = true;
        }

        public void setInActive() {
            isRunning = false;
        }
    }

    public byte[] cropYuvImage(byte[] srcImage,int stride, int width, int height, Rect cropRect) {
        if (cropRect.left > width ||
                cropRect.top > height ||
                cropRect.left + cropRect.width() > width ||
                cropRect.top + cropRect.height() > height) {
            Log.e(TAG,"can crop this image with the roi "+cropRect.toString());
            return null;
        }
        Log.i(TAG,"cropYuvImage,cropRect:" + cropRect.toString());
        int x = cropRect.left;
        int y = cropRect.top;
        int w = cropRect.width();
        int h = cropRect.height();
        int retLength = cropRect.width() * cropRect.height() * 3 / 2;
        int uv_index_src = stride * height;
        int uv_index_dst = w * h;
        byte[] ret = new byte[retLength];
        for (int i = 0; i < h; i++){
            System.arraycopy(srcImage, (i + y) * stride + x, ret, i * w, w);
            if (i % 2 == 0){
                System.arraycopy(srcImage, uv_index_src+ (i+y)/2*stride + x, ret, uv_index_dst + i/2 * w, w);
            }
        }
        return ret;
    }

    public Bitmap nv21ToRgbAndResize(Context context, byte[] srcImage,int srcWidth, int srcHeight, int dstWidth,
                                     int dstHeight) {
        RenderScript rs = RenderScript.create(context);

        ScriptIntrinsicYuvToRGB yuvToRgbIntrinsic =
                ScriptIntrinsicYuvToRGB.create(rs, Element.U8_4(rs));

        Type.Builder yuvType = new Type.Builder(rs, Element.U8(rs))
                .setX(srcImage.length);
        Allocation yuvIn = Allocation.createTyped(rs, yuvType.create(), Allocation.USAGE_SCRIPT);

        Type.Builder rgbaType = new Type.Builder(rs, Element.RGBA_8888(rs))
                .setX(srcWidth)
                .setY(srcHeight);
        Allocation rgbaOut = Allocation.createTyped(rs, rgbaType.create(), Allocation.USAGE_SCRIPT);

        yuvIn.copyFrom(srcImage);

        yuvToRgbIntrinsic.setInput(yuvIn);
        yuvToRgbIntrinsic.forEach(rgbaOut);

        Bitmap rgba = Bitmap.createBitmap(srcWidth, srcHeight, Bitmap.Config.ARGB_8888);
        rgbaOut.copyTo(rgba);

        float scaleWidth = ((float) dstWidth) / srcWidth;
        float scaleHeight = ((float) dstHeight) / srcHeight;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);
        Bitmap ret = Bitmap.createBitmap(rgba, 0, 0, srcWidth, srcHeight, matrix,true);
        if (rgba != null & !rgba.isRecycled()){
            rgba.recycle();
            rgba = null;
        }

        return ret;
    }

    public byte[] bitmapToJpeg(Bitmap bitmap, int orientation, TotalCaptureResult result){
        BitmapOutputStream bos = new BitmapOutputStream(1024);
        bitmap.compress(Bitmap.CompressFormat.JPEG,85,bos);
        byte[] bytes = bos.getArray();
        bytes = addExifTags(bytes, orientation, result);
        return bytes;
    }

    public byte[] nv21ToUpscaleJpeg(byte[] srcImage,int orientation,int srcWidth, int srcHeight,
                             int dstWidth,int dstHeight,
                             TotalCaptureResult result,
                             int quality){
        BitmapOutputStream bos = new BitmapOutputStream(1024);
        YuvImage image = new YuvImage(srcImage,ImageFormat.NV21,srcWidth,srcHeight,
                new int[]{srcWidth,srcWidth});
        Rect outRoi = new Rect(0,0,srcWidth,srcHeight);
        image.compressToJpeg(outRoi, quality, bos);
        byte[] bytes = bos.getArray();
        Bitmap src = BitmapFactory.decodeByteArray(bytes,0,bytes.length);
        float scaleWidth = ((float) dstWidth) / srcWidth;
        float scaleHeight = ((float) dstHeight) / srcHeight;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);
        Bitmap finalImage = Bitmap.createBitmap(src, 0, 0, srcWidth, srcHeight, matrix,true);
        bos.reset();
        finalImage.compress(Bitmap.CompressFormat.JPEG,quality,bos);
        byte[] ret = bos.getArray();
        ret = addExifTags(ret, orientation, result);
        if (src != null & !src.isRecycled()){
            src.recycle();
            src = null;
        }
        return ret;
    }

    private class BitmapOutputStream extends ByteArrayOutputStream {
        public BitmapOutputStream(int size) {
            super(size);
        }

        public byte[] getArray() {
            return buf;
        }
    }
}
