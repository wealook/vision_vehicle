//
// Created by cuibo7 on 2024/2/23.
//

#include "runtime_setting.h"
#include "logger.h"

bool ve::RuntimeSetting::getRtcPush() const {
    return rtcPush_;
}

void ve::RuntimeSetting::setRtcPush(bool rtcPush) {
    rtcPush_ = rtcPush;
}

uint16_t ve::RuntimeSetting::getPushType() const {
    return pushType_;
}

void ve::RuntimeSetting::setPushType(const uint16_t &pushType) {
    LOG_INFO("setPushType,old:" << pushType_ << ",new:" << pushType)
    pushType_ = pushType;

}

bool ve::RuntimeSetting::getDetection() const {
    return detection;
}

void ve::RuntimeSetting::setDetection(const bool &detection) {
    LOG_INFO("setDetection,old:" << detection << ",new:" << detection)
    this->detection = detection;
}
