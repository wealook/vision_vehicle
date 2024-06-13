#pragma once

#include "common.h"

namespace rknn {

    class RKDetectionPostprocess {
        virtual int init() = 0;
        virtual int post_process(rknn_app_context_t *app_ctx, void *outputs, letterbox_t *letter_box,
                                 float conf_threshold, float nms_threshold, object_detect_result_list *od_results) = 0;
        virtual int deinit() = 0;

    };
}