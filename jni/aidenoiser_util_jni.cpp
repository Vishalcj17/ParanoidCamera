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
#include "aidenoiserengine.h"

#ifdef __ANDROID__
#include "android/log.h"
#define printf(...) __android_log_print( ANDROID_LOG_ERROR, "aidejni", __VA_ARGS__ )
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineCreate(
        JNIEnv* env, jobject thiz, jintArray pInputFrameDim, jintArray pOutputFrameDim);
JNIEXPORT jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineProcessFrame(
        JNIEnv *env, jobject thiz, jbyteArray input, jbyteArray output,
        jlong expTimeInNs, jint iso, jfloat denoiseStrength, jint rGain, jint bGain, jint gGain, jintArray roi);
JNIEXPORT jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineAbort(
        JNIEnv* env, jobject thiz);
JNIEXPORT jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineDestroy(
        JNIEnv* env, jobject thiz);
#ifdef __cplusplus
}
#endif

AIDE_Handle handle;
uint32_t width;
uint32_t height;
uint32_t stride;

jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineCreate(
        JNIEnv* env, jobject thiz, jintArray pInputFrameDim, jintArray pOutputFrameDim)
{
    jint *inputFrameDim = env->GetIntArrayElements(pInputFrameDim, 0);
    AIDE_FrameDim _pInputFrameDim;
    _pInputFrameDim.width = (uint32_t)inputFrameDim[0];
    _pInputFrameDim.height = (uint32_t)inputFrameDim[1];
    _pInputFrameDim.stride = (uint32_t)inputFrameDim[2];
    width = (uint32_t)inputFrameDim[0];
    height = (uint32_t)inputFrameDim[1];
    stride = (uint32_t)inputFrameDim[2];
    printf("aide create,width=%d,height=%d,stride=%d", (uint32_t)inputFrameDim[0],(uint32_t)inputFrameDim[1],(uint32_t)inputFrameDim[2]);

    jint *outputFrameDim = env->GetIntArrayElements(pOutputFrameDim, 0);
    AIDE_FrameDim _outputFrameDim;
    _outputFrameDim.width = (uint32_t)outputFrameDim[0];
    _outputFrameDim.height = (uint32_t)outputFrameDim[1];
    _outputFrameDim.stride = (uint32_t)outputFrameDim[2];
    int createResult = AIDenoiserEngine_Create(&_pInputFrameDim, &_outputFrameDim, &handle);
    printf("aide create,createResult=%d,handle=%d", createResult, handle);

    //set out put
    uint32_t* pOutputFrame = new uint32_t[3];
    pOutputFrame[0] = _outputFrameDim.width;
    pOutputFrame[1] = _outputFrameDim.height;
    pOutputFrame[2] = _outputFrameDim.stride;
    printf("aide create,width=%d,height=%d,stride=%d", _outputFrameDim.width,_outputFrameDim.height,_outputFrameDim.stride);

    env->SetIntArrayRegion(pOutputFrameDim, 0, 3, (jint *)pOutputFrame);
    env->ReleaseIntArrayElements(pInputFrameDim, inputFrameDim, 0);
    env->ReleaseIntArrayElements(pOutputFrameDim, outputFrameDim, 0);
    delete pOutputFrame;
    return createResult;
}
void WriteData(FILE *fp, unsigned char *pStart, int width, int height, int stride)
{
    for (int i = 0; i < height; i++)
    {
        fwrite(pStart, stride, 1, fp);
        pStart += stride;
    }
}
jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineProcessFrame(
        JNIEnv *env, jobject thiz, jbyteArray input, jbyteArray output,
        jlong expTimeInNs, jint iso, jfloat denoiseStrength, jint rGain, jint bGain, jint gGain, jintArray roi)
{
    AIDE_ProcessFrameArgs args;
    args.rGain = (uint32_t)rGain;
    args.gGain = (uint32_t)gGain;
    args.bGain = (uint32_t)bGain;
    jint *croi = env->GetIntArrayElements(roi, NULL);
    args.roi.x = (uint32_t)croi[0];
    args.roi.y = (uint32_t)croi[1];
    args.roi.width = (uint32_t)croi[2];
    args.roi.height = (uint32_t)croi[3];
    args.denoiseStrength = (float)denoiseStrength;
    args.iso = (uint32_t)iso;
    args.expTimeInNs = (uint64_t)expTimeInNs;

    jbyte* inputArray = env->GetByteArrayElements(input, NULL);
    uint8_t *cinput = (uint8_t*)inputArray;
    uint8_t* cinputY = (uint8_t*)&(cinput[0]);
    uint8_t* cinputVU = (uint8_t*)&(cinput[stride*height]);

    jbyte* outputArray = env->GetByteArrayElements(output, NULL);
    uint8_t *coutput = (uint8_t*)outputArray;
    uint8_t* coutputY = (uint8_t*)&(coutput[0]);
    uint8_t* coutputVU = (uint8_t*)&(coutput[stride*height]);

    args.pInputLuma = cinputY;
    args.pInputChroma = cinputVU;
    args.pOutputLuma = coutputY;
    args.pOutputChroma = coutputVU;
    int result = AIDenoiserEngine_ProcessFrame(handle, &args, NULL);
    FILE *inputFile = fopen("/data/data/org.codeaurora.snapcam/files/aideoutput.yuv", "wb+");
    if ((inputFile != NULL)){
        WriteData(inputFile, coutputY, width, height, stride);
        WriteData(inputFile, coutputVU, width, height/2, stride);
        fclose(inputFile);
    } else {
        printf( "aideoutput is NULL");
    }
    //set out put
    env->ReleaseByteArrayElements(input, inputArray, JNI_ABORT);
    env->ReleaseByteArrayElements(output, outputArray, JNI_ABORT);
    env->ReleaseIntArrayElements(roi, croi, 0);
    return result;
}

jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineAbort(
        JNIEnv* env, jobject thiz)
{
    return AIDenoiserEngine_Abort(handle);
}

jint JNICALL Java_com_android_camera_aide_AideUtil_nativeAIDenoiserEngineDestroy(
        JNIEnv* env, jobject thiz)
{
    return AIDenoiserEngine_Destroy(handle);
}
