#pragma once

#include <iostream>

namespace ve {

    class GeneralSetting {

    public:
        static GeneralSetting &instance() {
            static GeneralSetting instance_ = GeneralSetting();
            return instance_;
        }

        bool load(const std::string &fileName);

        uint16_t getBasePort() {
            return this->basePort_;
        }

        uint16_t getCommandPort() const {
            return this->basePort_ + 1;
        }

        const std::string &getRtcServer() const;

        const std::string &getSaveBasePath() const;

        uint64_t getSaveFileMinSpaceSize() const;

        const std::string &getDetectionModelDir() const;

        void setDetectionModelDir(const std::string &detectionModelDir);

        const std::string &getSrsServer() const;

        void setSrsServer(const std::string &srsServer);

    private:
        GeneralSetting() {};
        uint16_t basePort_ = 20000;
        std::string rtcServer;
        std::string srsServer;
        std::string saveBasePath;
        uint64_t saveFileMinSpaceSize = 1024 * 1024 * 512 * 2;// 1G
        std::string detectionModelDir;
    };
}
