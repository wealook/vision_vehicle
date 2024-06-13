#include "general_usb_camera.h"
#include <memory>
#include <string>
#include <cstring>
#include "logger.h"

#if defined(LINUX_GCC)

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#endif
namespace wl {
    std::shared_ptr<Camera> GeneralUSBCamera::creator(const DeviceOptions &devOpts) {
        return std::shared_ptr<Camera>(
                std::make_shared<GeneralUSBCamera>(devOpts.deviceSerial, devOpts.cameraIndex, devOpts.externalIndex));
    }

    int GeneralUSBCamera::getCameraList(std::vector<CameraInfo> &cameraList, const std::string &extraParam) {
        cameraList.clear();
        for (int index = 0; index < 10; index++) {
            auto fd = open(("/dev/video" + std::to_string(index)).c_str(), O_RDWR);
            if (fd == -1) {
                continue;
            }
            close(fd);
            CameraInfo cameraInfo;
            cameraInfo.cameraID_ = index;
//            cameraInfo.linkType_ = CameraInfo::CameraLinkType::LINK_USB;
            cameraList.push_back(cameraInfo);
        }
        return 0;
    }

    GeneralUSBCamera::GeneralUSBCamera(const std::string videoFileName, int devId, int extIndex) {
        this->deviceIndex_ = devId;
        this->externalID_ = extIndex;
        this->videoFileName_ = videoFileName;
    }

    GeneralUSBCamera::~GeneralUSBCamera() {
        this->working_ = false;
        this->stopCapture();
    }


#if defined(_WINDOWS)
    void GeneralUSBCamera::startCapture(RGBDCallback *cb, bool block) {
        cb_ = cb;
        this->working_ = true;
#if defined(_WINDOWS)
        //        videoCapture_ = std::make_unique<cv::VideoCapture>(this->deviceIndex_, cv::CAP_DSHOW);
                videoCapture_ = std::make_unique<cv::VideoCapture>(this->deviceIndex_);
#else
        videoCapture_ = std::make_unique<cv::VideoCapture>(this->deviceIndex_, cv::CAP_ANY);
#endif
        videoCapture_->set(cv::VideoCaptureProperties::CAP_PROP_FPS,
                           this->cameraOptions_.fps ? *this->cameraOptions_.fps : 30);
        if (this->cameraOptions_.width && this->cameraOptions_.height) {
            videoCapture_->set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, *this->cameraOptions_.width);
            videoCapture_->set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, *this->cameraOptions_.height);
        }
        videoCapture_->set(cv::VideoCaptureProperties::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
//        videoCapture_->set(cv::VideoCaptureProperties::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('B', 'G', '1', '0'));

        auto width = (int) videoCapture_->get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
        auto height = (int) videoCapture_->get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
        this->cameraParameter_.width = width;
        this->cameraParameter_.height = height;
        LOG_INFO((int) this->cameraOptions_.trigerMode.initialized());
        if (this->cameraOptions_.trigerMode == CameraTriggerMode::Auto_Trigger) {

            this->thread_ = std::make_unique<std::thread>([this]() {
                cv::Mat img;
                startTimePoint_ = std::chrono::system_clock::now();
                while (this->working_) {
                    if (videoCapture_->read(img)) {
//                        LOG_INFO("t1")
                        cb_->onIncomingRGB(externalID_, img);
//                        LOG_INFO("t2")

                        receivedFrames_++;
                        if (receivedFrames_ % 100 == 0) {
                            auto end = std::chrono::system_clock::now();
                            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    end - startTimePoint_);
                            startTimePoint_ = end;
                            LOG_INFO("current capturing fps " << receivedFrames_ << ", fps= "
                                                              << (100000.0 / milliseconds.count()));
                        }
//                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    } else {
//                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
            });
        }
    };
#else

    void GeneralUSBCamera::startCapture(CaptureCallback *cb, bool block) {
        cb_ = cb;
        LOG_INFO("startCapture :" << this->deviceIndex_)
        fd_ = open(("/dev/video" + std::to_string(this->deviceIndex_)).c_str(), O_RDWR);
        if (fd_ == -1) {
            LOG_ERROR("open camera error:" << this->deviceIndex_)
            return;
        }
        struct v4l2_format fmt;
        // 设置视频格式
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = *this->cameraOptions_.width;
        fmt.fmt.pix.height = *this->cameraOptions_.height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.colorspace = V4L2_PIX_FMT_YUV422P;
//        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        if (ioctl(fd_, VIDIOC_S_FMT, &fmt) == -1) {
            LOG_ERROR("VIDIOC_S_FMT")
            return;
        }
        this->working_ = true;

        if (this->cameraOptions_.trigerMode == CameraTriggerMode::Auto_Trigger) {
//            jpegDecoder->init(*this->cameraOptions_.width, *this->cameraOptions_.height);
//            return;
            this->thread_ = std::make_unique<std::thread>([this]() {
                std::vector<char> tmpData((*this->cameraOptions_.width) * (*this->cameraOptions_.height) * 3);
//                cv::Mat tmp = cv::Mat(cv::Size(*this->cameraOptions_.width, *this->cameraOptions_.height), CV_8UC3,
//                                      tmpData.data());;

                struct v4l2_buffer buf;
                struct v4l2_requestbuffers reqbuf;
                // 分配缓冲区
                memset(&reqbuf, 0, sizeof(reqbuf));
                reqbuf.count = 1;
                reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                reqbuf.memory = V4L2_MEMORY_MMAP;
                if (ioctl(fd_, VIDIOC_REQBUFS, &reqbuf) == -1) {
                    LOG_ERROR("VIDIOC_REQBUFS");
                    return;
                }

                // 映射内存缓冲区
                memset(&buf, 0, sizeof(buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = 0;
                if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
                    LOG_ERROR("VIDIOC_QUERYBUF");
                    return;
                }
                char *buffer;

                auto length = buf.length;
//                LOG_INFO("length=" << length)
                buffer = static_cast<char *>(mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset));
                if (buffer == MAP_FAILED) {
                    LOG_ERROR("mmap")
                    return;
                }
                {
                    memset(&buf, 0, sizeof(buf));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    buf.index = 0;
                    if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
                        LOG_ERROR("VIDIOC_QBUF")
                        return;
                    }
                }
                // 开始采集
                if (ioctl(fd_, VIDIOC_STREAMON, &buf.type) == -1) {
                    LOG_ERROR("VIDIOC_STREAMON")
                    return;
                }

                startTimePoint_ = std::chrono::system_clock::now();
                while (this->working_) {
//                    LOG_INFO("start_copy")
                    memset(&buf, 0, sizeof(buf));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
//                    LOG_INFO("wait 1")
                    if (ioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
                        LOG_ERROR("VIDIOC_DQBUF");
                        continue;
                    }

//                    LOG_INFO("offset=" << buf.m.offset << ",len=" << buf.bytesused)

//                    LOG_INFO("wait 2")
                    receivedFrames_++;
                    if (receivedFrames_ % 100 == 0) {
                        auto end = std::chrono::system_clock::now();
                        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                                end - startTimePoint_);
                        startTimePoint_ = end;
                        LOG_INFO("current capturing fps " << receivedFrames_ << ", fps= "
                                                          << (100000.0 / milliseconds.count()));
                    }
                    /*  LOG_INFO("success ,length=" << buf.length)
                      std::vector<char> data(buf.length);
                      memcpy(data.data(), buffer, buf.length);
                      cv::Mat img = cv::imdecode(data, buf.length);
                      if (!img.empty()) {
                          LOG_INFO("emg." << img.cols)
                      }

*/
//                    std::vector<char> data(buf.bytesused);
//                    memcpy(data.data(), buffer, buf.bytesused);
                    auto t1 = std::chrono::system_clock::now();
                    auto len = buf.bytesused;

//                    LOG_INFO("mat2,width" << imgyuv.cols << ",height=" << imgyuv.rows << ",chanel=" << imgyuv.channels())

                    /*   if (!img.empty()) {
                           cv::Mat img2;
                           cv::cvtColor(img, img2, cv::COLOR_YUV2BGR_UYVY);
                           cv::imwrite("./save2.jpg", img2);
                       }*/

//                    LOG_INFO("end copy")

//                    jpegDecoder->decode(buffer, len, tmp, [this](void *data, size_t len) {
////                        cv::Mat tmp2 = cv::Mat(
////                                cv::Size(*this->cameraOptions_.width, (*this->cameraOptions_.height) * 1.5),
////                                CV_8UC1,
////                                (char*)data);
////                        cb_->onIncomingRGB(externalID_, tmp2);
////                        auto *tmp = data;
//                        cb_->onIncomingOrigin(externalID_, data, len, 1);
////                        cb_->onIncomingOrigin(externalID_, nullptr, len, 1);
//                    });
//                    LOG_INFO("1")
                    cb_->onIncomingOrigin(externalID_, buffer, len, 1);
//                    cb_->onIncomingRGB(externalID_, tmp);
                    auto t2 = std::chrono::system_clock::now();
                    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
//                    LOG_INFO("decode time cast: " << milliseconds.count())
//                    LOG_INFO((void *) imgyuv.data)

//                    cv::Mat rgb;
                    /*  if (!tmp.empty()) {
                          cv::cvtColor(tmp, rgb, cv::COLOR_YUV2BGR_I420);
                          cv::imwrite("./save_" + std::to_string(receivedFrames_) + ".png", rgb);
                      }*/

//                    cv::Mat mat2 = cv::imdecode(data, CV_8UC1, -1);
//                    LOG_INFO("mat2,width" << mat2.cols << ",height=" << mat2.rows)
//                    std::this_thread::sleep_for(std::chrono::milliseconds(30));

                    memset(&buf, 0, sizeof(buf));
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    buf.index = 0;
                    if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
                        LOG_ERROR("VIDIOC_QBUF")
                        continue;
                    }
                }

                // 关闭摄像头采集
                auto res = ioctl(fd_, VIDIOC_STREAMOFF, &buf.type);
                if (res == -1) {
                    LOG_ERROR("ioctl stream off");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if (munmap(buffer, length) == -1) {
                    LOG_ERROR("munmap");
                }
                close(this->fd_);
                LOG_INFO("endddddddddd")
            });
        }

//        FILE *fp = fopen(FILENAME, "wb");
//        fwrite(buffer, 1, length, fp);
//        fclose(fp);

        /*  // 停止采集
          if (ioctl(fd_, VIDIOC_STREAMOFF, &buf.type) == -1) {
              perror("VIDIOC_STREAMOFF");
              exit(EXIT_FAILURE);
          }

          // 释放缓冲区
          if (munmap(buffer, length) == -1) {
              perror("munmap");
              exit(EXIT_FAILURE);
          }
          // 关闭V4L2设备
          close(fd_);*/
    }

#endif

    void GeneralUSBCamera::stopCapture() {
        this->working_ = false;

        if (this->thread_ && this->thread_->joinable()) {
            this->thread_->join();
        }
        // 关闭摄像头采集
        auto type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        auto res = ioctl(fd_, VIDIOC_STREAMOFF, &type);
        if (res == -1) {
            LOG_ERROR("ioctl stream off");
        }
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        close(this->fd_);

    }

    int GeneralUSBCamera::cameraType() {
        return CameraType::Camera_RGB;
    }

    void GeneralUSBCamera::setCameraOption(CameraOptions opts) {
        cameraOptions_ = opts;
//        cameraOptions_.optionalSet(opts);
    }


    void GeneralUSBCamera::captureFrame(CaptureCallback *cb, int devId) {
//        cv::Mat img;
//        LOG_INFO("read completed!");
//        cb->onIncomingRGB(devId, img);
    };
}
