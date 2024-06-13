#pragma once
//#define OBJ_NUMB_MAX_SIZE 128
//#define OBJ_CLASS_NUM 80
//#define NMS_THRESH 0.45
//#define BOX_THRESH 0.25

#include "rknn_api.h"
#include "stdlib.h"
#include "image_utils.h"


namespace rknn {
    const int OBJ_NUMB_MAX_SIZE = 128;
    const int OBJ_CLASS_NUM = 80;
    
    const float NMS_THRESH = 0.45;
    const float BOX_THRESH = 0.25;
/**
 * @brief Image pixel format
 * 
 */
    typedef struct {
        rknn_context rknn_ctx;
        rknn_input_output_num io_num;
        rknn_tensor_attr *input_attrs;
        rknn_tensor_attr *output_attrs;
        int model_channel;
        int model_width;
        int model_height;
        bool is_quant;
    } rknn_app_context_t;

    typedef struct {
        image_rect_t box;
        float prop;
        int cls_id;
    } object_detect_result;
    typedef struct {
        int id;
        int count;
        object_detect_result results[OBJ_NUMB_MAX_SIZE];
    } object_detect_result_list;
}
