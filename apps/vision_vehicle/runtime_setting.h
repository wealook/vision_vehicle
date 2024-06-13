#pragma  once

#include <string>
#include <atomic>

namespace ve {
    class RuntimeSetting {


    public:

        static RuntimeSetting &instance() {
            static RuntimeSetting instance_ = RuntimeSetting();
            return instance_;
        }

        bool getRtcPush() const;

        void setRtcPush(bool rtcPush);

        uint16_t getPushType() const;

        void setPushType(const uint16_t& pushType);

        bool getDetection() const;

        void setDetection(const bool& detection);

    private:
        RuntimeSetting() {}

        // 是否推流
        std::atomic<bool> rtcPush_ = true;
        // 推流的内容类型 0 不推流  1 原始加时间戳的视频  2 检测结果标注的视频
        std::atomic<uint16_t> pushType_ = 0;
        // 是否检测 
        std::atomic<bool> detection = true;

    };
}



