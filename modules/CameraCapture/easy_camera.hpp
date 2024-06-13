#pragma once

#include <string>
#include "camera.h"
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include "logger.h"
#include "opencv2/core.hpp"

namespace wl {
    class EasyCamera : public CaptureCallback {
    public:
        EasyCamera() : rgbCB_(nullptr), originCB_(nullptr), waitingRGB_(false) {
        }

        virtual ~EasyCamera() {
        }

        typedef std::function<void(int, const cv::Mat &)> RGBObserver;
        typedef std::function<void(int, void *data, size_t len, int type)> OriginObserver;

    public:

        void updateOptions(const CameraOptions &opts) {
            options_ = opts;
            if (camera_) {
                camera_->setCameraOption(options_);
            }
        }

        CameraOptions cameraOptions() const {
            return options_;
        }

        void setDevOptions(const DeviceOptions &devOpt) {
            devOpts_ = devOpt;
        }

        void start(const std::string &driver, const std::string &sn, OriginObserver cb) {
            originCB_ = cb;
            devOpts_.deviceSerial = sn;
            camera_ = Camera::createInstance(driver, Camera_RGB, devOpts_);
            camera_->setCameraOption(options_);
            camera_->startCapture(this);
        }


        void restart(CameraOptions newOpt) {
            if (!camera_) {
                LOG_ERROR("invalid call to restart camera: the camera is not opened.");
                return;
            }
            camera_->stopCapture();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            options_ = newOpt;
            camera_->setCameraOption(options_);
            camera_->startCapture(this);
        }

        void stop() {
            if (camera_)
                camera_->stopCapture();
            waitingRGB_ = false;
        }

        /// Software trigger, still callback through callback function.
        void trigger() {
            if (camera_)
                camera_->captureFrame(this);

            camera_->cameraType();
        }

        int cameraType() {
            return camera_->cameraType();
        }


        /// Capture RGB or Depth only, based on camera type
        /// @param trigger do software trigger
        cv::Mat capture(int timeout, bool trigSoftware) {
            if (!camera_) {
                return cv::Mat();
            }
            {
                std::unique_lock<std::mutex> theLock(rgbMutex_);
                waitingRGB_ = true;
            }
            if (trigSoftware)
                trigger();
            std::unique_lock<std::mutex> theLock(rgbMutex_);
            if (waitingRGB_) {
                auto result = rgbCond_.wait_for(theLock, std::chrono::milliseconds(timeout));
                waitingRGB_ = false;
                if (result == std::cv_status::timeout) {
                    return cv::Mat();
                }
            }
            return rgbCaptured_;
        }

        int capture(cv::Mat &rgb, int timeout) {
            if (!camera_)
                return -1;

            {
                std::unique_lock<std::mutex> theLock(rgbMutex_);
                waitingRGB_ = true;
            }

            CameraTriggerMode ctm;
            ctm = *options_.trigerMode;
            if (ctm == CameraTriggerMode::Software_Trigger)
                trigger();

            std::unique_lock<std::mutex> theLock(rgbMutex_);
            if (waitingRGB_) {
                auto result = rgbCond_.wait_for(theLock, std::chrono::milliseconds(timeout));
                waitingRGB_ = false;
                if (result == std::cv_status::timeout) {
                    return -1;
                }
            }
            rgbCaptured_.copyTo(rgb);
            return 0;
        }


        void setRGBObserver(RGBObserver cb) {
            rgbCB_ = cb;
        }

    protected:


        virtual void onIncomingRGB(int extID, const cv::Mat &rgb) override {
            if (rgbCB_)
                rgbCB_(extID, rgb);

            std::unique_lock<std::mutex> theLock(rgbMutex_);
            if (waitingRGB_) {
                rgb.copyTo(rgbCaptured_);
                waitingRGB_ = false;
                rgbCond_.notify_all();
            }
        }

        virtual void onIncomingOrigin(int extID, void *data, size_t len, int type) override {
            if (originCB_)
                originCB_(extID, data, len, type);
        }
        DeviceOptions devOpts_;
        RGBObserver rgbCB_;
        OriginObserver originCB_;
        RGBObserver amplCB_;
        Camera::Ptr camera_;
        CameraOptions options_;
        std::condition_variable cond_;
        std::mutex rgbMutex_;
        bool waitingRGB_;
        std::condition_variable rgbCond_;
        cv::Mat first_;
        cv::Mat second_;
        cv::Mat rgbCaptured_;
    };

}
