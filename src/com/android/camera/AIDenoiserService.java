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

import android.graphics.ImageFormat;
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

public class AIDenoiserService extends Service {

    private static final int MAX_REQUIRED_IMAGE_NUM = 11;
    private CaptureModule mController;
    private AideUtil mAideUtil;
    private SwmfnrUtil mSwmfnrUtil;
    private static final String TAG = "snapcam_aideservice";
    private LinkedList<ZSLQueue.ImageItem> mMfnrImages = new LinkedList<ZSLQueue.ImageItem>();
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
    Semaphore mLock = new Semaphore(1);
    CameraActivity mActivity;

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
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        return 0;
    }

    @Override
    public void onDestroy() {
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
    }

    @Override
    public void onCreate() {
    }

    public void startAiDenoiserProcess() {

    }

    public void createMfnr(CaptureModule module) {
        mController = module;
        mMfnrQueue = new ZSLQueue(mController);
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

    public void startMfnrProcess(CameraActivity activity, float imageGain, boolean isAIDEenabled) {
        mActivity = activity;
        if (!mLock.tryAcquire()) {
            return;
        }
        ByteBuffer[] mSrcY = new ByteBuffer[5];
        ByteBuffer[] mSrcC = new ByteBuffer[5];
        byte[][] pSrcY = new byte[5][];
        byte[][] pSrcC = new byte[5][];
        try {
            List<ZSLQueue.ImageItem> itemsList = mMfnrQueue.getAllItems();
            ZSLQueue.ImageItem[] items = itemsList.toArray(new ZSLQueue.ImageItem[itemsList.size()]);
            Log.i(TAG,"startMfnrProcess， items.size: " + itemsList.size());
            int processSize = 5;
            if (itemsList.size() < processSize) {
                for (int i = 0; i < items.length; i++){
                    Image image = items[i].getImage();
                    image.close();
                }
                return;
            }
            mWidth = items[0].getImage().getWidth();
            mHeight = items[0].getImage().getHeight();
            mStrideY = items[0].getImage().getPlanes()[0].getRowStride();
            mStrideC = items[0].getImage().getPlanes()[2].getRowStride();

            for (int i =0;i <processSize; i++){
                Log.i(TAG, "startMfnrProcess， get item, i = " + i);
                Image image = items[i].getImage();
                ByteBuffer yBuf = image.getPlanes()[0].getBuffer();
                ByteBuffer cBuf = image.getPlanes()[2].getBuffer();
                mSrcY[i] = ByteBuffer.allocateDirect(mStrideY * mHeight);
                mSrcC[i] = ByteBuffer.allocateDirect(mStrideC * mHeight / 2);
                yBuf.get(mSrcY[i].array(), 0, yBuf.remaining());
                cBuf.get(mSrcC[i].array(), 1, cBuf.remaining());
                byte[] dataY = new byte[mStrideY*mHeight];
                mSrcY[i].get(dataY,0,mSrcY[i].remaining());
                pSrcY[i] = dataY;
                Log.i(TAG, "startMfnrProcess， yBuf, size=  " + yBuf.capacity() +",c,size:" + cBuf.capacity() + ",srcY size= " + dataY.length);
                byte[] dataC = new byte[mStrideC*mHeight/2];
                mSrcC[i].get(dataC,0,mSrcC[i].remaining());
                pSrcC[i] = dataC;

                byte[] data = new byte[mStrideY * mHeight * 3 /2];
                System.arraycopy(pSrcY[i], 0, data, 0, pSrcY[i].length);
                System.arraycopy(pSrcC[i], 0, data, pSrcY[i].length, pSrcC[i].length);
                mActivity.getMediaSaveService().addRawImage(data,"mfnrinput" + i,"yuv");
            }
            mMfnrOut = ByteBuffer.allocateDirect(mStrideY * mHeight *3/2);
            Log.i(TAG,"mWidth:" + mWidth + ",mHeight:" + mHeight + ",strideY:" + mStrideY +",strideC:" + mStrideC);
            int processResult = mSwmfnrUtil.nativeMfnrRegisterAndProcess(pSrcY,pSrcC,5,mStrideY, mStrideC, mWidth, mHeight,
                mMfnrOut.array(), mOutRoi, imageGain, isAIDEenabled);
            mSwmfnrUtil.nativeMfnrDeAllocate();
            for (int i = 0; i < items.length; i++){
                Image image = items[i].getImage();
                Log.i(TAG,"imgae close");
                image.close();
            }
            //param include the output
            Log.i(TAG, " processResult:" + processResult);
            for (int i =0;i < 4;i++){
                Log.i(TAG, " output roi:" + mOutRoi[i]);
            }
        } finally {
             mLock.release();
        }

    }

    public void startAideProcess(long expTimeInNs, int iso, float denoiseStrength, int rGain, int bGain, int gGain){
        Log.i(TAG,"startAideProcess, expTimeInNs：" + expTimeInNs + ",iso:" + iso + ",denoiseStrength:" +denoiseStrength + ",rGain:" + rGain + "rGain:" + bGain + ",gGain:" + gGain );
        int[] inputFrameDim = {mWidth, mHeight, mStrideY};
        int[] outputFrameDim = {mWidth, mHeight, mStrideY};
        int result = mAideUtil.nativeAIDenoiserEngineCreate(inputFrameDim, outputFrameDim);
        mAideOut = ByteBuffer.allocate(mStrideY*mHeight *3/2);

        mActivity.getMediaSaveService().addRawImage(mMfnrOut.array(),"aideinput","yuv");
        mAideUtil.nativeAIDenoiserEngineProcessFrame(mMfnrOut.array(), mAideOut.array(), expTimeInNs,
            iso, denoiseStrength, rGain, bGain, gGain, mOutRoi);
        mActivity.getMediaSaveService().addRawImage(mAideOut.array(),"aideoutput","yuv");

        mAideUtil.nativeAIDenoiserEngineAbort();
        mAideUtil.nativeAIDenoiserEngineDestroy();
    }

    public byte[] generateImage(CameraActivity activity,boolean isMfnr, int orientation, Size pictureSize, Rect rect){

        Log.d(TAG,"src mstrideY="+mStrideY+" mStrideC="+mStrideC);
        int dataLength = mStrideY * mHeight * 3 /2;
        byte[] srcImage = new byte[dataLength];
        int offset = mStrideY * mHeight;

        if(isMfnr){
            mMfnrOut.get(srcImage,0,mMfnrOut.remaining());
        }else{
            mAideOut.get(srcImage,0,mAideOut.remaining());
        }
        Log.d(TAG,"cropYuvImage");
        if(rect.width() > (mOutRoi[2]-mOutRoi[0]) || rect.height() > mOutRoi[3]-mOutRoi[1]){
            rect = new Rect(mOutRoi[0],mOutRoi[1],mOutRoi[2],mOutRoi[3]);
        }
        srcImage = cropYuvImage(srcImage,mStrideY, mWidth, mHeight, rect);
        Log.d(TAG,"nv21ToRgbAndResize, desWidth:" + pictureSize.getWidth() + ",height:" + pictureSize.getHeight());
        Bitmap bitmap = nv21ToRgbAndResize(activity, srcImage,rect.width(), rect.height(), pictureSize.getWidth(), pictureSize.getHeight());
        Log.d(TAG,"bitmapToJpeg");
        srcImage = bitmapToJpeg(bitmap, orientation,null);
        Log.d(TAG,"test done");
        System.gc();
        return srcImage;
    }

    public void destoryMfnr(){
        int destoryResult = mSwmfnrUtil.nativeMfnrDestroy();
        if(mMfnrQueue != null) {
            mMfnrQueue.onClose();
            mMfnrQueue = null;
        }
        clearImages();
        Log.i(TAG, " destoryResult:" + destoryResult);
    }

    private void clearImages() {
        for(ZSLQueue.ImageItem item: mMfnrImages) {
            try {
                item.getImage().close();
                Image raw = item.getRawImage();
                if (raw != null) {
                    raw.close();
                }
            } catch(Exception e) {
            }
        }
        mMfnrImages.clear();
    }

    public ImageHandlerTask getImageHandler() {
        return mImageHandlerTask;
    }

    private void addImage(ZSLQueue.ImageItem item) {
        mMfnrImages.add(item);
        if(mMfnrImages.size() >= MAX_REQUIRED_IMAGE_NUM - 1) {
            ZSLQueue.ImageItem it = mMfnrImages.getFirst();
            try {
                it.getImage().close();
                Image raw = item.getRawImage();
                if (raw != null) {
                    raw.close();
                }
            } catch(Exception e) {
            }
            mMfnrImages.removeFirst();
        }
    }

    class ImageHandlerTask implements Runnable, ImageReader.OnImageAvailableListener {
        Semaphore mMutureLock = new Semaphore(1);
        private ImageWrapper mImageWrapper = null;

        @Override
        public void onImageAvailable(ImageReader reader) {
            try {
                Image image = reader.acquireNextImage();
                if (!mMutureLock.tryAcquire()) {
                    image.close();
                    return;
                }
                if (mImageWrapper == null || mImageWrapper.isTaken()) {
                    mImageWrapper = new ImageWrapper(image);
                    mMutureLock.release();
                } else {
                    image.close();
                    mMutureLock.release();
                }
                if (mHandler != null) {
                    mHandler.post(this);
                }
            } catch (IllegalStateException e) {
                Log.e(TAG, "Max images has been already acquired. ");
            }
        }

        @Override
        public void run() {
            if(mImageWrapper != null) {
                Image image = mImageWrapper.getImage();
                if (mMfnrQueue != null) {
                    if (!mLock.tryAcquire()) {
                        image.close();
                        return;
                    }
                    mMfnrQueue.add(image, null);
                    mLock.release();
                }
            }
        }
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

    private class ImageWrapper {
        Image mImage;
        Image mRawImage;
        boolean mIsTaken;

        public ImageWrapper(Image image) {
            mImage = image;
            mRawImage = null;
            mIsTaken = false;
        }

        public boolean isTaken() {
            return mIsTaken;
        }

        public Image getImage() {
            mIsTaken = true;
            return mImage;
        }

        public Image getRawImage() {
            return mRawImage;
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

    public Bitmap nv21ToRgbAndResize(CameraActivity activity, byte[] srcImage,int srcWidth, int srcHeight, int dstWidth,
                                     int dstHeight) {
        RenderScript rs = RenderScript.create(activity.getApplicationContext());

        ScriptIntrinsicYuvToRGB yuvToRgbIntrinsic =
                ScriptIntrinsicYuvToRGB.create(rs, Element.U8_4(rs));
        ScriptIntrinsicResize resizeIntrinsic = ScriptIntrinsicResize.create(rs);

        Type.Builder yuvType = new Type.Builder(rs, Element.U8(rs))
                .setX(srcImage.length);
        Allocation yuvIn = Allocation.createTyped(rs, yuvType.create(), Allocation.USAGE_SCRIPT);

        Type.Builder rgbaType = new Type.Builder(rs, Element.RGBA_8888(rs))
                .setX(srcWidth)
                .setY(srcHeight);
        Allocation rgbaOut = Allocation.createTyped(rs, rgbaType.create(), Allocation.USAGE_SCRIPT);

        Type.Builder rgbaResizeType = new Type.Builder(rs, Element.RGBA_8888(rs))
                .setX(dstWidth)
                .setY(dstHeight);

        Allocation rgbaResizeOut = Allocation.createTyped(rs,rgbaResizeType.create(),
                Allocation.USAGE_SCRIPT);

        yuvIn.copyFrom(srcImage);

        yuvToRgbIntrinsic.setInput(yuvIn);
        yuvToRgbIntrinsic.forEach(rgbaOut);

        resizeIntrinsic.setInput(rgbaOut);
        resizeIntrinsic.forEach_bicubic(rgbaResizeOut);

        Bitmap ret = Bitmap.createBitmap(dstWidth, dstHeight, Bitmap.Config.ARGB_8888);
        rgbaResizeOut.copyTo(ret);

        return ret;
    }

    public byte[] bitmapToJpeg(Bitmap bitmap, int orientation, TotalCaptureResult result){
        BitmapOutputStream bos = new BitmapOutputStream(1024);
        bitmap.compress(Bitmap.CompressFormat.JPEG,85,bos);
        byte[] bytes = bos.getArray();
        bytes = PostProcessor.addExifTags(bytes, orientation, result);
        return bytes;
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