#pragma  once

#include <opencv2/core.hpp>
#include "general_setting.h"
#include "rk_detection.h"

namespace ve {
    class VisionProvider {


    public:
        int init(const ve::GeneralSetting &generalSet);


        int detection(const cv::Mat &img, int fd, rknn::object_detect_result_list &result);


    private:

        std::shared_ptr< rknn::RKDetection> rkDetection_;

        size_t detectionCounter_ = 0;
    };
}

