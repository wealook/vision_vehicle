
#include "video_saver.h"
#include <filesystem>
#include "logger.h"
#include "string_helper.h"
#include <memory>
#include "sys_utils.h"
#include <arpa/inet.h>
#include <array>

namespace ve {
    namespace fs = std::filesystem;

    inline char *__yang_put_be32(char *output, uint32_t nVal) {
        output[3] = nVal & 0xff;
        output[2] = nVal >> 8;
        output[1] = nVal >> 16;
        output[0] = nVal >> 24;
        return output + 4;
    }

    VideoSaver::VideoSaver() {

    }

    VideoSaver::~VideoSaver() {

    }


    void VideoSaver::pushEncoderData(const char *data, size_t len) {
        auto *tmp = new char[len];
        memcpy(tmp, data, len);
        std::lock_guard<std::mutex> theLock(this->mutex_);
        cacheData_.emplace_back(tmp, len);
    }

    void VideoSaver::saveFrame(const char *data, size_t len) {
        auto lastPos = len - 1;
        std::vector<std::pair<char *, size_t>> buffers;
        for (size_t index = len - 1; index >= 3; index--) {
            if (data[index] == 0x01
                && data[index - 1] == 0x00
                && data[index - 2] == 0x00
                && data[index - 3] == 0x00
                    ) {
                uint32_t len2 = (lastPos - index);
                __yang_put_be32((char *) data + index - 3, (len2));
                buffers.push_back(std::make_pair((char *) data + index - 3, len2 + 4));
                lastPos = index - 4;
            }
        }

//        LOG_ERROR("buffer size:" << buffers.size())
        for (int index = buffers.size() - 1; index >= 0; index--) {
            if (index != 0) {
                continue;
            }
            /*  LOG_WARN("modify:" << wl::StringHelper::toHexString(
                      reinterpret_cast<const unsigned char *>(buffers[index].first),
                      std::min((int) buffers[index].second, 255)));
              LOG_WARN("size:" << buffers[index].second)*/
            auto res = MP4WriteSample(file, videoTrickId_, reinterpret_cast<const uint8_t *>(buffers[index].first),
                                      buffers[index].second, MP4_INVALID_DURATION, 0, 1);

            if (!res) {
                LOG_WARN("MP4WriteSample error")
            }
        }
    }

    void VideoSaver::setSPSPPS(const char *data, size_t len) {
        auto lastPos = len - 1;
        std::vector<std::pair<char *, size_t>> buffers;
        for (size_t index = len - 1; index >= 3; index--) {
            if (data[index] == 0x01
                && data[index - 1] == 0x00
                && data[index - 2] == 0x00
                && data[index - 3] == 0x00
                    ) {
                uint32_t len2 = (lastPos - index);
                buffers.push_back(std::make_pair((char *) data + index + 1, len2));
                lastPos = index - 4;
            }
        }
        if (buffers.size() != 2) {
            LOG_ERROR("sps error");
            return;
        }
        this->pps_.resize(buffers[0].second);
        memcpy(this->pps_.data(), buffers[0].first, buffers[0].second);

        this->sps_.resize(buffers[1].second);
        memcpy(this->sps_.data(), buffers[1].first, buffers[1].second);

//        LOG_WARN("sps:" << this->sps_.size());
//        LOG_WARN("pps:" << this->pps_.size());
    }

    void VideoSaver::saveEncoderData(const char *data, size_t len, size_t width, size_t height) {
        uint8_t fps = 25;
        if (this->initWriter(fps, width, height)) {
//            outFile_.write(data, len);
            this->saveFrame(data, len);
            currentFileFrames_++;
//            LOG_INFO("currentFileFrames_:" << currentFileFrames_)
            if (currentFileFrames_ > fps * this->singleFileTime_) {
                this->closeWriter();
                currentFileFrames_ = 0;
            }
        }
    }

    int64_t VideoSaver::shouldDelete() const {
        if (this->saveBasePath_.empty()) {
            return 0;
        }
        auto space = fs::space(this->saveBasePath_);
//        LOG_INFO(space.capacity / 1024 / 1024 << "    " << space.available / 1024 / 1024 << "    "
//                                              << space.free / 1024 / 1024);
        return this->minSpace_ - space.available;
    }

    void VideoSaver::setSavePath(const std::string &path) {
        this->saveBasePath_ = path;
        if(!std::filesystem::exists(this->saveBasePath_)){
            std::filesystem::create_directories(this->saveBasePath_);
        }
    }

    void VideoSaver::deleteHistoricalFiles(uint64_t minSpaceSizeB) {
        if (this->saveBasePath_.empty()) {
            return;
        }
        if (fs::is_empty(this->saveBasePath_)) {
            LOG_INFO("empty dir:" << this->saveBasePath_);
            return;
        }
        fs::directory_iterator iter(this->saveBasePath_);
        std::vector<std::string> files = {};
        for (fs::directory_iterator end; iter != end; ++iter) {
            if (fs::is_directory(iter->path())) {
                files.emplace_back(iter->path().string());
            }
        }
        std::sort(files.begin(), files.end());
        uint64_t removedSize = 0;
        for (const auto &dir: files) {
            // TODO 当前正在写入的文件要排除
            if (fs::is_empty(dir)) {
                fs::remove(dir);
                LOG_INFO("empty sub dir:" << dir);
                continue;
            }
            fs::directory_iterator iterSub(dir);
            std::vector<std::string> filesSub = {};
            for (fs::directory_iterator end; iterSub != end; ++iterSub) {
                if (!fs::is_directory(iterSub->path())) {
                    filesSub.push_back(iterSub->path());
                }
            }
            for (const auto &it: filesSub) {
                auto fileSize = fs::file_size(it);
                if (fs::remove(it)) {
                    LOG_INFO("remove success :" << it << ",size:" << fileSize << "B" << ","
                                                << (fileSize / 1024 / 1024) << "MB");
                    removedSize += fileSize;
                    // 只要栅一个就行了
                    if (removedSize >= minSpaceSizeB) {
                        return;
                    }
                } else {
                    LOG_WARN("remove fail :" << it << ",size:" << fileSize << "B");
                }

            }
        }
    }

    void VideoSaver::start() {
//        av_register_all(); // 注册所有muxers, demuxers, 和协议
        this->running_ = true;
        this->threadFile_ = std::make_shared<std::thread>([this]() {
            int64_t deleteSize = 0;
            while (running_) {
                deleteSize = this->shouldDelete();
                if (deleteSize > 0) {
                    this->deleteHistoricalFiles((uint64_t) deleteSize * 1.5);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return;
        });
    }

    void VideoSaver::stop() {
        this->running_ = false;
        if (this->thread_ && this->thread_->joinable()) {
            this->thread_->join();
        }
        if (this->threadFile_ && this->threadFile_->joinable()) {
            this->threadFile_->join();
        }
        this->closeWriter();
        // 清理FFmpeg库
        // av_unregister_all();
    }



    bool VideoSaver::closeWriter() {

        MP4Close(file);
        file = nullptr;
        return true;
    }

    bool VideoSaver::initWriter(uint8_t fps, size_t width, size_t height) {
        if (this->file != MP4_INVALID_FILE_HANDLE && this->file != NULL) {
            return true;
        }

        auto saveFile = this->saveBasePath_ + "/" + wl::StringHelper::getTimestampStr("%Y%m%d%H");
        if (!fs::exists(saveFile)) {
            if (!fs::create_directories(saveFile)) {
                LOG_WARN("create path error: " << saveFile)
                return false;
            }
        }
        //LOG_INFO(videoWriter_.getBackendName());
        saveFile = saveFile + "/video_" + wl::StringHelper::getTimestampStr("%H%M%S") + ".mp4";
        this->file = MP4CreateEx(saveFile.c_str());
        if (file == MP4_INVALID_FILE_HANDLE) {
            LOG_ERROR("open file fialed.");
            return false;
        }
        MP4SetTimeScale(file, 90000);
        //添加h264 track
        // TODO 宽高
        videoTrickId_ = MP4AddH264VideoTrack(file, 90000, 90000 / 25, width, height,
                                             this->sps_[1], //sps[1] AVCProfileIndication
                                             this->sps_[2], //sps[2] profile_compat
                                             this->sps_[3], //sps[3] AVCLevelIndication
                                             3); // 4 bytes length before each NAL unit
        if (videoTrickId_ == MP4_INVALID_TRACK_ID) {
            LOG_ERROR("add video track failed");
            return false;
        }

//        std::array<uint8_t, 31> sps_pps = {0x67, 0x64, 0x10, 0x29, 0xAC, 0x1B, 0x1A, 0xA0, 0x50, 0x07, 0x9A, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03, 0x28, 0xF0,
//                                           0x88, 0x46, 0xA0, 0x68, 0xEE, 0x31, 0xB2, 0x1B};
        MP4AddH264SequenceParameterSet(file, videoTrickId_, this->sps_.data(), this->sps_.size());
        MP4AddH264PictureParameterSet(file, videoTrickId_, this->pps_.data(), this->pps_.size());
        MP4SetVideoProfileLevel(file, 0x7F);

//        //添加aac音频
//        MP4TrackId audio = MP4AddAudioTrack(file, 48000, 1024, MP4_MPEG4_AUDIO_TYPE);
//        if (video == MP4_INVALID_TRACK_ID) {
//            printf("add audio track failed.\n");
//            return false;
//        }
//        MP4SetAudioProfileLevel(file, 0x2);
        return true;

    }

    void VideoSaver::setMinSpace(uint64_t minSpace) {
        minSpace_ = minSpace;
    }

}