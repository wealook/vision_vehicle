#pragma  once

#include <stdint.h>
#include <vector>
#include "rknn_api.h"
#include "common.h"
#include "image_utils.h"
#include "rk_detection_postprocess.h"
//#define OBJ_NAME_MAX_SIZE 64
//#define OBJ_NUMB_MAX_SIZE 128
//#define OBJ_CLASS_NUM 80
//#define NMS_THRESH 0.45
//#define BOX_THRESH 0.25
namespace rknn::yolov6 {


    int init_post_process();

    void deinit_post_process();

    char *coco_cls_to_name(int cls_id);

    int post_process(rknn_app_context_t *app_ctx, void *outputs, letterbox_t *letter_box, float conf_threshold, float nms_threshold, object_detect_result_list *od_results);

    void deinitPostProcess();

}