
#include "general_setting.h"
#include "yaml-cpp/yaml.h"

namespace ve {
    bool GeneralSetting::load(const std::string &fileName) {
        YAML::Node config = YAML::LoadFile(fileName);
        if (config.IsNull()) {
            return false;
        }
        this->rtcServer = config["rtcServer"].as<std::string>();
        this->srsServer = config["srsServer"].as<std::string>();
        this->basePort_ = config["basePort"].as<uint16_t>();
        this->saveBasePath = config["saveBasePath"].as<std::string>();
        this->saveFileMinSpaceSize = config["saveFileMinSpaceSize"].as<uint64_t>();
        this->detectionModelDir = config["detectionModelDir"].as<std::string>();
        return true;
    }

    const std::string &GeneralSetting::getRtcServer() const {
        return rtcServer;
    }

    const std::string &GeneralSetting::getSaveBasePath() const {
        return saveBasePath;
    }

    uint64_t GeneralSetting::getSaveFileMinSpaceSize() const {
        return saveFileMinSpaceSize;
    }

    const std::string &GeneralSetting::getDetectionModelDir() const {
        return detectionModelDir;
    }

    void GeneralSetting::setDetectionModelDir(const std::string &detectionModelDir) {
        GeneralSetting::detectionModelDir = detectionModelDir;
    }

    const std::string &GeneralSetting::getSrsServer() const {
        return srsServer;
    }

    void GeneralSetting::setSrsServer(const std::string &srsServer) {
        GeneralSetting::srsServer = srsServer;
    }
}