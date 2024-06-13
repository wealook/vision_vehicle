#pragma  once

#include <string>

#include "rknn_api.h"
#include "common.h"
#include "file_utils.h"
#include "image_utils.h"
#include "logger.h"
#include <iostream>
#include <memory>
#include <cstring>
#include "postprocess.h"
#include "dma_alloc.h"
#include "dma_buffer.h"

namespace rknn::ppyoloe {
    class RKDetection {


    public:
        RKDetection() {
            dma_buf_alloc(DMA_HEAP_UNCACHE_PATH, 1280 * 960 * 4, &dmaDst_.fd, (void **) &dmaDst_.buffer);
            dmaDst_.size = 1280 * 960 * 4;

        }

        ~RKDetection() {
            this->release_ppyoloe_model();
            dma_buf_free(dmaDst_.size, &dmaDst_.fd, dmaDst_.buffer);
        }

        int init(const std::string &modelFile) {
            int ret;
            int model_len = 0;
            char *model;
            rknn_context ctx = 0;

            LOG_INFO("modelFile:" << modelFile)
            // Load RKNN Model
            model_len = read_data_from_file(modelFile.c_str(), &model);
            if (model == NULL) {
                LOG_ERROR("load_model fail! ");
                return -1;
            }

            ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_MEMORY_NON_CACHEABLE, NULL);
            free(model);
            if (ret < 0) {
                LOG_ERROR("rknn_init fail! ret=%d" << ret);
                return -1;
            }
            // Get Model Input Output Number
            rknn_input_output_num io_num;
            ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
            if (ret != RKNN_SUCC) {
                LOG_ERROR("rknn_query fail! ret=" << ret);
                return -1;
            }
            LOG_INFO("model input num: " << io_num.n_input << ",output num:" << io_num.n_output);
            std::vector<rknn_tensor_attr> input_attrs(io_num.n_input);
//        memset(input_attrs, 0, sizeof(input_attrs));
            for (int i = 0; i < io_num.n_input; i++) {
                input_attrs[i].index = i;
                ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
                if (ret != RKNN_SUCC) {
                    LOG_ERROR("rknn_query fail! ret=" << ret);
                    return -1;
                }
//            dump_tensor_attr(&(input_attrs[i]));
            }

            // Get Model Output Info
            LOG_INFO("output tensors: ");
            std::vector<rknn_tensor_attr> output_attrs(io_num.n_output);
//        memset(output_attrs, 0, sizeof(output_attrs));
            for (int i = 0; i < io_num.n_output; i++) {
                output_attrs[i].index = i;
                ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
                if (ret != RKNN_SUCC) {
                    LOG_INFO("rknn_query fail! ret=" << ret);
                    return -1;
                }
//            dump_tensor_attr(&(output_attrs[i]));
            }

            // Set to context
            app_ctx.rknn_ctx = ctx;

            // TODO
            if (output_attrs[0].qnt_type == RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC && output_attrs[0].type == RKNN_TENSOR_INT8) {
                app_ctx.is_quant = true;
            } else {
                app_ctx.is_quant = false;
            }
            app_ctx.io_num = io_num;
            app_ctx.input_attrs = (rknn_tensor_attr *) malloc(io_num.n_input * sizeof(rknn_tensor_attr));

            memcpy(app_ctx.input_attrs, input_attrs.data(), io_num.n_input * sizeof(rknn_tensor_attr));

            app_ctx.output_attrs = (rknn_tensor_attr *) malloc(io_num.n_output * sizeof(rknn_tensor_attr));

            memcpy(app_ctx.output_attrs, output_attrs.data(), io_num.n_output * sizeof(rknn_tensor_attr));

            if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
                LOG_INFO("model is NCHW input fmt");
                app_ctx.model_channel = input_attrs[0].dims[1];
                app_ctx.model_height = input_attrs[0].dims[2];
                app_ctx.model_width = input_attrs[0].dims[3];
            } else {
                LOG_INFO("model is NHWC input fmt");
                app_ctx.model_height = input_attrs[0].dims[1];
                app_ctx.model_width = input_attrs[0].dims[2];
                app_ctx.model_channel = input_attrs[0].dims[3];
            }
//        printf("model input height=%d, width=%d, channel=%d\n",
//               app_ctx.model_height, app_ctx.model_width, app_ctx.model_channel);

            return 0;
        }

        bool detection(image_buffer_t *img, object_detect_result_list *od_results) {
            int ret;
            image_buffer_t dst_img;
            letterbox_t letter_box;
            rknn_input inputs[app_ctx.io_num.n_input];
            rknn_output outputs[app_ctx.io_num.n_output];
            const float nms_threshold = NMS_THRESH;      // 默认的NMS阈值
            const float box_conf_threshold = BOX_THRESH; // 默认的置信度阈值
            int bg_color = 114;

            if (!(img) || (!od_results)) {
                return -1;
            }

            memset(od_results, 0x00, sizeof(*od_results));
            memset(&letter_box, 0, sizeof(letterbox_t));
            memset(&dst_img, 0, sizeof(image_buffer_t));
            memset(inputs, 0, sizeof(inputs));
            memset(outputs, 0, sizeof(outputs));

            // Pre Process
            dst_img.width = app_ctx.model_width;
            dst_img.height = app_ctx.model_height;
            dst_img.format = IMAGE_FORMAT_RGB888;
            dst_img.size = get_image_size(&dst_img);
            dst_img.virt_addr = static_cast<unsigned char *>(dmaDst_.buffer);
            dst_img.fd = dmaDst_.fd;
            if (dst_img.virt_addr == NULL) {
                printf("malloc buffer size:%d fail!\n", dst_img.size);
                return -1;
            }

            // letterbox
            ret = convert_image_with_letterbox(img, &dst_img, &letter_box, bg_color);
            if (ret < 0) {
                printf("convert_image_with_letterbox fail! ret=%d\n", ret);
                return -1;
            }

            for (int index = 0; index < 1; index++) {
                // Set Input Data
                inputs[index].index = 0;
                inputs[index].type = RKNN_TENSOR_UINT8;
                inputs[index].fmt = RKNN_TENSOR_NHWC;
                inputs[index].size = app_ctx.model_width * app_ctx.model_height * app_ctx.model_channel;
                inputs[index].buf = dst_img.virt_addr;
            }

            ret = rknn_inputs_set(app_ctx.rknn_ctx, app_ctx.io_num.n_input, inputs);
            if (ret < 0) {
                LOG_ERROR("rknn_input_set fail! ret=" << ret);
                return ret;
            }

            // Run
//    printf("rknn_run\n");
            auto t1 = std::chrono::system_clock::now();
            ret = rknn_run(app_ctx.rknn_ctx, nullptr);
            auto t2 = std::chrono::system_clock::now();
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
            LOG_INFO("rknn_run time cast: " << milliseconds.count())
            if (ret < 0) {
                LOG_ERROR("rknn_run fail! ret= " << ret);
                return ret;
            }
            // Get Output
            memset(outputs, 0, sizeof(outputs));
            for (int i = 0; i < app_ctx.io_num.n_output; i++) {
                outputs[i].index = i;
                outputs[i].want_float = (!app_ctx.is_quant);
            }
            ret = rknn_outputs_get(app_ctx.rknn_ctx, app_ctx.io_num.n_output, outputs, NULL);
            if (ret < 0) {
                printf("rknn_outputs_get fail! ret=%d\n", ret);
                return ret;
            }

            // Post Process
            post_process(&app_ctx, outputs, &letter_box, box_conf_threshold, nms_threshold, od_results);
            rknn_outputs_release(app_ctx.rknn_ctx, app_ctx.io_num.n_output, outputs);
            if (dst_img.virt_addr != NULL) {
//            free(dst_img.virt_addr);
            }
            return ret;
        }

        int release_ppyoloe_model() {
            if (app_ctx.input_attrs != NULL) {
                free(app_ctx.input_attrs);
                app_ctx.input_attrs = NULL;
            }
            if (app_ctx.output_attrs != NULL) {
                free(app_ctx.output_attrs);
                app_ctx.output_attrs = NULL;
            }
            if (app_ctx.rknn_ctx != 0) {
                rknn_destroy(app_ctx.rknn_ctx);
                app_ctx.rknn_ctx = 0;
            }
            return 0;
        }


    private:
        rknn_app_context_t app_ctx;

        DMABuffer dmaDst_;
    };
}