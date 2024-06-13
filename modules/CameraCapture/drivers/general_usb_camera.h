#pragma once

#include <memory>
#include <thread>
#include <string>
#include "camera.h"
#include <mutex>

namespace wl {

    class GeneralUSBCamera final : public Camera {

    public:
        static std::shared_ptr<Camera> creator(const DeviceOptions &devOpts);

        static int getCameraList(std::vector<CameraInfo> &cameraList, const std::string &extraParam);

        GeneralUSBCamera(const std::string videoFileName, int devId, int extIndex);

        ~GeneralUSBCamera();


//        CameraParameter getCameraParam(bool flag = false) override;

        void startCapture(CaptureCallback *cb, bool block = false) override;

        void stopCapture() override;

        int cameraType() override;

        void setCameraOption(CameraOptions opts) override;

        void captureFrame(CaptureCallback *cb, int devId = -1) override;

    private:
        std::string videoFileName_;
//        CameraParameter cameraParameter_;

        std::unique_ptr<std::thread> thread_;
        CaptureCallback *cb_;
        CameraOptions cameraOptions_;

        uint32_t receivedFrames_;
        std::chrono::system_clock::time_point startTimePoint_;
        int fd_ = -1;
    };
}
