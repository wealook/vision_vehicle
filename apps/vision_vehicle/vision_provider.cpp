//
// Created by cuibo7 on 2024/4/30.
//

#include "vision_provider.h"
#include "logger.h"
#include "jpeg_decoder.h"

namespace ve {
    int VisionProvider::init(const GeneralSetting &generalSet) {
        LOG_INFO("detectionModelDir:" << generalSet.getDetectionModelDir())
//        this->objectDetector_ = std::make_shared<PaddleDetection::ObjectDetector>(generalSet.getDetectionModelDir(), 8,
        this->rkDetection_ = std::make_shared<rknn::RKDetection>();
        this->rkDetection_->init(generalSet.getDetectionModelDir() + "/model.rknn");
        return 0;
    }

    int VisionProvider::detection(const cv::Mat &imgCop, int fd, rknn::object_detect_result_list &resultList) {
        if (imgCop.cols < 640 || imgCop.rows < 640) {
            return -1;
        }
        image_buffer_t src_image;
        memset(&src_image, 0, sizeof(image_buffer_t));
        src_image.width = imgCop.cols;
        src_image.height = imgCop.rows;
        src_image.format = IMAGE_FORMAT_BGR888;
        src_image.virt_addr = imgCop.data;
        src_image.fd = fd;
        src_image.size = imgCop.cols * imgCop.rows * 3;
//        object_detect_result_list resultList;
//        LOG_INFO("start   this->rkDetection_->detection")
        this->rkDetection_->detection(&src_image, &resultList);
//        LOG_INFO("items:" << resultList.count)
        /* for (int i = 0; i < resultList.count; ++i) {
             if (resultList.results[i].prop < 0.6) {
                 continue;
             }
             auto x = resultList.results[i].box.left;
             auto y = resultList.results[i].box.top;
             auto x2 = resultList.results[i].box.right;
             auto y2 = resultList.results[i].box.bottom;
             LOG_INFO("box: "
                              << "x:" << x
                              << "y:" << y
                              << "x2:" << x2
                              << "y2:" << y2
                              << ",scrop:" << resultList.results[i].prop
             )
             cv::rectangle(imgCop, cv::Rect(cv::Point(x, y), cv::Point(x2, y2)), cv::Scalar(255), 1);
         }
         cv::imwrite("img960.png", imgCop);*/
        return 0;

    }
}