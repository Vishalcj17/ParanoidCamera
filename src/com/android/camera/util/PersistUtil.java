/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package com.android.camera.util;

import android.graphics.Point;
import android.text.TextUtils;
import android.util.Log;

import java.lang.reflect.Method;

public class PersistUtil {

    public static final int CAMERA2_DEBUG_DUMP_IMAGE = 1;
    public static final int CAMERA2_DEBUG_DUMP_LOG = 2;
    public static final int CAMERA2_DEBUG_AUDIO = 3;
    public static final int CAMERA2_DEBUG_VIDEO = 4;
    public static final int CAMERA2_DEBUG_MEDIACODEC = 5;

    public static final int CAMERA2_DEBUG_DUMP_ALL = 100;
    public static final int CAMERA2_DEV_OPTION_ALL = 100;

    private static final int CAMERA_SENSOR_VERTICAL_ALIGNED = 1;

    // camera1 prop start
    private static final boolean PERSIST_CAMERA_SAVE_IN_SD_ENABLED =
            getBoolean("persist.sys.env.camera.saveinsd", false);
    private static final boolean PERSIST_LONG_SAVE_ENABLED =
            getBoolean("persist.sys.camera.longshot.save", false);
    private static final boolean PERSIST_CAMERA_PREVIEW_RESTART_ENABLED =
            getBoolean("persist.sys.camera.feature.restart", false);
    private static final boolean PERSIST_CAPTURE_ANIMATION_ENABLED =
            getBoolean("persist.sys.camera.capture.animate", true);
    private static final boolean PERSIST_SKIP_MEM_CHECK_ENABLED =
            getBoolean("persist.sys.camera.perf.skip_memck", false);
    private static final boolean PERSIST_ZZHDR_ENABLED =
            getBoolean("persist.sys.camera.zzhdr.enable", false);
    private static final int PERSIST_PREVIEW_SIZE =
            getInt("persist.sys.camera.preview.size", 0);
    // camera1 prop end

    private static final String PERSIST_HFR_LIMIT =
            get("persist.sys.camera.hfr.rate", "");
    private static final boolean PERSIST_SKIP_MEMORY_CHECK =
            getBoolean("persist.sys.camera.perf.skip_memck", false);
    private static final int PERSIST_LONGSHOT_SHOT_LIMIT =
            getInt("persist.sys.camera.longshot.shotnum", 60);
    private static final String PERSIST_CAMERA_PREVIEW_SIZE =
            get("persist.sys.camera.preview.size", "");
    private static final String PERSIST_CAMERA_VIDEO_SNAPSHOTSIZE =
            get("persist.sys.camera.video.snapshotsize", "");
    private static final String PERSIST_CAMERA_VIDEO_SIZE =
            get("persist.sys.camera.video.size", "");
    private static final boolean PERSIST_CAMERA_CAMERA2 =
            getBoolean("persist.sys.camera.camera2", true);
    private static final boolean PERSIST_CAMERA_ZSL =
            getBoolean("persist.sys.camera.zsl.disabled", false);
    private static final int PERSIST_CAMERA_CANCEL_TOUCHFOCUS_DELAY =
            getInt("persist.sys.camera.focus_delay", 5000);
    private static final int PERSIST_CAMERA_DEBUG =
            getInt("persist.sys.camera.debug", 0);
    private static final boolean PERSIST_CAMERA_FD_DEBUG =
            getBoolean("persist.sys.camera.fd.debug", false);
    private static final boolean PERSIST_CAMERA_TRACE_DEBUG =
            getBoolean("persist.sys.camera.trace.debug", false);
    private static final int PERSIST_CAMERA_DEV_DEBUG_OPTION =
            getInt("persist.sys.camera.devoption.debug", 0);
    // StillMore filter start
    private static final String PERSIST_CAMERA_STILLMORE_BRCOLR =
            get("persist.sys.camera.stm_brcolor", "0.5");
    private static final String PERSIST_CAMERA_STILLMORE_BRINTENSITY =
            get("persist.sys.camera.stm_brintensity", "0.6");
    private static final String PERSIST_CAMERA_STILLMORE_SMOOTHINGINTENSITY =
            get("persist.sys.camera.stm_smooth", "0");
    private static final int PERSIST_CAMERA_STILLMORE_NUM_REQUIRED_IMAGE =
            getInt("persist.sys.camera.stm_img_nums", 5);
    // StillMore filter end
    private static final int CIRCULAR_BUFFER_SIZE_PERSIST =
            getInt("persist.sys.camera.zsl.buffer.size", 9);
    private static final int SAVE_TASK_MEMORY_LIMIT_IN_MB =
            getInt("persist.sys.camera.perf.memlimit", 120);
    private static final boolean PERSIST_CAMERA_UI_AUTO_TEST_ENABLED =
            getBoolean("persist.sys.camera.ui.auto_test", false);
    private static final boolean PERSIST_SEND_REQUEST_AFTER_FLUSH =
            getBoolean("persist.sys.camera.send_request_after_flush", false);
    // ClearSight Start
    private static final long PERSIST_TIMESTAMP_LIMIT =
            getInt("persist.sys.camera.cs.threshold", 10);
    private static final int PERSIST_BURST_COUNT =
            getInt("persist.sys.camera.cs.burstcount", 4);
    private static final boolean PERSIST_DUMP_FRAMES_ENABLED =
            getBoolean("persist.sys.camera.cs.dumpframes", false);
    private static final boolean PERSIST_DUMP_YUV_ENABLED =
            getBoolean("persist.sys.camera.cs.dumpyuv", false);
    private static final int PERSIST_CS_TIMEOUT =
            getInt("persist.sys.camera.cs.timeout", 300);
    private static final boolean PERSIST_DUMP_DEPTH_ENABLED =
            getBoolean("persist.sys.camera.cs.dumpdepth", false);
    // ClearSight eend
    private static final String PERSIST_DISPLAY_UMAX =
            get("persist.sys.camera.display.umax", "");
    private static final String PERSIST_DISPLAY_LMAX =
            get("persist.sys.camera.display.lmax", "");
    private static final boolean PERSIST_VIDEO_LIVESHOT =
            getBoolean("persist.sys.camera.video.liveshot",false);
    private static final boolean PERSIST_VIDEO_EIS =
            getBoolean("persist.sys.camera.video.eis",false);
    private static final int PERSIST_BURST_PREVIEW_REQUEST_NUMS =
            getInt("persist.sys.camera.burst.preview.nums", 1);
    private static final boolean PERSIST_SSM_ENABLE =
            getBoolean("persist.sys.camera.ssm.enable", false);
    private static final boolean PERSIST_FD_RENDERING_SUPPORTED =
            getBoolean("persist.sys.camera.isFDRenderingSupported", false);
    private static final boolean PERSIST_CAM_ENABLE_POST_ZOOM_FOV =
            getBoolean("persist.sys.enable_post_zoom_fov", true);
    public static final boolean PERSIST_MULTI_CAMERA_ENABLED =
            getBoolean("persist.sys.camera.multiCameraEnabled", false);
    private static final boolean PERSIST_CAM_FD_SUPPORTED =
            getBoolean("persist.sys.camera.isCamFDSupported", false);
    private static final int PERSIST_MCTF_VALUE =
            getInt("persist.sys.camera.sessionParameters.mctf", 0);
    private static final boolean PERSIST_RAW_REPROCESS_ENABLE =
            getBoolean("persist.sys.camera.raw_reprocess_enable", false);
    private static final boolean PERSIST_RAW_REPROCESS_QCFA =
            getBoolean("persist.sys.camera.raw_reprocess_qcfa", false);
    private static final int PERSIST_ZOOM_FRAME_VALUE =
            getInt("persist.sys.camera.zoom.frame", 10);
    private static final boolean PERSIST_RAW_CB_INFO_SUPPORTED =
            getBoolean("persist.sys.camera.rawcbinfo", false);
    private static final int PERSIST_LIVE_SHOT_NUMBERS =
            getInt("persist.sys.camera.live_shot_numbers", 0);
    private static final int PERSIST_AIDE_FRAME_NUMBERS =
            getInt("persist.sys.camera.aide_frame_numbers", 0);
    private static final String PERSIST_CAMERA_MAX_BURST_SHOT_FPS =
            get("persist.sys.camera.maxBurstShotFPS", "0");

    public static String getHFRRate() {
        return PERSIST_HFR_LIMIT;
    }

    public static boolean getSkipMemoryCheck() {
        return PERSIST_SKIP_MEMORY_CHECK;
    }

    public static int getLongshotShotLimit() {
        return PERSIST_LONGSHOT_SHOT_LIMIT;
    }
    public static int getLongshotShotLimit(int defaultValue) {
        return  getInt("persist.sys.camera.longshot.shotnum", defaultValue);
    }

    public static Point getCameraPreviewSize() {
        Point result = null;
        if (PERSIST_CAMERA_PREVIEW_SIZE != null) {
            String[] sourceStrArray = PERSIST_CAMERA_PREVIEW_SIZE.split("x");
            if (sourceStrArray != null && sourceStrArray.length >= 2) {
                result = new Point();
                result.x = Integer.parseInt(sourceStrArray[0]);
                result.y = Integer.parseInt(sourceStrArray[1]);
            }
        }
        return result;
    }

    public static String getVideoSnapshotSize(){
        return PERSIST_CAMERA_VIDEO_SNAPSHOTSIZE;
    }

    public static Point getCameraVideoSize() {
        Point result = null;
        if (PERSIST_CAMERA_VIDEO_SIZE != null) {
            String[] sourceStrArray = PERSIST_CAMERA_VIDEO_SIZE.split("x");
            if (sourceStrArray != null && sourceStrArray.length >= 2) {
                result = new Point();
                result.x = Integer.parseInt(sourceStrArray[0]);
                result.y = Integer.parseInt(sourceStrArray[1]);
            }
        }
        return result;
    }

    private static Method getIntMethod = null;
    private static Method getBooleanMethod = null;
    private static Method getStringMethod = null;

    private static int getInt(final String key, final int def) {
        try {
            if (getIntMethod == null) {
                getIntMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("getInt", String.class, int.class);
            }
            return (int) getIntMethod.invoke(null, key, def);
        } catch (Exception e) {
            Log.e("Persist", "SystemProperties error: " + e.toString());
            return def;
        }
    }

    private static boolean getBoolean(final String key, final boolean def) {
        try {
            if (getBooleanMethod == null) {
                getBooleanMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("getBoolean", String.class,boolean.class);
            }

            return (Boolean)getBooleanMethod.invoke(null, key, def);
        } catch (Exception e) {
            Log.e("Persist", "SystemProperties error: " + e.toString());
            return def;
        }
    }

    private static String get(final String key, final String def) {
        try {
            if (getStringMethod == null) {
                getStringMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("get", String.class, String.class);
            }

            return (String)getStringMethod.invoke(null, key, def);
        } catch (Exception e) {
            Log.e("Persist", "SystemProperties error: " + e.toString());
            return def;
        }
    }

    public static int getZoomFrameValue() {return  PERSIST_ZOOM_FRAME_VALUE; }

    public static boolean getCamera2Mode() {
        return PERSIST_CAMERA_CAMERA2;
    }

    public static boolean getCameraZSLDisabled() {
        return PERSIST_CAMERA_ZSL;
    }

    public static int getCamera2Debug() {
        return PERSIST_CAMERA_DEBUG;
    }

    public static boolean getFdDebug(){
        return PERSIST_CAMERA_FD_DEBUG;
    }

    public static boolean getTraceDebug(){
        return PERSIST_CAMERA_TRACE_DEBUG;
    }

    public static int getDevOptionLevel() {
        return PERSIST_CAMERA_DEV_DEBUG_OPTION;
    }

    public static float getStillmoreBrColor(){
        float brColor = Float.parseFloat(PERSIST_CAMERA_STILLMORE_BRCOLR);
        return brColor = (brColor < 0 || brColor > 1) ? 0.5f : brColor;
    }
    public static float getMaxBurstShotFPS(){
        return Float.parseFloat(PERSIST_CAMERA_MAX_BURST_SHOT_FPS);
    }

    public static float getStillmoreBrIntensity(){
        float brIntensity = Float.parseFloat(PERSIST_CAMERA_STILLMORE_BRINTENSITY);
        return brIntensity = (brIntensity < 0 || brIntensity > 1) ? 0.6f : brIntensity;
    }

    public static float getStillmoreSmoothingIntensity(){
        float smoothingIntensity = Float.parseFloat(PERSIST_CAMERA_STILLMORE_SMOOTHINGINTENSITY);
        return smoothingIntensity = (smoothingIntensity < 0 || smoothingIntensity > 1) ?
                0f : smoothingIntensity;
    }

    public static int getStillmoreNumRequiredImages() {
        return (PERSIST_CAMERA_STILLMORE_NUM_REQUIRED_IMAGE < 3 ||
                PERSIST_CAMERA_STILLMORE_NUM_REQUIRED_IMAGE > 5) ?
                5 : PERSIST_CAMERA_STILLMORE_NUM_REQUIRED_IMAGE;
    }

    public static int getCancelTouchFocusDelay() {
        return PERSIST_CAMERA_CANCEL_TOUCHFOCUS_DELAY;
    }

    public static int getCircularBufferSize(){
        return CIRCULAR_BUFFER_SIZE_PERSIST;
    }

    public static int getSaveTaskMemoryLimitInMb(){
        return SAVE_TASK_MEMORY_LIMIT_IN_MB;
    }

    public static boolean isAutoTestEnabled(){
        return PERSIST_CAMERA_UI_AUTO_TEST_ENABLED;
    }

    public static boolean isSaveInSdEnabled(){
        return PERSIST_CAMERA_SAVE_IN_SD_ENABLED;
    }

    public static boolean isLongSaveEnabled(){
        return PERSIST_LONG_SAVE_ENABLED;
    }

    public static boolean isPreviewRestartEnabled(){
        return PERSIST_CAMERA_PREVIEW_RESTART_ENABLED;
    }

    public static boolean isCaptureAnimationEnabled(){
        return PERSIST_CAPTURE_ANIMATION_ENABLED;
    }

    public static boolean isSkipMemoryCheckEnabled(){
        return PERSIST_SKIP_MEM_CHECK_ENABLED;
    }

    public static boolean isZzhdrEnabled(){
        return PERSIST_ZZHDR_ENABLED;
    }

    public static boolean isSendRequestAfterFlush() {
        return PERSIST_SEND_REQUEST_AFTER_FLUSH;
    }

    public static int getPreviewSize(){
        //Read Preview Resolution from adb command
        //value: 0(default) - Default value as per snapshot aspect ratio
        //value: 1 - 640x480
        //value: 2 - 720x480
        //value: 3 - 1280x720
        //value: 4 - 1920x1080
        return PERSIST_PREVIEW_SIZE;
    }

    public static long getTimestampLimit(){
        return PERSIST_TIMESTAMP_LIMIT;
    }

    public static int getImageToBurst(){
        return PERSIST_BURST_COUNT;
    }

    public static boolean isDumpFramesEnabled(){
        return PERSIST_DUMP_FRAMES_ENABLED;
    }

    public static boolean isDumpYUVEnabled(){
        return PERSIST_DUMP_YUV_ENABLED;
    }

    public static int getClearSightTimeout(){
        return PERSIST_CS_TIMEOUT;
    }

    public static boolean isDumpDepthEnabled() {
        return PERSIST_DUMP_DEPTH_ENABLED;
    }

    public static boolean is3ADebugEnabled() {
        return getBoolean("persist.sys.cameraapp.3adebug", false);
    }

    public static boolean enableMediaRecorder() {
        return getBoolean("persist.sys.cameraapp.mediarecorder", true);
    }

    public static boolean isPersistVideoLiveshot(){
        return PERSIST_VIDEO_LIVESHOT;
    }

    public static boolean isPersistVideoEis(){
        return PERSIST_VIDEO_EIS;
    }

    public static String getDisplayUMax() {
        return PERSIST_DISPLAY_UMAX;
    }

    public static String getDisplayLMax() {
        return PERSIST_DISPLAY_LMAX;
    }

    public static int isBurstShotFpsNums() {
        return PERSIST_BURST_PREVIEW_REQUEST_NUMS;
    }

    public static boolean isSSMEnabled() {
        return PERSIST_SSM_ENABLE;
    }

    public static boolean isCameraPostZoomFOV() {
        return PERSIST_CAM_ENABLE_POST_ZOOM_FOV;
    }

    public static boolean isMultiCameraEnabled() {return  PERSIST_MULTI_CAMERA_ENABLED;}

    public static boolean isFDRENDERINGSUPPORTED() {return PERSIST_FD_RENDERING_SUPPORTED; }

    public static boolean isCameraFDSupported() {return PERSIST_CAM_FD_SUPPORTED; }

    public static int mctfValue() { return PERSIST_MCTF_VALUE; }

    public static boolean isRawReprocessQcfa() {
        return PERSIST_RAW_REPROCESS_QCFA;
    }

    public static boolean isRawReprocessEnable() {
        return PERSIST_RAW_REPROCESS_ENABLE;
    }

    public static boolean isRawCbInfoSupported() {return PERSIST_RAW_CB_INFO_SUPPORTED; }

    public static int isLiveShotNumbers() {return PERSIST_LIVE_SHOT_NUMBERS; }

    public static int getAideFrameNumbers() {return PERSIST_AIDE_FRAME_NUMBERS; }
}
