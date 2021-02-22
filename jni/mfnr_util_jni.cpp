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

#include <jni.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#else
#include <Windows.h>
#include <process.h>
#define  getpid()    _getpid()
#endif
#define MAX_ROW 8

#ifndef _LINUX
#define _LINUX
#endif
#include "camxtypes.h"

#include "camxmfnrwrapper.h"
#include <android/log.h>
#include <fstream>
#include <iostream>
#include <vector>
#define LOG_TAG "MfnrJni"

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jlong JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrCreate(
        JNIEnv* env, jobject thiz);
JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrAllocate(
        JNIEnv *env, jobject thiz, jint srcWidth, jint srcHeight);
JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrConfigure(
        JNIEnv* env, jobject thiz, jboolean enableGyroRefinement, jint imageRegDesc, jint imageRegMode, jfloat deghostingStrength,
        jfloat localMotionSmoothingStrength, jfloat dtfSpatialYStrength, jfloat dtfSpatialChromaStrength, jfloat sharpnessStrength, jfloat spatioTemporalDenoiseBalanceStrength, jfloat sharpnessScoreThreshold);
/*
JNIEXPORT jint JNICALL Java_com_android_camera_SwmfnrUtil_nativeMfnrRegister(
        JNIEnv* env, jobject thiz, jlong sessionId,
        jboolean pRefY,
        jboolean pRefC,
        jint refStrideY,
        jint refStrideC,
        jboolean pSrcY,
        jboolean pSrcC,
        jint srcStrideY,
        jint srcStrideC,
        jint width,
        jint height,
        jboolean pDstY,
        jboolean pDstC,
        jint dstStrideY,
        jint dstStrideC,
        jobject pMetadata,
        jobject &roiImage,
        jfloat sharpnessScore);
JNIEXPORT CamxResult JNICALL Java_com_android_camera_SwmfnrUtil_nativeMfnrProcess(
        JNIEnv* env, jobject thiz, jlong sessionId,
         jbyteArray pSrcY,
         jbyteArray pSrcC,
         jint numImages,
         jint srcStrideY,
         jint srcStrideC,
         jint srcWidth,
         jint srcHeight,
         jboolean pDstY,
         jboolean pDstC,
         jint dstStrideY,
         jint dstStrideC,
         const qrcpdefs::RoiWindow * const roiWindowPerImage,
         jfloat sharpnessScoreArray,
         qrcpdefs::RoiWindow &outputRoi,
         jfloat blendConfidence,
         jint& nBlendedFrames);
           */
JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrRegisterAndProcess(
        JNIEnv* env, jobject thiz, jint numImages, jint srcStrideY, jint srcStrideC,
        jint srcWidth, jint srcHeight, jbyteArray pDst, jintArray roi, jfloat imageGain, jboolean isAIDEenabled);

JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrDeAllocate(
        JNIEnv* env, jobject thiz);

JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrDestroy(
        JNIEnv* env, jobject thiz);

JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeRegisterImage(
        JNIEnv* env, jobject thiz, jobject srcY, int ylength, jobject srcCr, int crlength);

JNIEXPORT jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeReleaseImage(
        JNIEnv* env, jobject thiz);
#ifdef __cplusplus
}
#endif


using namespace qrcpdefs;
#define printf(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__ )

CamxResult LoadMFNRlib(uint8_t nCoresLib, void* fPtr);

typedef unsigned char uint8_t;
qrcpdefs::mfnrLib* m_funPtrs = (qrcpdefs::mfnrLib*) malloc(sizeof(qrcpdefs::mfnrLib));
UINTPTR_T sessionId;
uint8_t* cpSrcY[MAX_ROW];
uint8_t* cpSrcC[MAX_ROW];
int cpSrcIndex = 0;

jlong JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrCreate(
        JNIEnv* env, jobject thiz) {

    if (CamxResultSuccess != LoadMFNRlib(qrcpdefs::EIGHT_CORES, m_funPtrs))
    {
        printf("lib mfnr dlopen failed" );
        return -1;
    }
    sessionId = qrcp::MfnrCreate(m_funPtrs);
    printf("sessionId= %d " , sessionId);
    return sessionId;
}

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrAllocate(
        JNIEnv *env, jobject thiz, jint srcWidth, jint srcHeight) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"MfnrAllocate,%d",srcWidth);
    printf("MfnrAllocate_sessionId= %d " , sessionId);
    return qrcp::MfnrAllocate(sessionId, (UINT32)srcWidth, (UINT32)srcHeight, m_funPtrs);
}

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrConfigure(
        JNIEnv* env, jobject thiz,jboolean enableGyroRefinement, jint imageRegDesc, jint imageRegMode, jfloat deghostingStrength,
                 jfloat localMotionSmoothingStrength, jfloat dtfSpatialYStrength, jfloat dtfSpatialChromaStrength, jfloat sharpnessStrength, jfloat spatioTemporalDenoiseBalanceStrength, jfloat sharpnessScoreThreshold) {

    qrcp::MfnrTunableParams _params;
    _params.enableGyroRefinement = (bool)enableGyroRefinement;
    _params.imageRegDesc = (qrcpdefs::ImageRegistrationDescriptorSelector)imageRegDesc;
    _params.imageRegMode = (qrcpdefs::ImageRegistrationAccuracyMode)imageRegMode;
    _params.deghostingStrength = (float)deghostingStrength;
    _params.localMotionSmoothingStrength = (float)localMotionSmoothingStrength;
    _params.dtfSpatialYStrength = (float)dtfSpatialYStrength;
    _params.dtfSpatialChromaStrength = (float)dtfSpatialChromaStrength;
    _params.sharpnessStrength = (float)sharpnessStrength;
    _params.spatioTemporalDenoiseBalanceStrength = (float)spatioTemporalDenoiseBalanceStrength;
    _params.sharpnessScoreThreshold = (float)sharpnessScoreThreshold;
    printf("MfnrConfigure_sessionId= %d " , sessionId);
    return MfnrConfigure(sessionId, _params, m_funPtrs);
}

void WriteData(FILE *fp, unsigned char *pStart, int width, int height, int stride)
{
    for (int i = 0; i < height; i++)
    {
        fwrite(pStart, stride, 1, fp);
        pStart += stride;
    }
}

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeRegisterImage(
        JNIEnv* env, jobject thiz, jobject srcY, int ylength, jobject srcCr, int crlength)
    {
       jbyte* srcYArray = (jbyte*)env->GetDirectBufferAddress(srcY);
       jbyte* srcCrArray = (jbyte*)env->GetDirectBufferAddress(srcCr);
       cpSrcY[cpSrcIndex] = (uint8_t*)malloc(ylength * sizeof(uint8_t));
       cpSrcC[cpSrcIndex] = (uint8_t*)malloc(crlength * sizeof (uint8_t));
       memcpy(cpSrcY[cpSrcIndex], srcYArray, ylength * sizeof (uint8_t));
       memcpy(cpSrcC[cpSrcIndex], srcCrArray, crlength * sizeof (uint8_t));
       cpSrcIndex++;
       return 0;
    }


jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeReleaseImage(
        JNIEnv* env, jobject thiz)
    {
       for(int i = cpSrcIndex; i > 0; i--) {
            cpSrcIndex--;
            printf("cpSrcIndex=%d ", cpSrcIndex);
            if ( cpSrcY[cpSrcIndex] != NULL){
               free(cpSrcY[cpSrcIndex]);
            }
            if ( cpSrcC[cpSrcIndex] != NULL){
               free(cpSrcC[cpSrcIndex]);
            }
        }
        return 0;
    }

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrRegisterAndProcess(
        JNIEnv* env, jobject thiz, jint numImages, jint srcStrideY, jint srcStrideC,
        jint srcWidth, jint srcHeight, jbyteArray pDst, jintArray roi, jfloat imageGain, jboolean isAIDEenabled){

    printf("numImages=%d ", numImages);

    //outout buffer
    jbyte* imageDataNV21Array = env->GetByteArrayElements(pDst, NULL);
    uint8_t *out = (uint8_t*)imageDataNV21Array;
    uint8_t* outAddrY = (uint8_t*)&(out[0]);
    uint8_t* outAddrVU = (uint8_t*)&(out[srcStrideY*srcHeight]);

    uint32_t* outRoi = new uint32_t[4];
    qrcpdefs::RoiWindow outputRoi;
    std::vector<qrcpdefs::FrameMetaData> frameMetaDataPerImage( numImages );
    std::vector<qrcpdefs::FrameMetaData *> frameMetaDataPerImagePtr(numImages);
    for (int i = 0; i < numImages; i++)
    {
       frameMetaDataPerImagePtr[i] = frameMetaDataPerImage.data();
    }
    float blendConfidence = 0;
    UINT32 nBlendedFrames = 0;
    //frameMetaDataPerImagePtr is not used now

    for (int i = 0; i < 8; i++)
    {
        char fileName[256];
        snprintf(fileName, sizeof(fileName), "/data/data/org.codeaurora.snapcam/files/input%d.yuv", i);
        remove(fileName);
    }

    for (int i = 0; i < numImages; i++)
    {
        char fileName[256];
        snprintf(fileName, sizeof(fileName), "/data/data/org.codeaurora.snapcam/files/input%d.yuv", i);
        FILE *inputFile = fopen(fileName, "wb+");
        if ((inputFile != NULL)){
            WriteData(inputFile, cpSrcY[i], srcWidth, srcHeight, srcStrideY);
            WriteData(inputFile, cpSrcC[i], srcWidth, srcHeight/2, srcStrideY);
            fclose(inputFile);
        } else {
            printf( "inputFile is NULL");
        }
    }

    printf("MfnrRegisterAndProcess_sessionId= %d " , sessionId);
    CamxResult result = qrcp::MfnrRegisterAndProcess(sessionId, cpSrcY, cpSrcC, numImages, srcStrideY, srcStrideC, srcWidth, srcHeight, outAddrY, outAddrVU,
        srcStrideY, srcStrideY, &frameMetaDataPerImagePtr.front(), outputRoi, blendConfidence, nBlendedFrames, m_funPtrs, imageGain, isAIDEenabled);
    printf("result=%d" ,result);
    printf("setoutput1,roix=%d,y=%d,dx=%d,dy=%d",outputRoi.x,outputRoi.y,outputRoi.dx,outputRoi.dy );
    outRoi[0] = outputRoi.x;
    outRoi[1] = outputRoi.y;
    outRoi[2] = outputRoi.dx;
    outRoi[3] = outputRoi.dy;

    //set out put
    printf("setoutput2" );
    env->SetIntArrayRegion(roi, 0, 4, (jint *)outRoi);
    env->ReleaseByteArrayElements(pDst, imageDataNV21Array, JNI_ABORT);
    printf("write out put file to vendor" );
    FILE *pFile = fopen("/data/data/org.codeaurora.snapcam/files/mfnrout.yuv", "wb+");

    if ((pFile != NULL)){
        WriteData(pFile, outAddrY, srcWidth, srcHeight, srcStrideY);
        WriteData(pFile, outAddrVU, srcWidth, srcHeight/2, srcStrideY);
        fclose(pFile);
    } else {
        printf( " pFile is NULL");
    }

    delete outRoi;
    return result;
}

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrDeAllocate(
        JNIEnv* env, jobject thiz) {
    return qrcp::MfnrDeAllocate(sessionId, m_funPtrs);
}

jint JNICALL Java_com_android_camera_aide_SwmfnrUtil_nativeMfnrDestroy(
        JNIEnv* env, jobject thiz) {
    return qrcp::MfnrDestroy(sessionId, m_funPtrs);
}

char mfnr_lib_name[1024] = { "/vendor/lib64/libmmcamera_mfnr.so" };
char mfnr_4threads_lib_name[1024] = { "/vendor/lib64/libmmcamera_mfnr_t4.so" };

CamxResult LoadMFNRlib(uint8_t nCoresLib, void* fPtr)
{
    qrcpdefs::mfnrLib* pMfnrLibPtr = (qrcpdefs::mfnrLib*)fPtr;

#ifndef _MSC_VER

    if ((qrcpdefs::NcoreLibrary)nCoresLib == qrcpdefs::NcoreLibrary::EIGHT_CORES)
        pMfnrLibPtr->ptr = dlopen(mfnr_lib_name, RTLD_NOW | RTLD_LOCAL);
    else if ((qrcpdefs::NcoreLibrary)nCoresLib == qrcpdefs::NcoreLibrary::FOUR_CORES)
        pMfnrLibPtr->ptr = dlopen(mfnr_4threads_lib_name, RTLD_NOW | RTLD_LOCAL);
    else
    {
        printf("\n Invalid MFNR library to open");
        return CamxResultEFailed;
    }

    if (!pMfnrLibPtr->ptr)
    {
        const char* log = dlerror();

        if (log != NULL) {
            printf("Error during dlopen() of %s\n", mfnr_lib_name);
        }
        return CamxResultEFailed;
    }

    printf("\n Library Open Success: %s", mfnr_lib_name);

    *(void **)&(pMfnrLibPtr->Mfnry303) = dlsym(pMfnrLibPtr->ptr, "Mfnry303");
    if (!pMfnrLibPtr->Mfnry303)
    {
        printf("Error during dlsym() of Mfnry303 \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->Mfnry101) = dlsym(pMfnrLibPtr->ptr, "Mfnry101");
    if (!pMfnrLibPtr->Mfnry303)
    {
        printf("Error during dlsym() of Mfnry101 \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->Mfnry202) = dlsym(pMfnrLibPtr->ptr, "Mfnry202");
    if (!pMfnrLibPtr->Mfnry303)
    {
        printf("Error during dlsym() of Mfnry202 \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->Mfnry404) = dlsym(pMfnrLibPtr->ptr, "Mfnry404");
    if (!pMfnrLibPtr->Mfnry303)
    {
        printf("Error during dlsym() of Mfnry202 \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnrAbortFrame) = dlsym(pMfnrLibPtr->ptr, "MfnrAbortFrame");
    if (!pMfnrLibPtr->MfnrAbortFrame)
    {
        printf("Error during dlsym() of MfnrAbortFrame \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnrIsFrameAborted) = dlsym(pMfnrLibPtr->ptr, "MfnrIsFrameAborted");
    if (!pMfnrLibPtr->MfnrIsFrameAborted)
    {
        printf("Error during dlsym() of MfnrIsFrameAborted \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnrResetFrameAbortFlag) = dlsym(pMfnrLibPtr->ptr, "MfnrResetFrameAbortFlag");
    if (!pMfnrLibPtr->MfnrResetFrameAbortFlag)
    {
        printf("Error during dlsym() of MfnrResetFrameAbortFlag \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnrSetFrameRunningState) = dlsym(pMfnrLibPtr->ptr, "MfnrSetFrameRunningState");
    if (!pMfnrLibPtr->MfnrSetFrameRunningState)
    {
        printf("Error during dlsym() of MfnrSetFrameRunningState \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->Mfnruf6H4LqsSk) = dlsym(pMfnrLibPtr->ptr, "Mfnruf6H4LqsSk");
    if (!pMfnrLibPtr->Mfnruf6H4LqsSk)
    {
        printf("Error during dlsym() of Mfnruf6H4LqsSk \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->Mfnrjmf9K6K5NQ) = dlsym(pMfnrLibPtr->ptr, "Mfnrjmf9K6K5NQ");
    if (!pMfnrLibPtr->Mfnrjmf9K6K5NQ)
    {
        printf("Error during dlsym() of Mfnrjmf9K6K5NQ \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnriKtXNRGAUM) = dlsym(pMfnrLibPtr->ptr, "MfnriKtXNRGAUM");
    if (!pMfnrLibPtr->MfnriKtXNRGAUM)
    {
        printf("Error during dlsym() of MfnriKtXNRGAUM \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->MfnryPGmb6U2bt) = dlsym(pMfnrLibPtr->ptr, "MfnryPGmb6U2bt");
    if (!pMfnrLibPtr->MfnryPGmb6U2bt)
    {
        printf("Error during dlsym() of MfnryPGmb6U2bt \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    *(void **)&(pMfnrLibPtr->EstimateSharpnessScoreAndSortAnchorToFirst) = dlsym(pMfnrLibPtr->ptr, "EstimateSharpnessScoreAndSortAnchorToFirst");
    if (!pMfnrLibPtr->EstimateSharpnessScoreAndSortAnchorToFirst)
    {
        printf("Error during dlsym() of EstimateSharpnessScoreAndSortAnchorToFirst \n");
        dlclose(pMfnrLibPtr->ptr);
        pMfnrLibPtr->ptr = NULL;
        return CamxResultEFailed;
    }

    printf("\n MFNR Library Function Load Success");

#else

    *(void **)&(pMfnrLibPtr->Mfnry303) = Mfnry303;

    *(void **)&(pMfnrLibPtr->Mfnry101) = Mfnry101;

    *(void **)&(pMfnrLibPtr->Mfnry202) = Mfnry202;

    *(void **)&(pMfnrLibPtr->Mfnry404) = Mfnry404;

    *(void **)&(pMfnrLibPtr->MfnrAbortFrame) = MfnrAbortFrame;

    *(void **)&(pMfnrLibPtr->MfnrIsFrameAborted) = MfnrIsFrameAborted;

    *(void **)&(pMfnrLibPtr->MfnrResetFrameAbortFlag) = MfnrResetFrameAbortFlag;

    *(void **)&(pMfnrLibPtr->MfnrSetFrameRunningState) = MfnrSetFrameRunningState;

    *(void **)&(pMfnrLibPtr->Mfnruf6H4LqsSk) = Mfnruf6H4LqsSk;

    *(void **)&(pMfnrLibPtr->Mfnrjmf9K6K5NQ) = Mfnrjmf9K6K5NQ;

    *(void **)&(pMfnrLibPtr->MfnriKtXNRGAUM) = MfnriKtXNRGAUM;

    *(void **)&(pMfnrLibPtr->MfnryPGmb6U2bt) = MfnryPGmb6U2bt;

    *(void **)&(pMfnrLibPtr->EstimateSharpnessScoreAndSortAnchorToFirst) = EstimateSharpnessScoreAndSortAnchorToFirst;

#endif

    return CamxResultSuccess;
}

/* do not use these apis
jint JNICALL Java_com_android_camera_SwmfnrUtil_nativeMfnrRegister(
        JNIEnv* env, jobject thiz, jlong sessionId,
        jboolean pRefY,
        jboolean pRefC,
        jint refStrideY,
        jint refStrideC,
        jboolean pSrcY,
        jboolean pSrcC,
        jint srcStrideY,
        jint srcStrideC,
        jint width,
        jint height,
        jboolean pDstY,
        jboolean pDstC,
        jint dstStrideY,
        jint dstStrideC,
        jobject pMetadata,
        jobject &roiImage,
        jfloat sharpnessScore) {

    jclass frameMetaDataClass = env->FindClass("com/android/camera/SwmfnrUtil.FrameMetaData");
    FrameMetaData _pMetadata;
    _pMetadata.frameId = env->GetIntField(pMetadata, env->GetFieldID(frameMetaDataClass, "frameId", "I"));
    _pMetadata.gyroNumElements = env->GetIntField(pMetadata, env->GetFieldID(frameMetaDataClass, "gyroNumElements", "I"));
    jfieldID pGyroDataId = env->GetFieldID(frameMetaDataClass, "pGyroData", "Lcom/android/camera/SwmfnrUtil.GyroRotationData;");
    jobject pGyroData = env->GetObjectField(pMetadata, pGyroDataId);
    jclass gyroRotationDataClass = env->FindClass("com/android/camera/SwmfnrUtil.GyroRotationData");
    _pMetadata.pGyroData.xRotation = env->GetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "xRotation", "F"));
    _pMetadata.pGyroData.yRotation = env->GetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "yRotation", "F"));
    _pMetadata.pGyroData.zRotation = env->GetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "zRotation", "F"));
    _pMetadata.pGyroData.timeStamp = env->GetIntField(pGyroData, env->GetFieldID(gyroRotationDataClass, "timeStamp", "I"));

    jclass roiWindowClass = env->FindClass("com/android/camera/SwmfnrUtil.RoiWindow");
    RoiWindow _roiImage;
    _roiImage.x = env->GetIntField(roiImage, env->GetFieldID(roiWindowClass, "x", "I"));
    _roiImage.y = env->GetIntField(roiImage, env->GetFieldID(roiWindowClass, "y", "I"));
    _roiImage.dx = env->GetIntField(roiImage, env->GetFieldID(roiWindowClass, "dx", "I"));
    _roiImage.dy = env->GetIntField(roiImage, env->GetFieldID(roiWindowClass, "dy", "I"));
    jint result = MfnrRegister((UINTPTR_T)sessionId, (UINT8)pRefY, (UINT8)pRefC, (UINT32)refStrideY, (UINT32)refStrideC, (UINT8)pSrcY, (UINT8)pSrcC, (UINT32)srcStrideY, (UINT32)srcStrideC,
        (UINT32)width， (UINT32)height， (UINT8)pDstY， (UINT8)pDstC， (UINT32)dstStrideY， (UINT32)dstStrideC，  &_pMetadata, &_roiImage， (float)sharpnessScore);

    env->SetIntField(pMetadata, env->GetFieldID(frameMetaDataClass, "frameId", "I"), _pMetadata.frameId);
    env->SetIntField(pMetadata, env->GetFieldID(frameMetaDataClass, "gyroNumElements", "I"), _pMetadata.gyroNumElements);
    env->SetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "xRotation", "F"), _pMetadata.pGyroData.xRotation);
    env->SetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "yRotation", "F"), _pMetadata.pGyroData.yRotation);
    env->SetFloatField(pGyroData, env->GetFieldID(gyroRotationDataClass, "zRotation", "F"), _pMetadata.pGyroData.zRotation);
    env->SetIntField(pGyroData, env->GetFieldID(gyroRotationDataClass, "timeStamp", "I"), _pMetadata.pGyroData.timeStamp);

    env->SetIntField(roiImage, env->GetFieldID(frameMetaDataClass, "x", "I"), _roiImage.x);
    env->SetIntField(roiImage, env->GetFieldID(frameMetaDataClass, "y", "I"), _roiImage.y);
    env->SetIntField(roiImage, env->GetFieldID(frameMetaDataClass, "dx", "I"), _roiImage.dx);
    env->SetIntField(roiImage, env->GetFieldID(frameMetaDataClass, "dy", "I"), _roiImage.dy);

    env->SetIntField(sharpnessScore, env->GetFieldID(frameMetaDataClass, "frameId", "I"), sharpnessScore);
    return result;
}

CamxResult JNICALL Java_com_android_camera_SwmfnrUtil_nativeMfnrProcess(
        JNIEnv* env, jobject thiz, jlong sessionId,
         jbyteArray pSrcY,
         jbyteArray pSrcC,
         jint numImages,
         jint srcStrideY,
         jint srcStrideC,
         jint srcWidth,
         jint srcHeight,
         jbyte pDstY,
         jbyte pDstC,
         jint dstStrideY,
         jint dstStrideC,
         const qrcpdefs::RoiWindow * const roiWindowPerImage,
         jfloat sharpnessScoreArray,
         qrcpdefs::RoiWindow &outputRoi,
         jfloat blendConfidence,
         jint& nBlendedFrames){
    UINT8 *_pSrcY = (UINT8*)env->GetByteArrayElements(pSrcY, NULL);
    UINT8 *_pSrcC = (UINT8*)env->GetByteArrayElements(pSrcC, NULL);
    jclass roiWindowClass = env->FindClass("com/android/camera/SwmfnrUtil.RoiWindow");
    RoiWindow _roiWindowPerImage;
    _roiWindowPerImage.x = env->GetIntField(roiWindowPerImage, env->GetFieldID(roiWindowClass, "x", "I"));
    _roiWindowPerImage.y = env->GetIntField(roiWindowPerImage, env->GetFieldID(roiWindowClass, "y", "I"));
    _roiWindowPerImage.dx = env->GetIntField(roiWindowPerImage, env->GetFieldID(roiWindowClass, "dx", "I"));
    _roiWindowPerImage.dy = env->GetIntField(roiWindowPerImage, env->GetFieldID(roiWindowClass, "dy", "I"));
    RoiWindow _outputRoi;
    _outputRoi.x = env->GetIntField(outputRoi, env->GetFieldID(roiWindowClass, "x", "I"));
    _outputRoi.y = env->GetIntField(outputRoi, env->GetFieldID(roiWindowClass, "y", "I"));
    _outputRoi.dx = env->GetIntField(outputRoi, env->GetFieldID(roiWindowClass, "dx", "I"));
    _outputRoi.dy = env->GetIntField(outputRoi, env->GetFieldID(roiWindowClass, "dy", "I"));
    MfnrProcess((UINTPTR_T)sessionId, (UINT8)pSrcY, (UINT8)pSrcC, (UINT32)numImages, (UINT32)srcStrideY, (UINT32)srcStrideC, (UINT32)srcWidth, (UINT32)srcHeight, (UINT8)pDstY， (UINT8)pDstC，
        (UINT32)dstStrideY， (UINT32)dstStrideC，  &roiWindowPerImage, sharpnessScoreArray, outputRoi， (float)blendConfidence, (UINT32)nBlendedFrames);
    env->SetIntField(outputRoi, env->GetFieldID(frameMetaDataClass, "x", "I"), _outputRoi.x);
    env->SetIntField(outputRoi, env->GetFieldID(frameMetaDataClass, "y", "I"), _outputRoi.y);
    env->SetIntField(outputRoi, env->GetFieldID(frameMetaDataClass, "dx", "I"), _outputRoi.dx);
    env->SetIntField(outputRoi, env->GetFieldID(frameMetaDataClass, "dy", "I"), _outputRoi.dy);
    env->ReleaseByteArrayElements(pSrcY, (jbyte*)_pSrcY, JNI_ABORT);
    env->ReleaseByteArrayElements(pSrcC, (jbyte*)_pSrcC, JNI_ABORT);
    return result;
}
*/
