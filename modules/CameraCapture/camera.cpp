#include "logger.h"
#include "camera_driver.h"
//#include "camera_driver.cpp"
#include "camera.h"

namespace wl {


    std::shared_ptr<Camera> Camera::createInstance(const std::string &vendor,
                                                   int type,
                                                   const DeviceOptions &devOpts) {
        CameraDriver *driver = CameraDriver::getCameraDriver(vendor);
        if (driver == nullptr || driver->empty())
            return nullptr;
        return driver->creator_(devOpts);
    }

    int Camera::getCameraList(const std::string cameraType, std::vector<CameraInfo> &cameraList, const std::string &extraParam) {
        CameraDriver *driver = CameraDriver::getCameraDriver(cameraType);
        if (driver == nullptr || driver->empty())
            return 0;

        return driver->enumerator_(cameraList, extraParam);
    }

    std::vector<std::string> Camera::supportedCamera() {
        CameraDriver::getCameraDriver("");
        return CameraDriver::supportedDrivers();
    }


}