#pragma once

#include <vector>
#include <chrono>
#include "camera.h"

namespace wl {

    class CameraDriver {
    public:
        typedef std::shared_ptr<Camera> (*cameraCreator)(const DeviceOptions &devOpts);
        typedef int (*cameraEnumerator)(std::vector<CameraInfo> &cameraList, const std::string &extraParam);


        CameraDriver(const std::string &t, cameraCreator c, cameraEnumerator e)
                : creator_(c), enumerator_(e), cameraType_(t) {
            supportedCameras_.push_back(this);
        }
        static CameraDriver *getCameraDriver(const std::string &camera);
        bool empty() const {
            return ((creator_ == nullptr) || (enumerator_ == nullptr));
        }
        static std::vector<std::string> supportedDrivers() {
            std::vector<std::string> drivers;
            for (auto &driver: supportedCameras_) {
                drivers.push_back(driver->cameraType_);
            }
            return drivers;
        }

    protected:
        friend class Camera;

        static std::vector<CameraDriver *> supportedCameras_;
        cameraCreator creator_;
        cameraEnumerator enumerator_;
        std::string cameraType_;
    };

}
