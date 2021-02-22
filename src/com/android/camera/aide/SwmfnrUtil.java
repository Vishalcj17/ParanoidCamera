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

package com.android.camera.aide;

import android.util.Log;

import java.nio.ByteBuffer;
import java.util.List;
import android.os.SystemClock;
import com.android.camera.CameraActivity;
import com.android.camera.CaptureModule;

public class SwmfnrUtil {
    private static final String TAG = "SnapCam_SwmfnrUtil";

    private CaptureModule mController;
    private CameraActivity mActivity;
    private static boolean mIsSupported = false;

    public SwmfnrUtil() { }

    public native long nativeMfnrCreate();

    public native int nativeMfnrAllocate(int srcWidth, int srcHeight);

    public native int nativeMfnrConfigure(boolean enableGyroRefinement, int imageRegDesc, int imageRegMode, float deghostingStrength, float localMotionSmoothingStrength,
         float dtfSpatialYStrength, float dtfSpatialChromaStrength, float sharpnessStrength, float spatioTemporalDenoiseBalanceStrength, float sharpnessScoreThreshold);

    public native int nativeMfnrRegisterAndProcess(int numImages, int srcStrideY, int srcStrideC,
                                                   int srcWidth, int srcHeight, byte[] pDst, int[] roi, float imageGain, boolean isAIDEenabled); //pDst,roi are all output
    public native int nativeRegisterImage(ByteBuffer srcY, int yLength, ByteBuffer srcUV, int uvLength);

    public native int nativeReleaseImage();

    public native int nativeMfnrDeAllocate();

    public native int nativeMfnrDestroy();

    static {
        try {
            System.loadLibrary("jni_mfnrutil");
            Log.i(TAG, "load jni_mfnrutil successfully");
            mIsSupported = true;
        } catch (UnsatisfiedLinkError e) {
            mIsSupported = false;
            Log.d(TAG, e.toString());
        }
    }

    public static boolean isSwmfnrSupported(){
        return mIsSupported;
    }

    public static class MfnrTunableParams {
        //tuning params get from capture
        public boolean enableGyroRefinement;
        public int imageRegDesc = 1;
        public int imageRegMode;
        public float deghostingStrength = 0.5f;
        public float localMotionSmoothingStrength = 1f;
        public float dtfSpatialYStrength = 0.0f;
        public float dtfSpatialChromaStrength = 0.0f;
        public float sharpnessStrength = 0.5f;
        public float spatioTemporalDenoiseBalanceStrength = 0.5f;
        public float sharpnessScoreThreshold = 0.85f;

        public MfnrTunableParams(boolean enableGyroRefinement, int imageRegDesc, int imageRegMode, float deghostingStrength, float localMotionSmoothingStrength,
                 float dtfSpatialYStrength, float dtfSpatialChromaStrength , float sharpnessStrength, float spatioTemporalDenoiseBalanceStrength, float sharpnessScoreThreshold) {
            this.enableGyroRefinement = enableGyroRefinement;
            this.imageRegDesc = imageRegDesc;
            this.imageRegMode = imageRegMode;
            this.deghostingStrength =  deghostingStrength;
            this.localMotionSmoothingStrength = localMotionSmoothingStrength;
            this.dtfSpatialYStrength =  dtfSpatialYStrength;
            this.dtfSpatialChromaStrength = dtfSpatialChromaStrength;
            this.sharpnessStrength =  sharpnessStrength;
            this.spatioTemporalDenoiseBalanceStrength = spatioTemporalDenoiseBalanceStrength;
            this.sharpnessScoreThreshold = sharpnessScoreThreshold;
        }
    }

    public static class FrameMetaData {
        public int frameId;
        public int gyroNumElements;
        public GyroRotationData[] pGyroData;
    }

    public static class GyroRotationData {
        public float xRotation;
        public float yRotation;
        public float zRotation;
        public int timeStamp;
    }

    public static class RoiWindow {
        public int x;
        public int y;
        public int dx;
        public int dy;
    }

    public static class RegisterAndProcessParam {
        public int numImages;
        public int srcStrideY;
        public int srcStrideC;
        public int srcWidth;
        public int srcHeight;
        public byte[] pDstY;
        public byte[] pDstC;
        public int dstStrideY;
        public int dstStrideC;
        //FrameMetaData[] pMetadataArray;  //This parameter is not filled for now. This is a placehoder for Gyro data
        public RoiWindow outputRoi;
        //They are passed in as "call by reference" via the API, inside MFNR algorithm, we compute and assign values to these
        //those will be sent to Post filter OPE after MFNR for further filtering
        //You can set those values to 0, if you want them to initialized, before calling MFNR
        public float blendConfidence; // what confidence they got blended
        public int nBlendedFrames; //N frame given, how many frames got actually blended

        public RegisterAndProcessParam(int numImages, int srcStrideY, int srcStrideC, int srcWidth, int srcHeight) {
            this.numImages = numImages;
            this.srcStrideY = srcStrideY;
            this.srcStrideC = srcStrideC;
            this.srcWidth =  srcWidth;
            this.srcHeight = srcHeight;
            dstStrideY = 0;
            dstStrideC = 0;
            outputRoi = new RoiWindow();
        }

        public byte[] getDstY(){
            return pDstY;
        }

        public byte[] getDstC(){
            return pDstC;
        }
        public int getDstStrideY(){
            return dstStrideY;
        }

        public int getDstStrideC(){
            return dstStrideC;
        }
    }
}