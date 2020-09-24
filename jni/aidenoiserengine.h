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

#ifndef __AIDENOISEENGINE_H__
#define __AIDENOISEENGINE_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

#define ARRAY_LEN(arr)                  (sizeof(arr) / sizeof(arr[0]))
#define AIDE_STATIC_ASSERT(COND, MSG)   typedef char static_assertion_##MSG[(COND)?1:-1]

// AIDE is shorthand for AI Denoiser Engine

// AI Denoiser Engine Handle
typedef void * AIDE_Handle;

// Various error codes
typedef enum {
    AIDE_SUCCESS = 0,
    AIDE_PROCESS_FRAME_ABORTED,
    AIDE_ERROR_NOT_SUPPORTED,
    AIDE_ERROR_INVALID_PARAM,
    AIDE_ERROR_MEMORY,
    AIDE_ERROR_GENERAL,
    AIDE_ERROR_NSP_FATAL,
    AIDE_ERROR_NSP_RECOVERABLE,
    AIDE_ERROR_INVALID_STATE,
    AIDE_ERRORCODE_MAX,
} AIDE_ErroCode;

const char *AIDenoiserEngine_GetErrCodeStr(unsigned int code)
{
    static const char *strs[] = { "AIDE_SUCCESS",
                                  "AIDE_PROCESS_FRAME_ABORTED",
                                  "AIDE_ERROR_NOT_SUPPORTED",
                                  "AIDE_ERROR_INVALID_PARAM",
                                  "AIDE_ERROR_MEMORY",
                                  "AIDE_ERROR_GENERAL",
                                  "AIDE_ERROR_NSP_FATAL",
                                  "AIDE_ERROR_NSP_RECOVERABLE",
                                  "AIDE_ERROR_INVALID_STATE" };
    AIDE_STATIC_ASSERT(ARRAY_LEN(strs) == AIDE_ERRORCODE_MAX, error_code_unmatch);
    if (code < AIDE_ERRORCODE_MAX) {
        return strs[code];
    }
    return "UNKNOWN_ERR_CODE";
}

// Frame dimension structure
typedef struct {
    uint32_t   width;
    uint32_t   height;
    uint32_t   stride;
} AIDE_FrameDim;

// Properties to alter the behavior of the engine
// Use optionally if behavior other than default is desired
typedef enum {
    AIDE_PROPERTY_NSP_ERR_RECOVERY,   // Datatype: UINT32* – 0: Auto recover from
                                      //                        recoverable error
                                      //                     1: Do not recover
                                      //                        report error immediately
    AIDE_PROPERTY_MAX
} AIDE_PropId;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} AIDE_ROI;

// Structure to hold arguments to the ProcessFrame call
typedef struct {
    uint8_t            *pInputLuma;       // Image input luma plane pointer
    uint8_t            *pInputChroma;     // Image input chroma plane pointer
    uint8_t            *pOutputLuma;      // Image output luma plane pointer
    uint8_t            *pOutputChroma;    // Image output chroma plane pointer

    uint64_t            expTimeInNs;      // Exposure Time in nanoseconds – range: 1/100 – 1s
    uint32_t            iso;              // ISO = (real_gain * 100) / iso_100_gain
    float               denoiseStrength;  // Denoising strength: 0.0-1.0 (default = 0.5)
    uint32_t            rGain;            // R gain from 3A
    uint32_t            bGain;            // B gain from 3A
    uint32_t            gGain;            // G gain from 3A
    uint32_t            reservedData[20]; // Placeholder for future parameters

    AIDE_ROI            roi;              // Crop roi data
} AIDE_ProcessFrameArgs;

//////////////////////////////////////////////////////////////////////
// Create instance of engine
// Return value: AIDE_SUCCESS for success
//               check AIDE_ErrCode enum for possible errors
//////////////////////////////////////////////////////////////////////
unsigned int AIDenoiserEngine_Create(
    AIDE_FrameDim  *pInputFrameDim,   // Frame resolution of input
                                      // Only supported format is NV12
    AIDE_FrameDim  *pOutputFrameDim,  // Frame resolution of output (needs to be same as input)
                                      // Only supported format is NV12
    AIDE_Handle    *pHandle           // Output handle of the instance created
);

//////////////////////////////////////////////////////////////////////
// Set engine properties to alter the behavior of the engine
// Use optionally if behavior other than default is desired
// Return value: AIDE_SUCCESS for success
//               check AIDE_ErrCode enum for possible errors
//////////////////////////////////////////////////////////////////////
unsigned int AIDenoiserEngine_SetProperty(
    AIDE_Handle   handle,           // Handle to the engine instance
    AIDE_PropId   propId,           // Property ID, see AIDE_PropId definition
    void         *pData             // Pointer to data specific to each property
);

//////////////////////////////////////////////////////////////////////
// Synchronously process a frame
// Return value: AIDE_SUCCESS for success
//               check AIDE_ErrCode enum for possible errors
//////////////////////////////////////////////////////////////////////
unsigned int AIDenoiserEngine_ProcessFrame(
    AIDE_Handle            handle,           // Handle to the engine instance
    AIDE_ProcessFrameArgs *pArgs,            // Pointer to the structure holding arguments to the function
    uint32_t              *errorRecovered    // Output flag: set if an auto NSP error recovery
                                             //              has occurred
);

//////////////////////////////////////////////////////////////////////
// Abort a previous ProcessFrame request
// The original synchronous call to the ProcessFrame will terminate as
// soon as it can and returns AIDE_PROCESS_FRAME_ABORTED
// This API call will return immediately.
// Return value: AIDE_SUCCESS for success
//               check AIDE_ErrCode enum for possible errors
//////////////////////////////////////////////////////////////////////
unsigned int AIDenoiserEngine_Abort(
    AIDE_Handle            handle           // Handle to the engine instance
);

//////////////////////////////////////////////////////////////////////
// Destroy the engine instance
// If there is on-going ProcessFrame request, an abort will be
// implicitly called to terminate it early before destroy will be
// completed.
// Return value: AIDE_SUCCESS for success
//               check AIDE_ErrCode enum for possible errors
//////////////////////////////////////////////////////////////////////
unsigned int AIDenoiserEngine_Destroy(
    AIDE_Handle            handle           // Handle to the engine instance
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // #ifndef __AIDENOISEENGINE_H__
