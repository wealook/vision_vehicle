#include "drivers/general_usb_camera.h"
#include "camera_driver.h"

namespace wl {

#define IMPLEMENT_CAMERA_DRIVER(name, classname) \
    static CameraDriver _driver_##name(#name, classname::creator, classname::getCameraList);

    std::vector<CameraDriver *> CameraDriver::supportedCameras_;

    CameraDriver *CameraDriver::getCameraDriver(const std::string &camera) {
        IMPLEMENT_CAMERA_DRIVER(general_usb, GeneralUSBCamera);

        std::string cameraType = camera;
//    StringHelper::trim(cameraType);
        for (CameraDriver *driver: supportedCameras_) {
            if (driver && driver->cameraType_ == cameraType) {
                return driver;
            }
        }
        return nullptr;
    }

}
