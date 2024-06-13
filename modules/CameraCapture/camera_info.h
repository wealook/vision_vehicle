#pragma  once

namespace wl {
    class CameraInfo {
    public:
        CameraInfo() : cameraID_(-1), extraData_(nullptr) {
        }

        int cameraID_;
        std::string cameraName_;
        std::string cameraSerial_;
        std::string chPortID_;
        std::string chModelName_;
        std::string chFamilyName_;
        std::string chDeviceVersion_;
        std::string chManufacturerName_;
        std::string pid;
        std::string vid;
        void *extraData_;
    };
}