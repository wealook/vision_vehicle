#pragma once

#include "opencv2/core.hpp"

namespace wl {

    class CaptureCallback {
    public:
        virtual ~CaptureCallback() {}

        virtual void onIncomingRGB(int extID, const cv::Mat &rgb) = 0;

        virtual void onIncomingOrigin(int extID, void *data, size_t len, int type) {}
    };
}