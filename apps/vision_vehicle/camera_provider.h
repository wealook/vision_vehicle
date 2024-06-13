#pragma  once

#include <iostream>
#include <map>
#include "easy_camera.hpp"
//#include "video_pusher.h"
#include "general_setting.h"
#include "runtime_setting.h"
#include <memory>
#include <thread>
#include "video_saver.h"
#include "vision_provider.h"
#include "jpeg_decoder.h"
#include "h264_encoder.h"
#include "RgaUtils.h"
//#include "dma_alloc.hpp"
#include "dma_alloc.h"
#include "dma_buffer.h"
#include "im2d.hpp"
#include "httplib.h"

namespace ve {


    class CameraProvider {

    public:


        CameraProvider() {

        };

        ~CameraProvider() {
            if (this->threadPush_ && this->threadPush_->joinable()) {
                this->threadPush_->join();
            }
        };

        void initDMA(DMABuffer &dmaBuffer, size_t size) {
            auto ret = dma_buf_alloc(DMA_HEAP_UNCACHE_PATH, size, &dmaBuffer.fd, (void **) &dmaBuffer.buffer);
            if (ret < 0) {
                LOG_ERROR("alloc src dma_heap buffer failed!");
                return;
            }
            dmaBuffer.size = size;
            LOG_INFO("fd:" << dmaBuffer.fd << ",size" << dmaBuffer.size << ",buffer:" << dmaBuffer.buffer)
        }

        char *yang_put_be32(char *output, uint32_t nVal) {
            output[3] = nVal & 0xff;
            output[2] = nVal >> 8;
            output[1] = nVal >> 16;
            output[0] = nVal >> 24;
            return output + 4;
        }



        void start() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            running_ = true;
            jpegDecoder = std::make_shared<JpegDecoder>();

//            this->videoPusher_ = std::make_shared<VideoPusher>();
//            this->videoPusher_->initPublish(const_cast<char *>(ve::GeneralSetting::instance().getRtcServer().c_str()),
//                                            false);
            this->videoSaver_ = std::make_shared<VideoSaver>();
            this->videoSaver_->setSavePath(ve::GeneralSetting::instance().getSaveBasePath());
            this->videoSaver_->setMinSpace(ve::GeneralSetting::instance().getSaveFileMinSpaceSize());
            LOG_INFO("1");
            this->videoSaver_->start();
            LOG_INFO("2");

            this->visionProvider_ = std::make_shared<VisionProvider>();
            this->visionProvider_->init(GeneralSetting::instance());

//            cv::Mat tmp;
//            this->visionProvider_->detection(tmp);
            this->camera_ = std::make_shared<wl::EasyCamera>();
            LOG_INFO("2-2");

            wl::DeviceOptions dOptions;
            //    dOptions.deviceSerial = "C:/Users/GCTG-22002/Videos/20230707_130339.mp4";
            dOptions.cameraIndex = 4;
            dOptions.externalIndex = 0;
            wl::CameraOptions cOptions;
            cOptions.trigerMode = wl::CameraTriggerMode::Auto_Trigger;
            cOptions.width = 1920;
            cOptions.height = 1080;
//            cOptions.width = 1280;
//            cOptions.height = 960;

            // TODO 对齐的问题
            this->initDMA(this->dmaBufferEncoded_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferBGRYUV1_, (*cOptions.width) * (*cOptions.height) * 4);

            this->initDMA(this->dmaBufferBGR1_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferBGR2_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferBGR3_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferBGR4_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferBGR4Crop_, (*cOptions.width) * (*cOptions.height) * 4);
            // 预留对齐多分配一半
            this->initDMA(this->dmaBufferBGRYUV2_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferH264_, (*cOptions.width) * (*cOptions.height) * 4);

            this->initDMA(this->dmaBufferH264header_, 1024);
            this->initDMA(this->dmaBufferHalfBottomYUV_, (*cOptions.width) * (*cOptions.height) / 2);

            this->initDMA(this->dmaBufferBGRYUVDetection_, (*cOptions.width) * (*cOptions.height) * 4);
            this->initDMA(this->dmaBufferH264Detection_, (*cOptions.width) * (*cOptions.height) * 4);

            LOG_INFO("2-3-0");
            jpegDecoder->init((*cOptions.width), (*cOptions.height),
                              dmaBufferBGRYUV1_.fd, dmaBufferBGRYUV1_.size, dmaBufferBGRYUV1_.buffer,
                              dmaBufferEncoded_.fd, dmaBufferEncoded_.size, dmaBufferEncoded_.buffer
            );
            cOptions.fps = 25;
            /*if (this->imgYUV_.empty()) {
                this->imgYUV_ = cv::Mat((*cOptions.height) * 3 / 2, *cOptions.width, CV_8UC1, cv::Scalar(255));
            }*/
            LOG_INFO("2-3");
            this->imgYUVData_.resize((*cOptions.height) * 3 / 2 * (*cOptions.width));
            this->imgYUVData2_.resize((*cOptions.height) * 3 / 2 * (*cOptions.width));
            LOG_INFO("2-4");

            this->imgYUVTmpData_.resize((*cOptions.height) / 2 * (*cOptions.width));
            this->imgYUVTmpData_.resize((*cOptions.height) / 2 * (*cOptions.width));
            this->imgBGRData_.resize((*cOptions.height) * 3 * (*cOptions.width));
            LOG_INFO("3-1");
            this->h264Encoder_ = std::make_shared<H264Encoder>();
            LOG_INFO("3");
            this->h264Encoder_->init(*cOptions.width, (*cOptions.height),
                                     dmaBufferBGRYUV2_.fd, dmaBufferBGRYUV2_.size, dmaBufferBGRYUV2_.buffer,
                                     dmaBufferH264_.fd, dmaBufferH264_.size, dmaBufferH264_.buffer, 2
            );
            LOG_INFO("4")
            this->h264Encoder_->run([this, cOptions](const char *data, size_t len, int type, size_t pts) {
                if (this->videoSaver_) {
//                    LOG_INFO("origin:" << wl::StringHelper::toHexString(reinterpret_cast<const unsigned char *>(data),
//                                                                         std::min(255, (int) len)));
//                    if (this->videoPusher_ && RuntimeSetting::instance().getPushType() == 1) {
//                        this->videoPusher_->putEncodeData(const_cast<char *>(data), len, type, pts);
//                    }
                    if (type == 0) {
                        this->videoSaver_->setSPSPPS(data, len);
                    } else {
//                        LOG_INFO("t1")
                        this->videoSaver_->saveEncoderData(data, len, *cOptions.width, *cOptions.height);
//                        LOG_INFO("t2")
                    }
                }
                return;
            });
            LOG_INFO("5")
            this->h264EncoderDetection_ = std::make_shared<H264Encoder>();
            this->h264EncoderDetection_->init(*cOptions.width, (*cOptions.height),
                                              dmaBufferBGRYUVDetection_.fd, dmaBufferBGRYUVDetection_.size,
                                              dmaBufferBGRYUVDetection_.buffer,
                                              dmaBufferH264Detection_.fd, dmaBufferH264Detection_.size,
                                              dmaBufferH264Detection_.buffer, 2
            );
            LOG_INFO("6")
            this->h264EncoderDetection_->run([this](const char *data, size_t len, int type, size_t pts) {
//                if (this->videoPusher_ && RuntimeSetting::instance().getPushType() == 2) {
//                    this->videoPusher_->putEncodeData(const_cast<char *>(data), len, type, pts);
//                }
            });
            int defaultCameraIndex = -1;
            std::vector<wl::CameraInfo> li;
            wl::Camera::getCameraList("general_usb", li);
            for (const auto &it: li) {
//                LOG_INFO(it.cameraID_ << ",type" << it.linkType_)
                defaultCameraIndex = it.cameraID_;
                break;
            }
            dOptions.cameraIndex = defaultCameraIndex;
            camera_->setDevOptions(dOptions);
            camera_->updateOptions(cOptions);

            camera_->start("general_usb", dOptions.deviceSerial,
                           [this, cOptions](int externalId, void *data, size_t len, int type) {
                               if (jpegDecoder->decode(static_cast<char *>(data), len, tmpEmpty_)) {
                                   LOG_WARN("decode error")
                                   return;
                               }
//                               LOG_INFO("t1")
                               this->YUV2BGR(*cOptions.width, *cOptions.height, this->dmaBufferBGRYUV1_,
                                             this->dmaBufferBGR1_);
//                               LOG_INFO("t2")

                               this->BGRCopy(*cOptions.width, *cOptions.height, this->dmaBufferBGR1_,
                                             this->dmaBufferBGR2_);
//                               LOG_INFO("t3")

                               std::unique_lock<std::mutex> theLock(this->mutexYUV_);
                               this->BGRCopy(*cOptions.width, *cOptions.height, this->dmaBufferBGR1_,
                                             this->dmaBufferBGR3_);
                               hasDetectionImg_ = true;
                               theLock.unlock();
//                               LOG_INFO("t4")

                               cv::Mat tmp = cv::Mat(cv::Size(*cOptions.width, *cOptions.height), CV_8UC3,
                                                     this->dmaBufferBGR2_.buffer);
                               this->writeImateTimestamp(tmp);
//                               LOG_INFO("t5")

                               this->cvtI420(*cOptions.width, *cOptions.height, this->dmaBufferBGR2_,
                                             this->dmaBufferBGRYUV2_);
//                               LOG_INFO("t6")

                               this->h264Encoder_->pushYUVImage(nullptr, *cOptions.width, *cOptions.height);
//                               LOG_INFO("t7")

                               return;

                           });

            threadPush_ = std::make_shared<std::thread>([this, cOptions]() {
                while (running_) {
                    if (!RuntimeSetting::instance().getDetection()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }
                    if (!hasDetectionImg_) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        continue;
                    }

                    int width = *cOptions.width;
                    int height = *cOptions.height;
                    std::unique_lock<std::mutex> theLock(this->mutexYUV_);
                    this->BGRCopy(width, height, this->dmaBufferBGR3_, this->dmaBufferBGR4_);
                    hasDetectionImg_ = false;
                    theLock.unlock();
                    this->imgCrop(width, height, this->dmaBufferBGR4_, 960, 960, this->dmaBufferBGR4Crop_);
                    cv::Mat imgCop = cv::Mat(cv::Size(960, 960), CV_8UC3, this->dmaBufferBGR4Crop_.buffer);
                    rknn::object_detect_result_list resultList;
//                    cv::imwrite("imgcrop.png",imgCop);
                    this->visionProvider_->detection(imgCop, dmaBufferBGR4Crop_.fd, resultList);
                    cv::Mat tmp = cv::Mat(cv::Size(width, height), CV_8UC3, this->dmaBufferBGR4_.buffer);
                    bool find = false;
                    for (int i = 0; i < resultList.count; ++i) {
                        if (resultList.results[i].prop < 0.6) {
                            continue;
                        }
                        auto x = resultList.results[i].box.left;
                        auto y = resultList.results[i].box.top;
                        auto x2 = resultList.results[i].box.right;
                        auto y2 = resultList.results[i].box.bottom;
//                        LOG_INFO("box: "
//                                         << "x:" << x
//                                         << "y:" << y
//                                         << "x2:" << x2
//                                         << "y2:" << y2
//                                         << ",scrop:" << resultList.results[i].prop
//                        )
                        cv::rectangle(tmp, cv::Rect(cv::Point(x + (width - 960) / 2, y + (height - 960) / 2),
                                                    cv::Point(x2 + (width - 960) / 2, y2 + (height - 960) / 2)),
                                      cv::Scalar(0, 0, 255), 2);
                        find = true;
                    }
                    this->writeImateTimestamp(tmp);
                    this->cvtI420(width, height, this->dmaBufferBGR4_, this->dmaBufferBGRYUVDetection_);
                    if (RuntimeSetting::instance().getPushType() == 2) {
                        this->h264EncoderDetection_->pushYUVImage(nullptr, width, height);
                    }
                    continue;
                }
            });
//            cv::Mat empty = cv::Mat(1, 1, CV_8UC1, cv::Scalar(255));
//            this->setCachedPushFrame(0, empty);
        }

        void writeImateTimestamp(const cv::Mat &img) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()) % 1000;
            auto timeStr = wl::StringHelper::getTimestampStr("%Y-%m-%d %H:%M:%S") + "." + std::to_string(ms.count());
            cv::putText(img, timeStr, cv::Point(50, img.rows - 50), cv::FONT_HERSHEY_SIMPLEX, 1.5,
                        cv::Scalar(0, 0, 255), 4);
        }

        void cvtI420(uint16_t width, uint16_t height, DMABuffer &srcDMA, DMABuffer &dstDMA) {
            this->BGR2YUV(width, height, srcDMA, dstDMA);
            return;
            {
                // rga 直接转420p会报错，可能是大于4g内存的原因
                // 因此，rga先转420sp，手动交换内存转为420p
                memset(this->dmaBufferHalfBottomYUV_.buffer, 0, this->dmaBufferHalfBottomYUV_.size);
                auto baseIndex = width * height;
                for (int index = 0; index < width * height / 2; index += 2) {
                    ((char *) (this->dmaBufferHalfBottomYUV_.buffer))[index / 2] = ((char *) (dstDMA.buffer))[index +
                                                                                                              baseIndex];
                    ((char *) (this->dmaBufferHalfBottomYUV_.buffer))[index / 2 +
                                                                      width * height / 4] = ((char *) (dstDMA.buffer))[
                            index + baseIndex + 1];
                }
                memcpy(((char *) (dstDMA.buffer)) + baseIndex, ((char *) (this->dmaBufferHalfBottomYUV_.buffer)),
                       width * height / 2);
            }
        }

        void stop() {
            LOG_INFO("0")
            LOG_INFO(camera_)
            if (camera_) {
                camera_->stop();
            }
            LOG_INFO("1")
            if (this->videoSaver_) {
                this->videoSaver_->stop();
            }
            if (this->h264Encoder_) {
                this->h264Encoder_->stop();
            }
        }

    private:
        void setCachedPushFrame(int idx, const cv::Mat &img) {
            std::lock_guard<std::mutex> theLock(mutexPush_);
            this->cachedPushFrames_[idx] = img.clone();
        }

        void resumeCachedPushFrame(int idx, cv::Mat &img) {
            std::lock_guard<std::mutex> theLock(mutexPush_);
            this->cachedPushFrames_[idx].copyTo(img);
            this->cachedPushFrames_[idx] = cv::Mat();
        }

        int YUV2BGR(uint16_t width, uint16_t height, DMABuffer &srcDMA, DMABuffer &dstDMA) {
            auto src_handle = importbuffer_fd(srcDMA.fd, width * height * get_bpp_from_format(RK_FORMAT_YCbCr_420_SP));
            auto dst_handle = importbuffer_fd(dstDMA.fd, width * height * get_bpp_from_format(RK_FORMAT_BGR_888));
            if (src_handle == 0 || dst_handle == 0) {
                printf("import dma_fd error! ");
                return -1;
            }
            auto src_img = wrapbuffer_handle_t(src_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               RK_FORMAT_YCbCr_420_SP);
            auto dst_img = wrapbuffer_handle_t(dst_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               RK_FORMAT_BGR_888);

            auto ret44 = imcheck(src_img, dst_img, {}, {});
            if (IM_STATUS_NOERROR != ret44) {
                LOG_ERROR("%d, check error! %s");
                return -1;
            }
            ret44 = imcvtcolor(src_img, dst_img, RK_FORMAT_YCbCr_420_SP, RK_FORMAT_BGR_888);
            if (ret44 == IM_STATUS_SUCCESS) {
            } else {
                LOG_ERROR("error " << ret44)
                return -1;
            }
            return 0;
        }

        int BGRCopy(uint16_t width, uint16_t height, DMABuffer &srcDMA, DMABuffer &dstDMA) {
            auto src_handle = importbuffer_fd(srcDMA.fd, width * height * get_bpp_from_format(RK_FORMAT_BGR_888));
            auto dst_handle = importbuffer_fd(dstDMA.fd, width * height * get_bpp_from_format(RK_FORMAT_BGR_888));
            if (src_handle == 0 || dst_handle == 0) {
                printf("import dma_fd error! ");
                return -1;
            }
            auto src_img = wrapbuffer_handle_t(src_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               RK_FORMAT_BGR_888);
            auto dst_img = wrapbuffer_handle_t(dst_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               RK_FORMAT_BGR_888);

            auto ret44 = imcheck(src_img, dst_img, {}, {});
            if (IM_STATUS_NOERROR != ret44) {
                LOG_ERROR("%d, check error! %s");
                return -1;
            }
            ret44 = imcopy(src_img, dst_img);
            if (ret44 == IM_STATUS_SUCCESS) {
            } else {
                LOG_ERROR("error " << ret44)
                return -1;
            }
            return 0;
        }

        int imgCrop(uint16_t width, uint16_t height, DMABuffer &srcDMA, uint16_t width2, uint16_t height2,
                    DMABuffer &dstDMA) {
            auto src_handle = importbuffer_fd(srcDMA.fd, width * height * get_bpp_from_format(RK_FORMAT_BGR_888));
            auto dst_handle = importbuffer_fd(dstDMA.fd, width2 * height2 * get_bpp_from_format(RK_FORMAT_BGR_888));
            if (src_handle == 0 || dst_handle == 0) {
                printf("import dma_fd error! ");
                return -1;
            }
            auto src_img = wrapbuffer_handle_t(src_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               RK_FORMAT_BGR_888);
            auto dst_img = wrapbuffer_handle_t(dst_handle, width2, height2, width2, std::ceil(height2 / 16.0) * 16,
                                               RK_FORMAT_BGR_888);

            auto ret44 = imcheck(src_img, dst_img, {}, {});
            if (IM_STATUS_NOERROR != ret44) {
                LOG_ERROR("%d, check error! %s");
                return -1;
            }
            im_rect rect;
            rect.x = (width - width2) / 2;
            rect.y = (height - height2) / 2;
            rect.width = width2;
            rect.height = height2;
//            LOG_INFO("x:" << rect.x)
//            LOG_INFO("y:" << rect.y)
//            LOG_INFO("width:" << rect.width)
//            LOG_INFO("height:" << rect.height)
            ret44 = imcrop(src_img, dst_img, rect);
            if (ret44 == IM_STATUS_SUCCESS) {
            } else {
                LOG_ERROR("error " << ret44)
                return -1;
            }
            return 0;
        }

        int BGR2YUV(uint16_t width, uint16_t height, DMABuffer &srcDMA, DMABuffer &dstDMA) {
            auto srcFormat = RK_FORMAT_BGR_888;
            auto dstFormat = RK_FORMAT_YCbCr_420_SP;
//            auto dstFormat = RK_FORMAT_YCbCr_420_P;
            auto src_handle = importbuffer_fd(srcDMA.fd, width * height * get_bpp_from_format(srcFormat));
            auto dst_handle = importbuffer_fd(dstDMA.fd, width * height * get_bpp_from_format(dstFormat));
            if (src_handle == 0 || dst_handle == 0) {
                printf("import dma_fd error! ");
                return -1;
            }
            auto src_img = wrapbuffer_handle_t(src_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               srcFormat);
            auto dst_img = wrapbuffer_handle_t(dst_handle, width, height, width, std::ceil(height / 16.0) * 16,
                                               dstFormat);

            auto ret44 = imcheck(src_img, dst_img, {}, {});
            if (IM_STATUS_NOERROR != ret44) {
                LOG_ERROR("%d, check error! %s");
                return -1;
            }
            ret44 = imcvtcolor(src_img, dst_img, srcFormat, dstFormat);
            if (ret44 == IM_STATUS_SUCCESS) {
            } else {
                LOG_ERROR("error " << ret44)
                return -1;
            }
            return 0;
        }


    private:
        std::shared_ptr<wl::EasyCamera> camera_;
        std::map<int, cv::Mat> cachedPushFrames_;
        std::recursive_mutex mutex_;
        std::mutex mutexPush_;
        std::mutex mutexYUV_;
//        std::shared_ptr<VideoPusher> videoPusher_;
        std::shared_ptr<std::thread> threadPush_;
        std::shared_ptr<VideoSaver> videoSaver_;
        std::shared_ptr<VisionProvider> visionProvider_;
        std::shared_ptr<H264Encoder> h264Encoder_;
        std::shared_ptr<H264Encoder> h264EncoderDetection_;
        bool running_ = false;
        cv::Mat lastYUV_;
        std::vector<char> imgYUVData_;
        std::vector<char> imgYUVData2_;
        std::vector<char> imgBGRData_;
        std::vector<char> imgYUVTmpData_;
        std::atomic<bool> hasDetectionImg_ = false;

        // yuv->bgr
        DMABuffer dmaBufferBGR1_;
        // bgr for video
        DMABuffer dmaBufferBGR2_;
        // bgr for detection
        DMABuffer dmaBufferBGR3_;
        DMABuffer dmaBufferBGR4_;
        DMABuffer dmaBufferBGR4Crop_;
        // mjepg
        DMABuffer dmaBufferEncoded_;
        // mjpeg-> bgr
        DMABuffer dmaBufferBGRYUV1_;

        //  video bgr -> yuv
        DMABuffer dmaBufferBGRYUV2_;

        DMABuffer dmaBufferBGRYUVDetection_;
        DMABuffer dmaBufferH264Detection_;
        // h264
        DMABuffer dmaBufferH264_;
        DMABuffer dmaBufferH264header_;

        DMABuffer dmaBufferHalfBottomYUV_;

        std::shared_ptr<JpegDecoder> jpegDecoder;

        cv::Mat tmpEmpty_;

        size_t h264HeaderLen_ = 0;
    };
}


