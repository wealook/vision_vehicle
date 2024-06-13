#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include <memory>
#include "camera_info.h"
#include "camera_options.h"
#include "device_options.h"
#include "capture_callback.h"

namespace wl {



//    enum CameraTriggerMode {
//        Auto_Trigger = 0,
//        Software_Trigger = 1,
//        Hardware_Trigger = 2,
//        Software_Trigger_Once = 3,
//        Hardware_Trigger_Once = 4
//    };
//
//    enum CameraColorType {
//        Color_BGR = 0,      //by default  BGR mode
//        Color_Gray = 1,     //mono mode
//        Color_I420 = 2      //YUV we only support I420 format by now
//    };


    enum CameraType {
        Camera_RGB = 1,
    };

/**
 * Common Depth Camera Capture callback with RGB and depth in cv::Mat 
 */


    class Camera {
    public:
        typedef std::shared_ptr<Camera> Ptr;

        virtual ~Camera() {}

        bool isWorking() { return working_; }


        static Camera::Ptr createInstance(const std::string &vendor, int type, const DeviceOptions &deviceOptions);

        static int getCameraList(const std::string vendor, std::vector<CameraInfo> &cameraList, const std::string &extraParam = "");

        static std::vector<std::string> supportedCamera();

        virtual void setCameraOption(CameraOptions opts) {}

        virtual void startCapture(CaptureCallback *cb, bool block = false) = 0;

        virtual void stopCapture() = 0;

        virtual int cameraType() = 0;


        /**
         * Software trigger the camera to capture one frame, but still use callback to return data
         * It is not widely implemented yet, but it is useful in some cases
         */
        virtual void captureFrame(CaptureCallback *cb, int devId = -1) {}

        /**
         * Software trigger one frame and block to wait the frame for the timeout milliseconds, you MUST NOT set callback
         * It is not implemented by most drivers
         */
//        virtual cv::Mat getOneFrame(int timeout = 3000) { return cv::Mat(); };

        virtual std::string deviceName() { return ""; }

    protected:
        bool working_;
        int deviceIndex_;
        int externalID_;
    };

}
