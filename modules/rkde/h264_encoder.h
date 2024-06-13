#pragma  once

#include "mpp_log.h"
#include "rk_mpi.h"
#include "logger.h"
#include "sys_utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "string_helper.h"
#include "jpeg_decoder.h"
#include <functional>

class H264Encoder {
public:
    H264Encoder() {

    }

    ~H264Encoder() {

    }

    int init(uint16_t width, uint16_t height, int fd, size_t size, void *buffer, int fd2, size_t size2, void *buffer2,
             int IType) {
        this->width_ = width;
        this->height_ = height;
        LOG_INFO(this->width_)
        LOG_INFO(this->height_)
        // buffer错误会导致系统崩溃
/*
        mpp_buffer_group_get_internal(&bufferGroup, MPP_BUFFER_TYPE_ION);
        LOG_INFO("buffergroup:" << bufferGroup)

        mpp_buffer_group_limit_config(&bufferGroup, width * height * 4, 4);
        */


        mpp_buffer_group_get_external(&bufferGroup, MPP_BUFFER_TYPE_DMA_HEAP);
        MppBufferInfo info[2];
        info[0].ptr = buffer;
        info[0].size = size;
        info[0].index = 0;
        info[0].fd = fd;
        info[0].type = MPP_BUFFER_TYPE_DMA_HEAP;

        info[1].ptr = buffer2;
        info[1].size = size2;
        info[1].index = 0;
        info[1].fd = fd2;
        info[1].type = MPP_BUFFER_TYPE_DMA_HEAP;
        LOG_INFO("init -2")

        auto ret = mpp_buffer_commit(bufferGroup, &info[0]);
        ret = mpp_buffer_commit(bufferGroup, &info[1]);
        if (ret) {
            LOG_ERROR("failed to mpp_buffer_commit ret  " << ret);
            return -1;
        }

        ret = mpp_buffer_get(bufferGroup, &bufferFrame, this->width_ * this->height_ * 4);
        if (ret) {
            LOG_ERROR("mpp_buffer_get ::" << ret)
            LOG_ERROR("mpp_buffer_get ::" << (width_ * height_ * 3 / 2))
            LOG_ERROR(this->width_)
            LOG_ERROR(this->height_)
        }
        ret = mpp_buffer_get(bufferGroup, &packageBuffer, this->width_ * this->height_ * 4);
        if (ret) {
            LOG_ERROR("mpp_buffer_get failed " << ret);
        }
        // 初始化RKMPP
        ret = mpp_create(&ctx, &mpi);
        if (ret) {
            // 错误处理
            LOG_ERROR("error " << ret)
            return -1;
        }
        MppPollType timeout = MPP_POLL_NON_BLOCK;
        ret = mpi->control(ctx, MPP_SET_INPUT_TIMEOUT, &timeout);
        if (ret) {
            LOG_ERROR("mpi control set input timeout %d ret " << ret);
            return ret;
        }

        timeout = MPP_POLL_BLOCK;
        auto timeoutT = 50;
        ret = mpi->control(ctx, MPP_SET_OUTPUT_TIMEOUT, &timeoutT);
        if (ret) {
            LOG_ERROR("mpi control set output timeout %d ret " << ret);
            return ret;
        }
        ret = mpp_init(ctx, MPP_CTX_ENC, MPP_VIDEO_CodingAVC);
        if (ret) {
            LOG_ERROR("ret2=" << ret)
            return -1;
        }

        MppEncCfg cfg = NULL;
        ret = mpp_enc_cfg_init(&cfg);
        if (ret) {
            LOG_ERROR("mpp_dec_cfg_init fail=" << ret)
            return -1;
        }
        ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
        if (ret) {
            LOG_ERROR("  failed to get decoder cfg ret  " << ret);
            return -1;
        }
        ret = mpi->control(ctx, MPP_ENC_GET_CFG, cfg);
        if (ret) {
            LOG_ERROR("MPP_DEC_SET_CFG fail=" << ret)
            return -1;
        }

//            MppEncRefCfg ref = NULL;
/*

            */
/* setup default parameter *//*

            if (p->fps_in_den == 0)
                p->fps_in_den = 1;
            if (p->fps_in_num == 0)
                p->fps_in_num = 30;
            if (p->fps_out_den == 0)
                p->fps_out_den = 1;
            if (p->fps_out_num == 0)
                p->fps_out_num = 30;

            if (!p->bps)
                p->bps = p->width * p->height / 8 * (p->fps_out_num / p->fps_out_den);
*/

        mpp_enc_cfg_set_s32(cfg, "prep:width", width);
        mpp_enc_cfg_set_s32(cfg, "prep:height", height);
        auto hor_stride = width;
        auto ver_stride = std::ceil(height / 16.0) * 16;
        mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", hor_stride);
        mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", ver_stride);
        mpp_enc_cfg_set_s32(cfg, "prep:format", MPP_FMT_YUV420SP);
//        auto rc_mode = MPP_ENC_RC_MODE_VBR;
        auto rc_mode = MPP_ENC_RC_MODE_CBR;
        mpp_enc_cfg_set_s32(cfg, "rc:mode", rc_mode);
        auto fps_out_num = 25;
        auto fps_out_den = 1;
        auto fps_out_flex = 0;

        auto fps_in_num = 25;
        auto fps_in_den = 1;
        auto fps_in_flex = 0;

//        auto bps = width * height / 8 * (fps_out_num / fps_out_den);
        auto bps = width * height * 8 * 4;
//        auto bps_max = width * height * 8*10;
        auto bps_max = width * height * 8 * 10;
        auto bps_min = width * height * 4 * 1;
        /* fix input / output frame rate */
        mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", fps_in_flex);
        mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", fps_in_num);
        mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", fps_in_den);
        mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", fps_out_flex);
        mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", fps_out_num);
        mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", fps_out_den);

        /* drop frame or not when bitrate overflow */
        mpp_enc_cfg_set_u32(cfg, "rc:drop_mode", MPP_ENC_RC_DROP_FRM_DISABLED);
        mpp_enc_cfg_set_u32(cfg, "rc:drop_thd", 20);        /* 20% of max bps */
        mpp_enc_cfg_set_u32(cfg, "rc:drop_gap", 1);         /* Do not continuous drop frame */

        /* setup bitrate for different rc_mode */

//            mpp_enc_cfg_set_s32(cfg, "rc:bps_target", p->bps);
        switch (rc_mode) {
            case MPP_ENC_RC_MODE_FIXQP : {
//                        do not setup bitrate on FIXQP mode
            }
                break;
            case MPP_ENC_RC_MODE_CBR : {
//                     CBR mode has narrow bound
                mpp_enc_cfg_set_s32(cfg, "rc:bps_target", bps);
            }
                break;
            case MPP_ENC_RC_MODE_VBR :
            case MPP_ENC_RC_MODE_AVBR : {
//                     VBR mode has wide bound
                mpp_enc_cfg_set_s32(cfg, "rc:bps_max", bps_max ? bps_max : bps * 34 / 16);
                mpp_enc_cfg_set_s32(cfg, "rc:bps_min", bps_min ? bps_min : bps / 16 / 16);
            }
                break;
            default : {
//                     default use CBR mode
                mpp_enc_cfg_set_s32(cfg, "rc:bps_max", bps_max ? bps_max : bps * 17 / 16);
                mpp_enc_cfg_set_s32(cfg, "rc:bps_min", bps_min ? bps_min : bps * 15 / 16);
            }
                break;
        }
        auto type = MPP_VIDEO_CodingAVC;

        /* setup qp for different codec and rc_mode */
        switch (type) {
            case MPP_VIDEO_CodingAVC :
            case MPP_VIDEO_CodingHEVC : {
                switch (rc_mode) {
                    case MPP_ENC_RC_MODE_CBR :
                    case MPP_ENC_RC_MODE_VBR :
                    case MPP_ENC_RC_MODE_AVBR : {
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_init", -1);
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_max", 40);
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_min", 20);
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", 40);
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", 20);
                        mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 2);
                    }
                        break;
                    default : {
                    }
                        break;
                }
            }

            default : {
            }
                break;
        }


        /* setup codec  */
        mpp_enc_cfg_set_s32(cfg, "codec:type", MPP_VIDEO_CodingAVC);
        switch (type) {
            case MPP_VIDEO_CodingAVC : {
                /*
                 * H.264 profile_idc parameter
                 * 66  - Baseline profile
                 * 77  - Main profile
                 * 100 - High profile
                 */
                mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
                mpp_enc_cfg_set_s32(cfg, "h264:prefix_mode", 0);

                /*
                 * H.264 level_idc parameter
                 * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
                 * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
                 * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
                 * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
                 * 50 / 51 / 52         - 4K@30fps
                 */
                mpp_enc_cfg_set_s32(cfg, "h264:level", 41);
                mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
                mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", 0);
                mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", 1);
                mpp_enc_cfg_set_s32(cfg, "h264:stream_type", 0);

            }
                break;
            case MPP_VIDEO_CodingHEVC :
            case MPP_VIDEO_CodingMJPEG :
            case MPP_VIDEO_CodingVP8 : {
            }
                break;
            default : {
            }
                break;
        }

        auto split_mode = 0;
        auto split_arg = 20480;
        auto split_out = 1;
        mpp_enc_cfg_set_s32(cfg, "split:mode", split_mode);
        if (split_mode) {
            LOG_INFO("%p split mode %d arg %d out %d " << split_mode << "," << split_arg << "," << split_out)
            mpp_enc_cfg_set_s32(cfg, "split:arg", split_arg);
            mpp_enc_cfg_set_s32(cfg, "split:out", split_out);
        }
        // config gop_len and ref cfg
        if (IType == 1) {
            mpp_enc_cfg_set_s32(cfg, "rc:gop", 1);
        } else {
            mpp_enc_cfg_set_s32(cfg, "rc:gop", fps_out_num * 1);
//            mpp_enc_cfg_set_s32(cfg, "rc:gop", 5);
        }

        ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
        if (ret) {
            LOG_ERROR("mpi control enc set cfg failed ret " << ret);
        }

        /* if (ref)
             mpp_enc_ref_cfg_deinit(&ref);*/
        return ret;
    }


    void outThread() {
/*        std::ofstream out("test.h264", std::ios::binary);
        if (!out) {
            std::cerr << "无法打开文件" << std::endl;
            return;
        }*/

        bool readHeader = false;
        int ret = 0;
        while (working_) {
            MppPacket packet = NULL;
            mpp_packet_init_with_buffer(&packet, packageBuffer);
            /* NOTE: It is important to clear output packet length!! */
            mpp_packet_set_length(packet, 0);
//            LOG_INFO("t1");
            if (!readHeader) {
                auto ret1 = mpi->control(ctx, MPP_ENC_GET_HDR_SYNC, packet);
//                LOG_INFO("t2");
                if (ret1) {
                    LOG_ERROR("mpi control enc get extra info failed");
                } else {
                    readHeader = true;
                    /* get and write sps/pps for H.264 */
                    void *ptr = mpp_packet_get_pos(packet);
                    size_t len = mpp_packet_get_length(packet);
                    if (this->cacheHeaderDataLen_ < len) {
                        this->cacheHeaderData_.reset(new uchar[len]);
                        cacheHeaderDataLen_ = len;
                    }
                    this->currentHeaderLen_ = len;
                    memcpy(this->cacheHeaderData_.get(), (uchar *) ptr, len);
                }
            }
            ret = mpi->encode_get_packet(ctx, &packet);
            if (ret || NULL == packet) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                mpp_packet_deinit(&packet);
                continue;
            }
            auto ptr = mpp_packet_get_pos(packet);
            auto len = mpp_packet_get_length(packet);
            auto pkt_eos = mpp_packet_get_eos(packet);
            auto pts = mpp_packet_get_pts(packet);
            int type = 1;

            if (ptr != nullptr && len > 0 && ((uchar *) ptr)[4] == 0x06) {
                type = 1;
                if (this->outCB_ && currentHeaderLen_ > 0) {
                    //   out.write(reinterpret_cast<const char *>(this->cacheHeaderData_.get()), this->currentHeaderLen_);
                    this->outCB_(reinterpret_cast<const char *>(this->cacheHeaderData_.get()), this->currentHeaderLen_,
                                 0, pts);
                }
            } else {
                type = 2;
            }
            if (this->outCB_ && len > 0) {
                this->outCB_(reinterpret_cast<const char *>((uchar *) ptr), len, type, pts);
            }
            ret = mpp_packet_deinit(&packet);
            if (ret) {
                LOG_ERROR("mpp_packet_deinit fail:" << ret)
            }
        }
        LOG_INFO("useage 2:" << mpp_buffer_group_usage(bufferGroup))
        // out.close();
        LOG_INFO("output exist")
    }

    int pushYUVImage(void *data, uint16_t width, uint16_t height) {
        if (!this->working_) {
            LOG_WARN("encoder not working ")
            return -1;
        }
        MppFrame frame = NULL;
        auto ret = mpp_frame_init(&frame);
        if (ret) {
            LOG_ERROR("mpp_frame_init failed " << ret);
            return -1;
        }
        pts_++;


        mpp_frame_set_width(frame, width);
        mpp_frame_set_height(frame, height);
        mpp_frame_set_hor_stride(frame, width);
        mpp_frame_set_ver_stride(frame, std::ceil(height / 16.0) * 16);
        mpp_frame_set_fmt(frame, MPP_FMT_YUV420SP);
        mpp_frame_set_eos(frame, 1);
        mpp_frame_set_pts(frame, pts_);

        /*ret = mpp_buffer_write(bufferFrame, 0, (void *) data, width * height * 3 / 2);
        if (ret) {
            LOG_ERROR("mpp_buffer_write failed " << ret);
            return -1;
        }*/
        mpp_frame_set_buffer(frame, bufferFrame);
//        std::cout << "The address of the pointer is: " << data << std::endl;
//        LOG_INFO("ttt")
//        MppBuffer tt = (MppBuffer  ) data;
//        LOG_INFO("ttt2")

//        LOG_INFO(tt);
//        mpp_frame_set_buf_size(frame, width * height * 3 / 2);
//        LOG_INFO(" framse size :" << mpp_frame_get_buf_size(frame));
        ret = mpi->encode_put_frame(ctx, frame);
        mpp_frame_deinit(&frame);
        if (ret) {
            LOG_ERROR("put frame ret:" << ret)
            return -1;
        }
//        mpp_buffer_put(&bufferFrame);
//
//        LOG_INFO("useage:" << mpp_buffer_group_usage(bufferGroup))
        return 0;
    }


    int run(std::function<void(const char *data, size_t len, int type, size_t pts)> outcb) {
        this->outCB_ = outcb;
        this->working_ = true;
//        auto ret = mpp_buffer_get(bufferGroup, &bufferFrame, width_ * height_ * 3 / 2);

        threadOutput = std::thread([this]() {
            this->outThread();
        });
        return -1;
    }

    int stop() {
        LOG_INFO("stop1")
        this->working_ = false;
        if (threadOutput.joinable()) {
            threadOutput.join();
        }
        if (bufferFrame) {
            mpp_buffer_put(bufferFrame);
        }
        if (packageBuffer) {
            mpp_buffer_put(packageBuffer);
        }
        if (bufferGroup) {
            mpp_buffer_group_put(bufferGroup);
        }
        if (ctx) {
//            mpp_destroy(ctx);
        }
        LOG_INFO("stop3")
        return 0;
    }
//
//    void *getFramePtr() {
//        LOG_INFO(mpp_buffer_get_ptr(bufferFrame))
//        return mpp_buffer_get_ptr(bufferFrame);
//    }

private:
    MppCtx ctx;
    MppApi *mpi = NULL;
    MppFrameFormat format;
    MppBufferGroup bufferGroup = NULL;

    std::thread threadOutput;
    bool working_ = false;
    uint64_t pts_ = 0;
    MppBuffer bufferFrame = NULL;
    MppBuffer packageBuffer = NULL;

    std::function<void(const char *data, size_t len, int type, size_t pts)> outCB_;

    std::shared_ptr<uint8_t> cacheHeaderData_;
    size_t cacheHeaderDataLen_ = 0;
    size_t currentHeaderLen_ = 0;
    uint16_t width_ = 0;
    uint16_t height_ = 0;

};


