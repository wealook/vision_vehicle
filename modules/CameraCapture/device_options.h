#pragma  once

#include <string>
#include <vector>

namespace wl {


    class DeviceOptions {
    public:
        DeviceOptions(const std::string &path = "", int camIdx = 0, int externalID = 0)
                : cameraIndex(camIdx), externalIndex(externalID), paramPath(path) {}

        virtual ~DeviceOptions() {}

        int cameraIndex;                        ///< Camera Index which will be open when there are multiple cameras of that driver hooked, by default, 0
        int externalIndex;                      ///< External Identifier for the opened device, in case of upper layer APP opened multiple device, this id can differentiate the callback data
        std::string deviceSerial;               ///< Camera serial number will be open, it has higher priority than cameraIndex, when this is not empty, it will be used
        std::string cacheFolder;                ///< Particular parameter for fake camera, will read saved images from this folder
        std::string vid;                        ///< USB vendor id of the UVC camera, should be merged to deviceSerial in future
        std::string pid;                        ///< USB product id of the UVC camera, used together with vid, should be merged to deviceSerial
        std::string paramPath;                  ///< Stereo Parameter for Camera to recitify/undistort the image
        std::vector<std::string> serialNums;    ///< Dual Camera, camera list
        std::vector<int> cameraIndexes;         ///< Dual Camera, open with indexes
        std::string extraParam;                 ///< Some device particular extra parameters, which can only be parsed by the device itself, no general meaning, now only for internal use
    };

}