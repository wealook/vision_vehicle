#pragma once

#include <thread>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include "mp4v2/mp4v2.h"
//#include "mp4v2/general.h"
//#include "mp4v2/file.h"

#include <queue>

namespace ve {
//    namespace fs = std::filesystem;
//    MP4FileHandle file = MP4CreateEx("test.mp4", MP4_DETAILS_ALL, 0, 1, 1, 0, 0, 0, 0);

    class VideoSaver {

    public:

        VideoSaver();

        ~VideoSaver();


        void pushEncoderData(const char *data, size_t len);

        void saveEncoderData(const char *data, size_t len, size_t width, size_t height);

        void setSPSPPS(const char *data, size_t len);

        void setSavePath(const std::string &path);

        /**
         *
         * @param minSpaceSizeB 最少需要删除多少空间的文件 单位是字节
         */
        void deleteHistoricalFiles(uint64_t minSpaceSizeB);

        void start();

        void stop();

    private:


        int64_t shouldDelete() const;

        bool initWriter(uint8_t fps, size_t width, size_t height);

        bool closeWriter();
//        {
//            auto path = fs::current_path();
//            auto space = fs::space(path);
//
//        }

        std::shared_ptr<std::thread> thread_;
        std::shared_ptr<std::thread> threadFile_;

        std::string saveBasePath_;
        uint64_t minSpace_ = 1024 * 1024 * 512 * 2;// 1G
    public:
        void setMinSpace(uint64_t minSpace);

        void saveFrame(const char *data, size_t len);

    private:
        uint32_t singleFileTime_ = 60;// 每个文件的时长 秒
        // dir[小时]/秒-时长(秒)
        bool running_ = false;
        std::string currentSaveFile_;
        size_t currentFileFrames_ = 0;
        std::mutex mutex_;
        std::ofstream outFile_;
        std::deque<std::pair<char *, size_t >> cacheData_;

        MP4FileHandle file = NULL;
        MP4TrackId videoTrickId_;

        std::vector<uint8_t> sps_;
        std::vector<uint8_t> pps_;
    public:


    };

}


