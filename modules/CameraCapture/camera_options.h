#pragma  once

#include <optional>
#include <string>


namespace wl {
    enum class CameraTriggerMode {
        Auto_Trigger = 0,
        Software_Trigger = 1,
        Hardware_Trigger = 2,
        Software_Trigger_Once = 3,
        Hardware_Trigger_Once = 4
    };

    enum class CameraColorType {
        Color_BGR = 0,      //by default  BGR mode
        Color_Gray = 1,     //mono mode
        Color_I420 = 2      //YUV we only support I420 format by now
    };

    class CameraOptions {
    public:
        std::optional<int> exposureTime;
        std::optional<int> exposureColor;
        std::optional<int> emitter;
        std::optional<float> laserPower;
        std::optional<int> gain;
        std::optional<float> Gamma;

        std::optional<int> fps;

        //sepcial for realsense
        std::optional<bool> norgb;
        std::optional<bool> rgbonly;

        std::optional<int> width;
        std::optional<int> height;

        std::optional<bool> enableGetPointCloud;
        std::optional<bool> enableGetInfraFrame;
        std::optional<bool> enableGetDepth;
        std::optional<bool> alignDepth2Color;
        std::optional<int> disparityShift;


        //industry camera options
        std::optional<CameraTriggerMode> trigerMode;
        std::optional<CameraColorType> pixelType;
        std::optional<int> lineSource;    //hard trigger, line source, by default 0
        std::optional<int> edgeMode;        //0 : rising edge trigger, 1: falling edge trigger, by default 0

        std::optional<bool> doDistort;
        std::optional<bool> flipLeftRight;


        std::optional<std::string> computeUnit;    ///< opencl, cpu, cuda
        std::optional<int> maxPath;                ///< max path, 4 or 6 or 8

        std::optional<int> roiX;
        std::optional<int> roiY;
        std::optional<int> roiWidth;
        std::optional<int> roiHeight;
    };
}
