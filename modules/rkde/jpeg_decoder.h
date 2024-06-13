#pragma  once

#include "rk_mpi.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"

class JpegDecoder {

public:
    ~JpegDecoder();

    int init(uint16_t width, uint16_t height, int fd, size_t size, void *buffer, int fd2, size_t size2, void *buffer2);

    int decode(char *buffer, uint32_t bufferSize, cv::Mat &imgbgr);

    static int cvtcolor(uint16_t width, uint16_t height, char *srcData, char *BGRadd);


    static int cvtcolor2(uint16_t width, uint16_t height, char *srcData, char *BGRadd);

    static int resize(char *srcData, char *dstdata, uint16_t width, uint16_t height, float scaleX, float scaleY);

    static int crop(const cv::Mat &src, cv::Mat &dst, cv::Rect rect);

    static int
    crop2(char *srcData, char *dstdata, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint16_t width2,
          uint16_t height2);

    static int bgr2yuv(uint16_t width, uint16_t height, char *srcData, char *BGRadd);


private:
    MppCtx ctx;
    MppApi *mpi = NULL;
    MppFrame frame = NULL;
    MppBufferGroup bufferGroupFrame = NULL;
    std::vector<uchar> tmpBuffer_;
};

//int main() {
//
//
//    mpp_packet_deinit(&packet);
//
//    LOG_INFO("opencv start decode")
//    cv::Mat mat2 = cv::imdecode(buffer, -1);
//    LOG_INFO("opencv end decode")
//    return 0;
//}