#ifndef CAMX_MFNR_WRAPPER_H
#define CAMX_MFNR_WRAPPER_H

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


//==============================================================================
// Included modules
//==============================================================================
#include <camxdefs.h>
#include <vector>
#include <thread>

#ifndef _MSC_VER
#include <dlfcn.h>
#endif

//==============================================================================
// MACROS
//==============================================================================
#ifdef _MSC_VER
#define CP_DLL_PUBLIC __declspec(dllexport)
#else
#define CP_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#include <android/log.h>
#define LOG_TAG "MfnrJniwrapper"
#define printf(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__ )


extern "C"
{

namespace qrcpdefs
{
    //------------------------------------------------------------------------------
   /// @brief
   ///   ImageRegistrationDescriptorSelector
   //------------------------------------------------------------------------------
   enum ImageRegistrationDescriptorSelector
   {
      BLOCK_DESCRIPTOR = 0,
      INVARIANT_DESCRIPTOR = 1,
      IMAGE_REGISTRATION_DESCRIPTOR_MAX = 2,
   };

   //------------------------------------------------------------------------------
   /// @brief
   ///   ImageRegistrationAccuracyMode
   //------------------------------------------------------------------------------
   enum ImageRegistrationAccuracyMode
   {
      FAST = 0,
      PRECISE = 1,
      IMAGE_REGISTRATION_MODE_MAX = 2,
   };

   //------------------------------------------------------------------------------
   /// @brief
   ///   RoiWindow
   //------------------------------------------------------------------------------
   struct RoiWindow
   {
      UINT32 x;
      UINT32 y;
      UINT32 dx;
      UINT32 dy;
   };

   //------------------------------------------------------------------------------
   /// @brief
   ///   GyroRotationData
   //------------------------------------------------------------------------------
   struct GyroRotationData
   {
      float xRotation; //in rad/s
      float yRotation;
      float zRotation;
      UINT64 timeStamp;
   };

   //------------------------------------------------------------------------------
   /// @brief
   ///   FrameMetaData
   //------------------------------------------------------------------------------
   struct FrameMetaData
   {
      UINT32 frameId;
      UINT32 gyroNumElements;
      GyroRotationData * pGyroData;
   };


   // MFNR Lib Function Pointers
   typedef struct {
       void *ptr;

       UINTPTR_T(*Mfnry303)(void);
       CamxResult(*Mfnry101)(UINTPTR_T, UINT32, UINT32);
       CamxResult(*Mfnry202)(UINTPTR_T);
       CamxResult(*Mfnry404)(UINTPTR_T);
       CamxResult(*MfnrAbortFrame)(UINTPTR_T);
       bool(*MfnrIsFrameAborted)(UINTPTR_T);
       CamxResult(*MfnrResetFrameAbortFlag)(UINTPTR_T);
       CamxResult(*MfnrSetFrameRunningState)(UINTPTR_T, bool);

       CamxResult(*Mfnruf6H4LqsSk)();

       // Configure
       CamxResult(*Mfnrjmf9K6K5NQ)(UINTPTR_T pPrivate,
           bool enableGyroRefinement,
           qrcpdefs::ImageRegistrationDescriptorSelector imageregDesc,
           qrcpdefs::ImageRegistrationAccuracyMode imageregMode,
           float deghostingStrength,
           float localMotionSmoothingStrength,
           float dtfYStrength,
           float dtfChromaStrength,
           float sharpnessStrength,
           float spatioTemporalDenoiseBalanceStrength,
           float sharpnessScoreThreshold);

       // Register
       CamxResult(*MfnriKtXNRGAUM)(
           UINTPTR_T sessionId,
           const UINT8* pRefY,
           const UINT8* pRefC,
           UINT32 refStrideY,
           UINT32 refStrideC,
           const UINT8* pSrcY,
           const UINT8* pSrcC,
           UINT32 srcStrideY,
           UINT32 srcStrideC,
           UINT32 width,
           UINT32 height,
           UINT8* pDstY,
           UINT8* pDstC,
           UINT32 dstStrideY,
           UINT32 dstStrideC,
           const qrcpdefs::FrameMetaData * const pMetadata,
           qrcpdefs::RoiWindow &roiImage,
           float &sharpnessScore);

       // Process
       CamxResult(*MfnryPGmb6U2bt)(
           UINTPTR_T sessionId,
           const uint8_t* const pSrcY[],
           const uint8_t* const pSrcC[],
           uint32_t numImages,
           uint32_t srcStrideY,
           uint32_t srcStrideC,
           uint32_t srcWidth,
           uint32_t srcHeight,
           uint8_t* pDstY,
           uint8_t* pDstC,
           uint32_t dstStrideY,
           uint32_t dstStrideC,
           const qrcpdefs::RoiWindow * const roiWindowPerImage,
           const float * const sharpnessScoreArray,
           qrcpdefs::RoiWindow &outputRoi,
           float& blendConfidence,
           uint32_t& nBlendedFrames,
           float imageGain,
           bool isAIDEenabled);

       uint32_t(*EstimateSharpnessScoreAndSortAnchorToFirst)(
           UINTPTR_T pPrivate,
           const UINT8* const pSrcY[],
           const UINT8* const pSrcC[],
           UINT32 srcStrideY,
           UINT32 srcStrideC,
           UINT32 srcWidth,
           UINT32 srcHeight,
           UINT32 numImagesIn,
           UINT8* pSrcYSorted[],
           UINT8* pSrcCSorted[]);
   } mfnrLib;


   enum NcoreLibrary
   {
       FOUR_CORES = 0,
       EIGHT_CORES = 1
   };


}

//==============================================================================
// EXTERNAL FUNCTIONS
//==============================================================================

CP_DLL_PUBLIC CamxResult Mfnruf6H4LqsSk( void );

CP_DLL_PUBLIC CamxResult
Mfnrjmf9K6K5NQ( UINTPTR_T sessionId,
   bool enableGyroRefinement,
   qrcpdefs::ImageRegistrationDescriptorSelector imageregDesc,
   qrcpdefs::ImageRegistrationAccuracyMode imageregMode,
   float deghostingStrength,
   float localMotionSmoothingStrength,
   float dtfYStrength,
   float dtfChromaStrength,
   float sharpnessStrength,
   float spatioTemporalDenoiseBalanceStrength,
   float sharpnessScoreThreshold);

CP_DLL_PUBLIC CamxResult
MfnriKtXNRGAUM( UINTPTR_T sessionId,
   const uint8_t* pRefY,
   const uint8_t* pRefC,
   uint32_t refStrideY,
   uint32_t refStrideC,
   const uint8_t* pSrcY,
   const uint8_t* pSrcC,
   uint32_t srcStrideY,
   uint32_t srcStrideC,
   uint32_t width,
   uint32_t height,
   uint8_t* pDstY,
   uint8_t* pDstC,
   uint32_t dstStrideY,
   uint32_t dstStrideC,
   const qrcpdefs::FrameMetaData * const pMetadata,
   qrcpdefs::RoiWindow &roiImage,
   float &sharpnessScore );

CP_DLL_PUBLIC CamxResult
MfnryPGmb6U2bt( UINTPTR_T sessionId,
   const uint8_t* const pSrcY[],
   const uint8_t* const pSrcC[],
   uint32_t numImages,
   uint32_t srcStrideY,
   uint32_t srcStrideC,
   uint32_t srcWidth,
   uint32_t srcHeight,
   uint8_t* pDstY,
   uint8_t* pDstC,
   uint32_t dstStrideY,
   uint32_t dstStrideC,
   const qrcpdefs::RoiWindow * const roiWindowPerImage,
   const float * const sharpnessScoreArray,
   qrcpdefs::RoiWindow &outputRoi,
   float& blendConfidence,
   uint32_t& nBlendedFrames,
   float imageGain,
   bool isAIDEenabled);

CP_DLL_PUBLIC CamxResult Mfnry101( UINTPTR_T sessionId,
   UINT32 srcWidth,
   UINT32 srcHeight );

CP_DLL_PUBLIC CamxResult Mfnry202( UINTPTR_T sessionId );

CP_DLL_PUBLIC UINTPTR_T Mfnry303( void );

CP_DLL_PUBLIC CamxResult Mfnry404( UINTPTR_T sessionId );

CP_DLL_PUBLIC CamxResult MfnrAbortFrame(UINTPTR_T sessionId);

CP_DLL_PUBLIC bool MfnrIsFrameAborted(UINTPTR_T sessionId);

CP_DLL_PUBLIC CamxResult MfnrResetFrameAbortFlag(UINTPTR_T pPrivate);

CP_DLL_PUBLIC CamxResult MfnrSetFrameRunningState(UINTPTR_T pPrivate, bool state);

CP_DLL_PUBLIC uint32_t EstimateSharpnessScoreAndSortAnchorToFirst(
    UINTPTR_T pPrivate,
    const UINT8* const pSrcY[],
    const UINT8* const pSrcC[],
    UINT32 srcStrideY,
    UINT32 srcStrideC,
    UINT32 srcWidth,
    UINT32 srcHeight,
    UINT32 numImagesIn,
    UINT8* pSrcYSorted[],
    UINT8* pSrcCSorted[]);

//==============================================================================
// DECLARATIONS
//==============================================================================
namespace qrcp
{
   namespace internal
   {
      //==============================================================================
      // UTILITY FUNCTIONS
      //==============================================================================
      static inline bool
         IsImgDimensionsValid( UINT32 width, UINT32 height, UINT32 stride, UINT32 channels )
      {
         if (width == 0 || width > static_cast<UINT32>(std::numeric_limits<int32_t>::max()))
            return false;

         if (height == 0 || height > static_cast<UINT32>(std::numeric_limits<int32_t>::max()))
            return false;

         if (stride < width * channels || stride > static_cast<UINT32>(std::numeric_limits<int32_t>::max()))
            return false;

         return true;
      }
   } // namespace internal



   //------------------------------------------------------------------------------
   /// @brief
   ///   Tunable parameters for MFNR
   //------------------------------------------------------------------------------
   struct MfnrTunableParams
   {
      ///   Enable Gyro for image registration decision
      ///   True/False , Default Value : True
      bool enableGyroRefinement;

      ///   Image Registration Descriptor Selector
      ///   BLOCK_DESCRIPTOR/INVARIANT_DESCRIPTOR , Default Value : INVARIANT_DESCRIPTOR
      qrcpdefs::ImageRegistrationDescriptorSelector imageRegDesc;

      ///   Image Registration Accuracy Mode
      ///   FAST/PRECISE , Default Value : Fast
      qrcpdefs::ImageRegistrationAccuracyMode imageRegMode;

      ///   Controls amount of deghosting during temporal blending. Range: 0 ~ 1.
      ///   Larger value means preference for deghosting/less blending. Default Value : 0.5.
      float deghostingStrength;

      ///   Controls amount of spatial smoothing on local motion areas. Range: 0 ~ 1.
      ///   Larger value means more smoother output in  local motion areas. Default value : 1.0
      float localMotionSmoothingStrength;

      ///   Controls amount of dtf Y spatial smoothing Range: 0 ~ 0.5.
      ///   Larger value means more smoother output. Default value : 0.0 (disabled)
      float dtfSpatialYStrength;

      ///   Controls amount of dtf chroma spatial smoothing Range: 0 ~ 0.5.
      ///   Larger value means more smoother output. Default value : 0.0 (disabled)
      float dtfSpatialChromaStrength;
      
      ///   Controls amount of dtf chroma spatial smoothing Range: 0 ~ 1.
      ///   Larger value means more sharper output. Default value : 0.5
      float sharpnessStrength;

      ///   Controls noise balancing between blended and non-blended regions to makesure uniformity Range: 0 ~ 1
      ///   Higher the value means more balancing leading to less denoising and better uniformity. Default value : 0.5
      float spatioTemporalDenoiseBalanceStrength;

      /// Controls frame rejections based on sharpest frame in the batch. Rejects frames beyond this threshold for MFNR processing
      /// Higher the value, narrower the threshold. Default: 0.85. range 0.0 - 1.0. 
      float sharpnessScoreThreshold;
   };

   //==============================================================================
   // API FUNCTIONS
   //==============================================================================
 


   //------------------------------------------------------------------------------
   /// @brief
   ///   Allows users to override default parameters
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @param params
   ///   Tunable parameters
   ///
   /// @return
   ///   True if successful; false otherwise
   //------------------------------------------------------------------------------
   static inline CamxResult
      MfnrConfigure( UINTPTR_T sessionId,
         const MfnrTunableParams& params, qrcpdefs::mfnrLib* fPtr )
   {
       fPtr->Mfnruf6H4LqsSk();

      // Check range of image registration descriptor
      if (params.imageRegDesc >= qrcpdefs::IMAGE_REGISTRATION_DESCRIPTOR_MAX)
      {
         return CamxResultEInvalidArg;
      }

      // Check range of image registration mode
      if (params.imageRegMode >= qrcpdefs::IMAGE_REGISTRATION_MODE_MAX)
      {
         return CamxResultEInvalidArg;
      }

      // Check range of deghostingStrength
      if (params.deghostingStrength < 0.0f || params.deghostingStrength > 1.0f)
      {
         return CamxResultEInvalidArg;
      }

      // Check range of localMotionSmoothingStrength
      if (params.localMotionSmoothingStrength < 0.0f || params.localMotionSmoothingStrength > 1.0f)
      {
         return CamxResultEInvalidArg;
      }

      // Check range of dtfSpatialYStrength
      if (params.dtfSpatialYStrength < 0.0f || params.dtfSpatialYStrength > 0.5f)
      {
         return CamxResultEInvalidArg;
      }

      // Check range of dtfSpatialChromaStrength
      if (params.dtfSpatialChromaStrength < 0.0f || params.dtfSpatialChromaStrength > 0.5f)
      {
         return CamxResultEInvalidArg;
      }

      return fPtr->Mfnrjmf9K6K5NQ( sessionId,
         params.enableGyroRefinement,
         params.imageRegDesc,
         params.imageRegMode,
         params.deghostingStrength,
         params.localMotionSmoothingStrength,
         params.dtfSpatialYStrength,
         params.dtfSpatialChromaStrength,
         params.sharpnessStrength,
         params.spatioTemporalDenoiseBalanceStrength,
         params.sharpnessScoreThreshold);
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Register NV21 image with the reference image
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @param pRefY
   ///   Luma channel of reference image
   ///
   /// @param pRefC
   ///   Chroma channel of reference image
   ///
   /// @param refStrideY
   ///   Stride in bytes for reference images provided in pRefY
   ///
   /// @param refStrideC
   ///   Stride in bytes for reference images provided in pRefC
   ///
   /// @param pSrcY
   ///   Luma channel of input image
   ///
   /// @param pSrcC
   ///   Chroma channel of input image
   ///
   /// @param srcStrideY
   ///   Stride in bytes for input images provided in pSrcY
   ///
   /// @param srcStrideC
   ///   Stride in bytes for input images provided in pSrcC
   ///
   /// @param width
   ///   Width of input images provided in pSrcY
   ///
   /// @param height
   ///   Height of input images provided in pSrcY
   ///
   /// @param pDstY
   ///   Luma channel of output image
   ///
   /// @param pDstC
   ///   Chroma channel of output image
   ///
   /// @param dstStrideY
   ///   Stride in bytes of pDstY
   ///
   /// @param dstStrideC
   ///   Stride in bytes of pDstC
   ///
   /// @param metadata
   ///   FrameMetaData
   ///
   /// @return
   ///   True if successful; false otherwise
   //------------------------------------------------------------------------------
   static inline CamxResult
      MfnrRegister( UINTPTR_T sessionId,
         const UINT8* pRefY,
         const UINT8* pRefC,
         UINT32 refStrideY,
         UINT32 refStrideC,
         const UINT8* pSrcY,
         const UINT8* pSrcC,
         UINT32 srcStrideY,
         UINT32 srcStrideC,
         UINT32 width,
         UINT32 height,
         UINT8* pDstY,
         UINT8* pDstC,
         UINT32 dstStrideY,
         UINT32 dstStrideC,
         const qrcpdefs::FrameMetaData * const pMetadata,
         qrcpdefs::RoiWindow &roiImage,
         float &sharpnessScore,
         qrcpdefs::mfnrLib* fPtr)
   {

       printf("MfnrRegisterAndProcess_ : srcStrideY = %d, srcStrideVU = %d, Width x Height = (%d x %d)", srcStrideY, srcStrideC, width, height);
       printf(" MfnrRegisterAndProcess_: dstStrideY = %d, dstStrideVU = %d", dstStrideY, dstStrideC);

      // Validate pSrcY and srcC
      // All pointers should be non-NULL
      if (pSrcY == NULL || pSrcC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      // Validate srcWidth, srcHeight, srcStrideY, and srcStrideC
      // Image size should be at least VGA
      // srcWidth and srcHeight should be multiple of 2
      if (std::max( width, height ) < 640 ||
         std::min( width, height ) < 480 ||
         (width % 2) != 0 || (height % 2) != 0 ||
         !internal::IsImgDimensionsValid( width, height, srcStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( width / 2, height / 2, srcStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      // Validate pSrcY and pSrcC
      // Should not cause memory access violation
      volatile UINT8 tmp;

      tmp = pSrcY[0];
      tmp = pSrcY[srcStrideY * height - 1];
      tmp = pSrcC[0];
      tmp = pSrcC[srcStrideC * height / 2 - 1];


      // Validate pDstY and pDstC
      // All pointers should be non-NULL
      if (pDstY == NULL || pDstC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      // Validate dstStrideY and dstStrideC
      if (!internal::IsImgDimensionsValid( width, height, dstStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( width / 2, height / 2, dstStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      // Validate pDstY and dstVU
      // Should not cause memory access violation
      pDstY[0] = 0;
      pDstY[dstStrideY * height - 1] = 0;
      pDstC[0] = 0;
      pDstC[dstStrideC * height / 2 - 1] = 0;

      //Validate src and dst are not same
      if (pSrcY == pDstY || pSrcC == pDstC)
      {
         return CamxResultEExists;
      }

      // Call into the library
      return fPtr->MfnriKtXNRGAUM( sessionId,
         pRefY,
         pRefC,
         refStrideY,
         refStrideC,
         pSrcY,
         pSrcC,
         srcStrideY,
         srcStrideC,
         width,
         height,
         pDstY,
         pDstC,
         dstStrideY,
         dstStrideC,
         pMetadata,
         roiImage,
         sharpnessScore );
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Processing *registered* NV21 image(s) with MFNR
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @param pSrcY
   ///   *Registered* Luma channel of input image
   ///
   /// @param pSrcC
   ///   *Registered* Chroma channel of input image
   ///
   /// @param numImages
   ///   Number of images
   ///
   /// @param srcStrideY
   ///   Stride in bytes for input images provided in pSrcY
   ///
   /// @param srcStrideC
   ///   Stride in bytes for input images provided in pSrcC
   ///
   /// @param srcWidth
   ///   Width of input images provided in pSrcY
   ///
   /// @param srcHeight
   ///   Height of input images provided in pSrcY
   ///
   /// @param pDstY
   ///   Luma channel of output image
   ///
   /// @param pDstC
   ///   Chroma channel of output image
   ///
   /// @param dstStrideY
   ///   Stride in bytes of pDstY
   ///
   /// @param dstStrideC
   ///   Stride in bytes of dstStrideC
   ///
   /// @param roiImageArray
   ///   Roi Windows for the input images
   ///
   /// @param sharpnessScoreArray
   ///   Sharpness score of input images
   ///
   /// @param outputRoi
   ///   output roi window
   ///
   /// @return
   ///   CamxResult
   //------------------------------------------------------------------------------
   static inline CamxResult
      MfnrProcess( UINTPTR_T sessionId,
         const UINT8* const pSrcY[],
         const UINT8* const pSrcC[],
         UINT32 numImages,
         UINT32 srcStrideY,
         UINT32 srcStrideC,
         UINT32 srcWidth,
         UINT32 srcHeight,
         UINT8* pDstY,
         UINT8* pDstC,
         UINT32 dstStrideY,
         UINT32 dstStrideC,
         const qrcpdefs::RoiWindow * const roiWindowPerImage,
         const float * const sharpnessScoreArray,
         qrcpdefs::RoiWindow &outputRoi,
         float&  blendConfidence,
         UINT32& nBlendedFrames,
         qrcpdefs::mfnrLib* fPtr,
         float imageGain,
         bool isAIDEenabled)
   {
      // Validate number of images
      if (numImages < 1)
      {
         return CamxResultEInvalidArg;
      }

      // Validate pSrcY and pSrcC
      // All pointers should be non-NULL
      if (pSrcY == NULL || pSrcC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      for (UINT32 i = 0; i < numImages; ++i)
      {
         if (pSrcY[i] == NULL || pSrcC[i] == NULL)
         {
            return CamxResultEInvalidPointer;
         }
      }

      // Validate srcWidth, srcHeight, srcStrideY, and srcStrideC
      // Image size should be at least VGA
      // srcWidth and srcHeight should be multiple of 2
      if (std::max( srcWidth, srcHeight ) < 640 ||
         std::min( srcWidth, srcHeight ) < 480 ||
         (srcWidth % 2) != 0 || (srcHeight % 2) != 0 ||
         !internal::IsImgDimensionsValid( srcWidth, srcHeight, srcStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( srcWidth / 2, srcHeight / 2, srcStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      // Validate pSrcY and pSrcC
      // Should not cause memory access violation

printf("MfnrProcess,  numImages == %d,srcStrideY=%d, srcHeight=%d",numImages, srcStrideY, srcHeight);
      {
         volatile UINT8 tmp;
         const UINT8 *pY, *pC;
         for (UINT32 i = 0; i < numImages; ++i)
         {
            pY = pSrcY[i];
            pC = pSrcC[i];
            tmp = pY[0];
            printf("i1 == %d,tmp=%d",i, tmp);

            tmp = pY[srcStrideY * srcHeight - 1];
            printf("i2 == %d,tmp=%d",i, tmp);

            tmp = pC[0];
            printf("i3 == %d,tmp=%d",i, tmp);

            tmp = pC[srcStrideC * srcHeight / 2 - 1];
            printf("i4 == %d,tmp=%d",i, tmp);

         }
      }

      // Validate pDstY and pDstC
      // All pointers should be non-NULL
      if (pDstY == NULL || pDstC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      // Validate dstStrideY and dstStrideC
      if (!internal::IsImgDimensionsValid( srcWidth, srcHeight, dstStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( srcWidth / 2, srcHeight / 2, dstStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      // Validate pDstY and pDstC
      // Should not cause memory access violation
      pDstY[0] = 0;
      pDstY[dstStrideY * srcHeight - 1] = 0;
      pDstC[0] = 0;
      pDstC[dstStrideC * srcHeight / 2 - 1] = 0;

      //Initialize MFNR
      fPtr->Mfnruf6H4LqsSk();

      // Call into the library
      return fPtr->MfnryPGmb6U2bt( sessionId,
         pSrcY,
         pSrcC,
         numImages,
         srcStrideY,
         srcStrideC,
         srcWidth,
         srcHeight,
         pDstY,
         pDstC,
         dstStrideY,
         dstStrideC,
         roiWindowPerImage,
         sharpnessScoreArray,
         outputRoi,
         blendConfidence,
         nBlendedFrames,
         imageGain,
         isAIDEenabled);
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Processing NV21 image(s) with MFNR
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @param pSrcY
   ///   Luma channel of input image(s)
   ///
   /// @param pSrcC
   ///   Chroma channel of input image(s)
   ///
   /// @param numImages
   ///   Number of images
   ///
   /// @param srcStrideY
   ///   Stride in bytes for input images provided in pSrcY
   ///
   /// @param srcStrideC
   ///   Stride in bytes for input images provided in pSrcC
   ///
   /// @param srcWidth
   ///   Width of input images provided in pSrcY
   ///
   /// @param srcHeight
   ///   Height of input images provided in pSrcY
   ///
   /// @param pDstY
   ///   Luma channel of output image
   ///
   /// @param pDstC
   ///   Chroma channel of output image
   ///
   /// @param dstStrideY
   ///   Stride in bytes of pDstY
   ///
   /// @param dstStrideC
   ///   Stride in bytes of pDstC
   ///
   /// @param pMetadataArray
   ///   Meta data for all frames
   ///
   /// @param outputRoi
   ///   output roi window
   ///
   /// @return
   ///   CamxResult
   //------------------------------------------------------------------------------
   static inline CamxResult
      MfnrRegisterAndProcess( UINTPTR_T sessionId,
         const UINT8* const pSrcY[],
         const UINT8* const pSrcC[],
         UINT32 numImages,
         UINT32 srcStrideY,
         UINT32 srcStrideC,
         UINT32 srcWidth,
         UINT32 srcHeight,
         UINT8* pDstY,
         UINT8* pDstC,
         UINT32 dstStrideY,
         UINT32 dstStrideC,
         const qrcpdefs::FrameMetaData * const pMetadataArray[],
         qrcpdefs::RoiWindow &outputRoi,
         float& blendConfidence,
         UINT32& nBlendedFrames,
         qrcpdefs::mfnrLib* fPtr,
         float imageGain,
         bool isAIDEenabled)
   {

#ifdef ENABLE_INTERMEDIATE_DUMPS_ON_ANDROID

#define MFNR_LIB_DUMP_PATH "/data/vendor/camera/"

       static uint32_t gMfnrServiceCallsCnt = 0;

       std::string dumpFilePrefix = MFNR_LIB_DUMP_PATH "MFNR_" + std::to_string(gMfnrServiceCallsCnt) + "_";

       std::string regInfName, regOutWarpedfName;
       FILE *regIn_fp = NULL, *regOut_fp = NULL;

#endif

       // Print input params
       printf("\n MFNR SW LIB VERSION 1.1");
       printf("\n MfnrRegisterAndProcess : srcStrideY = %d, srcStrideVU = %d, Width x Height = (%d x %d)", srcStrideY, srcStrideC, srcWidth, srcHeight);
       printf("\n MfnrRegisterAndProcess: dstStrideY = %d, dstStrideVU = %d", dstStrideY, dstStrideC);

      //Validate number of images
      if (numImages < 1)
      {
         return CamxResultEInvalidArg;
      }

      // Validate pSrcY and pSrcC
      // All pointers should be non-NULL
      if (pSrcY == NULL || pSrcC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      for (UINT32 i = 0; i < numImages; ++i)
      {
         if (pSrcY[i] == NULL || pSrcC[i] == NULL)
         {
            return CamxResultEInvalidPointer;
         }
      }

      // Validate srcWidth, srcHeight, srcStrideY, and srcStrideC
      // Image size should be at least VGA
      // srcWidth and srcHeight should be multiple of 2
      if (std::max( srcWidth, srcHeight ) < 640 ||
         std::min( srcWidth, srcHeight ) < 480 ||
         (srcWidth % 2) != 0 || (srcHeight % 2) != 0 ||
         !internal::IsImgDimensionsValid( srcWidth, srcHeight, srcStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( srcWidth / 2, srcHeight / 2, srcStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      // Validate pSrcY and pSrcC
      // Should not cause memory access violation
             printf("\n MfnrRegisterAndProcess: numImages = %d",numImages);

      {
         volatile UINT8 tmp;
         const UINT8 *pY, *pC;
         for (UINT32 i = 0; i < numImages; ++i)
         {
            pY = pSrcY[i];
            pC = pSrcC[i];
            tmp = pY[0];
            tmp = pY[srcStrideY * srcHeight - 1];
            printf("py == %d,tmp=%d",i, tmp);
            tmp = pC[0];
            tmp = pC[srcStrideC * srcHeight / 2 - 1];
            printf("pc == %d,tmp=%d",i, tmp);
         }
      }

      // Validate pDstY and pDstC
      // All pointers should be non-NULL
      if (pDstY == NULL || pDstC == NULL)
      {
         return CamxResultEInvalidPointer;
      }

      // Validate dstStrideY and dstStrideVU
      if (!internal::IsImgDimensionsValid( srcWidth, srcHeight, dstStrideY, 1 ) ||
         !internal::IsImgDimensionsValid( srcWidth / 2, srcHeight / 2, dstStrideC, 2 ))
      {
         return CamxResultEInvalidArg;
      }

      const UINT8 MAX_MFNR_FRAMES = 10; // Max allowable Nframes in MFNR;


      if (numImages > MAX_MFNR_FRAMES)
      {
          printf("\n MfnrRegisterAndProcess: MFNR supports only upto 10 frames...%d frames sent", numImages);
          return CamxResultEInvalidArg;
      }


      // Set Blend Params to best values by default
      blendConfidence = 1.0;
      nBlendedFrames  = numImages == 1 ? 1 : (numImages - 1);



      // Validate pDstY and pDstC
      // Should not cause memory access violation
      pDstY[0] = 0;
      pDstY[dstStrideY * srcHeight - 1] = 0;
      pDstC[0] = 0;
      pDstC[dstStrideC * srcHeight / 2 - 1] = 0;

      // Set MFNR library processing state
      fPtr->MfnrSetFrameRunningState(sessionId, true);


      UINT8* pSrcY_t[MAX_MFNR_FRAMES] = { 0 };
      UINT8* pSrcC_t[MAX_MFNR_FRAMES] = { 0 };

      // Estimate sharpness of all frames. Sort Anchor to be first frame. Reject other frames
      const uint32_t nSharpestFrames = fPtr->EstimateSharpnessScoreAndSortAnchorToFirst(sessionId, pSrcY, pSrcC, srcStrideY, srcStrideC, srcWidth, srcHeight, numImages, pSrcY_t, pSrcC_t);

      std::vector<UINT8*> pWarpedY(nSharpestFrames), pWarpedC(nSharpestFrames);
      std::vector<qrcpdefs::RoiWindow> roiWindowPerImage(nSharpestFrames);
      std::vector<float> sharpnessScore(nSharpestFrames);

      std::vector<std::thread> mfnrThreadPool;

      // Cast to help thread invoker in identifying correct overloaded function
      std::function <void(
          UINTPTR_T, const UINT8*,const UINT8*,UINT32,UINT32,const UINT8*,const UINT8*,UINT32 ,
          UINT32,UINT32,UINT32,UINT8*,UINT8*,UINT32,UINT32,const qrcpdefs::FrameMetaData * const ,
          qrcpdefs::RoiWindow &,float&, qrcpdefs::mfnrLib*)> pMfnrRegister =
          static_cast<CamxResult(*)(UINTPTR_T, const UINT8*, const UINT8*, UINT32, UINT32, const UINT8*, const UINT8*, UINT32,
                                   UINT32, UINT32, UINT32, UINT8*, UINT8*, UINT32, UINT32, const qrcpdefs::FrameMetaData * const,
                                   qrcpdefs::RoiWindow &, float&, qrcpdefs::mfnrLib*)>(qrcp::MfnrRegister);

      CamxResult rc = 0;

      const bool enableDCpass = false;

      uint8_t threadStartIdx = enableDCpass ? 2 : 1;
      const uint8_t nParallelReg = nSharpestFrames - threadStartIdx;

#ifdef ENABLE_DUMPING
      int64_t trStart, trStop, tpStop;

      trStart = cv::getTickCount();
#endif

      // Run Data Collection pass of registration
      if (enableDCpass)
      {
          pWarpedY[1] = new UINT8[srcHeight * dstStrideY];
          pWarpedC[1] = new UINT8[srcHeight * dstStrideY / 2];

          MfnrRegister(
              sessionId,
              pSrcY_t[0],
              pSrcC_t[0],
              srcStrideY,
              srcStrideC,
              pSrcY_t[1],
              pSrcC_t[1],
              srcStrideY,
              srcStrideC,
              srcWidth,
              srcHeight,
              pWarpedY[1],
              pWarpedC[1],
              dstStrideY,
              dstStrideC,
              pMetadataArray[1],
              (roiWindowPerImage[1]),
              (sharpnessScore[1]),
              fPtr);
      }


      for (UINT32 i = threadStartIdx; i < nSharpestFrames; i += nParallelReg)
      {
          for (UINT32 nTid = 0; nTid < nParallelReg; nTid++)
          {
              UINT32 idx = i + nTid;

              pWarpedY[idx] = new UINT8[srcHeight * dstStrideY];
              pWarpedC[idx] = new UINT8[srcHeight * dstStrideY / 2];

              mfnrThreadPool.push_back(std::thread(
                  pMfnrRegister,
                  sessionId,
                  pSrcY_t[0],
                  pSrcC_t[0],
                  srcStrideY,
                  srcStrideC,
                  pSrcY_t[idx],
                  pSrcC_t[idx],
                  srcStrideY,
                  srcStrideC,
                  srcWidth,
                  srcHeight,
                  pWarpedY[idx],
                  pWarpedC[idx],
                  dstStrideY,
                  dstStrideC,
                  pMetadataArray[idx],
                  std::ref(roiWindowPerImage[idx]),
                  std::ref(sharpnessScore[idx]),
                  fPtr));
          }

          // Join nParallel Registration Threads
          for (size_t n = 0; n < mfnrThreadPool.size(); n++)
              if (mfnrThreadPool[n].joinable()) mfnrThreadPool[n].join();

          mfnrThreadPool.clear();
      }

#ifdef ENABLE_DUMPING
      trStop = cv::getTickCount();
#endif

      if (fPtr->MfnrIsFrameAborted(sessionId))
      {
          //De-allocate memory
          for (UINT32 i = 1; i < nSharpestFrames; i++)
          {
              delete[] pWarpedY[i];
              delete[] pWarpedC[i];
          }

          fPtr->MfnrResetFrameAbortFlag(sessionId);

#ifndef ENABLE_BLOCKING_ABORT_CALL
          fPtr->MfnrSetFrameRunningState(sessionId, false);
#endif

          return CamxResultECancelledRequest;
      }

      if (CamxResultSuccess == rc)
      {
         pWarpedY[0] = const_cast<UINT8*>(pSrcY_t[0]);
         pWarpedC[0] = const_cast<UINT8*>(pSrcC_t[0]);

         rc = MfnrProcess( sessionId,
            &pWarpedY.front(),
            &pWarpedC.front(),
            nSharpestFrames,
            dstStrideY,
            dstStrideC,
            srcWidth,
            srcHeight,
            pDstY,
            pDstC,
            dstStrideY,
            dstStrideC,
            &roiWindowPerImage.front(),
            &sharpnessScore.front(),
            outputRoi,
            blendConfidence,
            nBlendedFrames,
            fPtr,
            imageGain,
            isAIDEenabled);
      }

#ifdef ENABLE_INTERMEDIATE_DUMPS_ON_ANDROID

      // Dump MFNR Registration inputs and Warped Outputs

      printf("\n\t%s: Writing Registration In and Warped Out ");
      for (uint32_t i = 0; i < numImages; i++)
      {
          regInfName = dumpFilePrefix + "RegIn_"+ std::to_string(i) + "_" + std::to_string(srcStrideY) + "x" + std::to_string(srcHeight) + "_.yuv";
          regOutWarpedfName = dumpFilePrefix + "RegOutWarped_" + std::to_string(i) + "_" + std::to_string(dstStrideY) + "x" + std::to_string(srcHeight) + "_.yuv";
          
          regIn_fp = fopen(regInfName.c_str(), "wb");
          fwrite(pSrcY[i], sizeof(char), srcStrideY * srcHeight * 1.5, regIn_fp);
          fclose(regIn_fp);

          regOut_fp = fopen(regInfName.c_str(), "wb");
          fwrite(pWarpedY[i], sizeof(char), dstStrideY * srcHeight * 1.5, regOut_fp);
          fclose(regOut_fp);
      }

      gMfnrServiceCallsCnt++;

#endif

#ifdef ENABLE_DUMPING
      tpStop = cv::getTickCount();

      printf("\n\t%s: Combined Registration Time %.2f ms\n", __FUNCTION__, (trStop - trStart) * 1000.0 / cv::getTickFrequency());
      printf("\n\t%s: Combined Process Time %.2f ms\n", __FUNCTION__, (tpStop - trStop) * 1000.0 / cv::getTickFrequency());
#endif
      //De-allocate memory
      for (UINT32 i = 1; i < nSharpestFrames; i++)
      {
         delete[] pWarpedY[i];
         delete[] pWarpedC[i];
      }

      if (fPtr->MfnrIsFrameAborted(sessionId))
      {
          fPtr->MfnrResetFrameAbortFlag(sessionId);

#ifndef ENABLE_BLOCKING_ABORT_CALL
          fPtr->MfnrSetFrameRunningState(sessionId, false);
#endif
          return CamxResultECancelledRequest;
      }

      // Reset MFNR Library Processing Running State
      fPtr->MfnrSetFrameRunningState(sessionId, false);

      return rc;
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Allocate temp working space needed for MFNR
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @param srcWidth
   ///   Width of input images provided in pSrcY
   ///
   /// @param srcHeight
   ///   Height of input images provided in pSrcY
   ///
   /// @return
   ///
   //------------------------------------------------------------------------------
   static inline CamxResult MfnrAllocate( UINTPTR_T sessionId,
      UINT32 srcWidth,
      UINT32 srcHeight,
      qrcpdefs::mfnrLib* fPtr)
   {
      return fPtr->Mfnry101( sessionId,
         srcWidth ,
         srcHeight );
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   De-allocate the temp working space
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @return
   ///
   //------------------------------------------------------------------------------

   static inline CamxResult MfnrDeAllocate(UINTPTR_T sessionId, qrcpdefs::mfnrLib* fPtr)
   {
      return fPtr->Mfnry202( sessionId );
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Create mfnr session
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @return
   ///
   //------------------------------------------------------------------------------

   static inline UINTPTR_T MfnrCreate(qrcpdefs::mfnrLib* fPtr)
   {
      printf("MfnrCreate");
      return fPtr->Mfnry303();
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Destroy sesssion
   ///
   /// @param sessionId
   ///   sessionId
   ///
   /// @return
   ///
   //------------------------------------------------------------------------------

   static inline CamxResult MfnrDestroy( UINTPTR_T sessionId, qrcpdefs::mfnrLib* fPtr)
   {
      return fPtr->Mfnry404( sessionId );
   }

   static inline CamxResult MfnrAbort(UINTPTR_T sessionId, qrcpdefs::mfnrLib* fPtr)
   {
       return fPtr->MfnrAbortFrame(sessionId);
   }

   static inline bool MfnrIsAborted(UINTPTR_T pPrivate, qrcpdefs::mfnrLib* fPtr)
   {
       return fPtr->MfnrIsFrameAborted(pPrivate);
   }

   static inline CamxResult MfnrResetAbortFlag(UINTPTR_T pPrivate, qrcpdefs::mfnrLib* fPtr)
   {
       return fPtr->MfnrResetFrameAbortFlag(pPrivate);
   }

   static inline CamxResult MfnrSetRunningState(UINTPTR_T pPrivate, bool state, qrcpdefs::mfnrLib* fPtr)
   {
       return fPtr->MfnrSetFrameRunningState(pPrivate, state);
   }
}
}

#endif //CAMX_MFNR_WRAPPER_H
