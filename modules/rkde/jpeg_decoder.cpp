//
// Created by cuibo on 2024/4/20.
//

#include "jpeg_decoder.h"
#include "im2d.hpp"
#include "logger.h"
#include "sys_utils.h"


#include <iostream>
#include <fstream>
#include <vector>

JpegDecoder::~JpegDecoder() {
    if (frame) {
        mpp_frame_deinit(&frame);
    }

    if (bufferGroupFrame) {
        mpp_buffer_group_put(bufferGroupFrame);
    }
    if (ctx) {
        mpp_destroy(ctx);
    }

}

int JpegDecoder::init(uint16_t width, uint16_t height, int fd, size_t size, void *buffer, int fd2, size_t size2,
                      void *buffer2) {
    auto ret1 = mpp_frame_init(&frame);
    if (ret1) {
        LOG_ERROR("mpp_frame_init failed");
        return -1;
    }

    MppBuffer frm_buf = NULL;
    int ret = 0;
    /*   ret = mpp_buffer_group_get_internal(&bufferGroupFrame, MPP_BUFFER_TYPE_ION);
       LOG_INFO("buffergroup:" << bufferGroupFrame)
       if (ret) {
           LOG_ERROR("mpp_buffer_group_get_internal:" << ret)
           return -1;
       }
       ret = mpp_buffer_group_limit_config(&bufferGroupFrame, width * height * 4, 25);
       if (ret) {
           LOG_ERROR("mpp_buffer_group_limit_config:" << ret)
           return -1;
       }*/
    LOG_INFO("init -1")
    ret = mpp_buffer_group_get_external(&bufferGroupFrame, MPP_BUFFER_TYPE_DMA_HEAP);
    if (ret) {
        LOG_ERROR("failed to mpp_buffer_group_get_external ret: " << ret);
        return -1;
    }
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

    ret = mpp_buffer_commit(bufferGroupFrame, &info[0]);
    ret = mpp_buffer_commit(bufferGroupFrame, &info[1]);
    if (ret) {
        LOG_ERROR("failed to mpp_buffer_commit ret  " << ret);
        return -1;
    }
    LOG_INFO("init -3")
    LOG_INFO("useag1111111e:" << mpp_buffer_group_usage(bufferGroupFrame))

    ret = mpp_buffer_get(bufferGroupFrame, &frm_buf, width * height * 4);
    if (ret) {
        LOG_ERROR("mpp_buffer_get  " << ret);
        return -1;
    }
    mpp_frame_set_buffer(frame, frm_buf);

    ret = mpp_create(&ctx, &mpi);
    if (ret) {
        LOG_ERROR("error " << ret)
        return -1;
    }
    ret = mpp_init(ctx, MPP_CTX_DEC, MPP_VIDEO_CodingMJPEG);
    if (ret) {
        LOG_ERROR("error " << ret)
        return -1;
    }
    MppDecCfg cfg = NULL;
    mpp_dec_cfg_init(&cfg);
    ret = mpi->control(ctx, MPP_DEC_GET_CFG, cfg);
    if (ret) {
        LOG_ERROR("%p failed to get decoder cfg ret %d\n" << ret);
        return -1;
    }
    ret = mpi->control(ctx, MPP_DEC_SET_CFG, cfg);
    if (ret) {
        LOG_ERROR("Failed to MPP_DEC_SET_CFG " << ret);
        return -1;
    }
    auto outFmt = MPP_FRAME_FMT_YUV;
    ret = mpi->control(ctx, MPP_DEC_SET_OUTPUT_FORMAT, &outFmt);
//    LOG_INFO(ret + 0)
    if (ret < 0) {
        LOG_ERROR("Failed to set output format " << ret);
        return -1;
    }
    LOG_INFO("useage:" << mpp_buffer_group_usage(bufferGroupFrame))
    return 0;
}

int JpegDecoder::decode(char *buffer, uint32_t bufferSize, cv::Mat &imgBGR) {
//    LOG_INFO(buffer << "  size:" << bufferSize);
    int ret = -1;
    MppBuffer pkg_buf = NULL;
    ret = mpp_buffer_get(bufferGroupFrame, &pkg_buf, bufferSize);
//    LOG_INFO("useage:" << mpp_buffer_group_usage(bufferGroupFrame))
    if (ret) {
        LOG_ERROR("failed to get buffer for input frame ret " << ret);
        return -1;
    }
    ret = mpp_buffer_write(pkg_buf, 0, buffer, bufferSize);
    if (ret) {
        LOG_ERROR("failed to mpp_buffer_write ret " << ret);
        return -1;
    }
    MppPacket packet = NULL;
    ret = mpp_packet_init_with_buffer(&packet, pkg_buf);
    if (ret) {
        LOG_ERROR("mpp_packet_init_with_buffer failed " << ret);
        return -1;
    }
//    LOG_INFO("1")
//    ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
    auto timeout = 100;
    ret = mpi->poll(ctx, MPP_PORT_INPUT, static_cast<MppPollType>(50));
    if (ret) {
        LOG_ERROR("%p mpp input poll failed " << ret);
        return -1;
    }
    MppTask task = NULL;
    ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);  /* input queue */
    if (ret) {
        LOG_ERROR("%p mpp task input dequeue failed " << ret);
        return -1;
    }
    mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
    mpp_task_meta_set_frame(task, KEY_OUTPUT_FRAME, frame);
    ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);  /* input queue */
    if (ret) {
        LOG_ERROR("%p mpp task input enqueue failed " << ret);
        return -1;
    }
    ret = mpi->poll(ctx, MPP_PORT_OUTPUT, static_cast<MppPollType>(50));
    if (ret) {
        LOG_ERROR("%p mpp output poll failed " << ret);
        return -1;
    }
    ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task); /* output queue */
    if (ret) {
        LOG_ERROR("%p mpp task output dequeue failed " << ret);
        return -1;
    }
    int success = -1;
    if (task) {
        MppFrame frame_out = NULL;
//        ret = mpp_task_meta_get_frame(task, KEY_OUTPUT_FRAME, &frame_out);
//        if (ret) {
//            LOG_ERROR("get frame error:" << ret)
//        }
        if (frame) {
            MppBuffer imageBuf = mpp_frame_get_buffer(frame);

            auto width = mpp_frame_get_width(frame);
            auto height = mpp_frame_get_height(frame);
            auto h_stride = mpp_frame_get_hor_stride(frame);
            auto v_stride = mpp_frame_get_ver_stride(frame);
//            LOG_INFO("h_stride:" << h_stride)
//            LOG_INFO("v_stride:" << v_stride)
            auto fmt = mpp_frame_get_fmt(frame);
//            LOG_INFO("fmt:" << fmt)
//            LOG_INFO(mpp_buffer_get_index(imageBuf))
//            LOG_INFO(mpp_buffer_get_offset(imageBuf))
            if (width > 0 && height > 0) {
//                LOG_INFO("t1")
//                auto *t =  mpp_buffer_get_ptr(imageBuf);
//                cb(imageBuf, width * height * 1.5);
//                LOG_INFO("t2")
                success = 0;
//                LOG_INFO(t)
//                cb(nullptr, width * height * 1.5);
//                JpegDecoder::cvtcolor(width, height, (char *) mpp_buffer_get_ptr(imageBuf),
//                                      reinterpret_cast<char *>(imgBGR.data));
            } else {
                LOG_WARN("decode error")
            }

        }
    }
    mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
    mpp_packet_deinit(&packet);
//    LOG_INFO("bufferGroupFrame useage 1:" << mpp_buffer_group_usage(bufferGroupFrame))
    mpp_buffer_put(pkg_buf);
//    LOG_INFO("bufferGroupFrame useage 2:" << mpp_buffer_group_usage(bufferGroupFrame))
    return success;
}

int JpegDecoder::cvtcolor(uint16_t width, uint16_t height, char *srcData, char *BGRadd) {
    auto src_buf_size = width * height * 3 / 2;
    auto dst_buf_size = width * height * 3;
    auto src_format = RK_FORMAT_YCbCr_420_SP;
    auto dst_format = RK_FORMAT_BGR_888;
    auto src_handle = importbuffer_virtualaddr(srcData, src_buf_size);
    auto dst_handle = importbuffer_virtualaddr(BGRadd, dst_buf_size);
    auto src_img = wrapbuffer_handle(src_handle, width, height, src_format);
    auto dst_img = wrapbuffer_handle(dst_handle, width, height, dst_format);
    auto ret44 = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret44) {
        LOG_ERROR("%d, check error! %s");
        return -1;
    }
    LOG_INFO("start cvt")
    ret44 = imcvtcolor(src_img, dst_img, src_format, dst_format);
    LOG_INFO("end cvt")
    if (ret44 == IM_STATUS_SUCCESS) {
//        auto imgyuv = cv::Mat(cv::Size(width, height), CV_8UC3, distImage.data());
//        LOG_INFO("start write")
//        cv::imwrite("test_cvr.png", imgyuv);

//        LOG_INFO("success")
    } else {
        LOG_ERROR("error " << ret44)
        return -1;
    }
    return 0;
}

int JpegDecoder::cvtcolor2(uint16_t width, uint16_t height, char *srcData, char *BGRadd) {
    auto src_buf_size = width * height * 3;
    auto dst_buf_size = width * height * 1.5;
    auto src_format = RK_FORMAT_BGR_888;
    auto dst_format = RK_FORMAT_YCbCr_420_SP;
//    auto dst_format = RK_FORMAT_YCrCb_420_SP;
    auto src_handle = importbuffer_virtualaddr(srcData, src_buf_size);
    auto dst_handle = importbuffer_virtualaddr(BGRadd, dst_buf_size);
    auto src_img = wrapbuffer_handle(src_handle, width, height, src_format);
    auto dst_img = wrapbuffer_handle(dst_handle, width, height, dst_format);
    auto ret44 = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret44) {
        LOG_ERROR("%d, check error! %s");
        return -1;
    }
//    LOG_INFO("start cvt")
    ret44 = imcvtcolor(src_img, dst_img, src_format, dst_format);
//    LOG_INFO("end cvt")
    if (ret44 == IM_STATUS_SUCCESS) {
    } else {
        LOG_ERROR("error " << ret44)
        return -1;
    }
    return 0;
}

int JpegDecoder::bgr2yuv(uint16_t width, uint16_t height, char *srcData, char *BGRadd) {
    auto src_buf_size = width * height * 3;
    auto dst_buf_size = width * height * 1.5;
    auto src_format = RK_FORMAT_BGR_888;
//    auto dst_format = RK_FORMAT_YCbCr_420_SP;
    auto dst_format = RK_FORMAT_YCrCb_420_SP;

    auto src_handle = importbuffer_virtualaddr(srcData, src_buf_size);
    auto dst_handle = importbuffer_virtualaddr(BGRadd, dst_buf_size);

    auto src_img = wrapbuffer_handle(src_handle, width, height, src_format);
    auto dst_img = wrapbuffer_handle(dst_handle, width, height, dst_format);
    auto ret44 = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret44) {
        LOG_ERROR("%d, check error! %s");
        return -1;
    }
    ret44 = imcvtcolor(src_img, dst_img, src_format, dst_format);
    if (ret44 == IM_STATUS_SUCCESS) {

    } else {
        LOG_ERROR("error " << ret44)
        return -1;
    }
    return 0;
}

int
JpegDecoder::crop2(char *srcData, char *dstdata, uint16_t width, uint16_t height, uint16_t x, uint16_t y,
                   uint16_t width2,
                   uint16_t height2) {

    auto src_buf_size = width * height * 4;
    auto dst_buf_size = width2 * height2 * 4;

//    LOG_INFO(src_buf_size)

//    LOG_INFO(dst_buf_size)

    auto src_handle = importbuffer_virtualaddr(srcData, src_buf_size);
    auto dst_handle = importbuffer_virtualaddr(dstdata, dst_buf_size);

    LOG_INFO(src_handle)
    LOG_INFO(dst_handle)
//    auto src_img = wrapbuffer_virtualaddr_t(srcData, width, height, width, height, RK_FORMAT_BGR_888);
//    auto dst_img = wrapbuffer_virtualaddr_t(dstdata, width2, width2, width2, height, RK_FORMAT_BGR_888);
    auto src_img = wrapbuffer_handle(src_handle, width, height, RK_FORMAT_BGRA_8888);
    auto dst_img = wrapbuffer_handle(dst_handle, width2, height2, RK_FORMAT_BGRA_8888);
    im_rect rectSrc;
    rectSrc.x = 0;
    rectSrc.y = 0;
    rectSrc.width = width;
    rectSrc.height = height;
    im_rect rectDst;
    rectDst.x = 0;
    rectDst.y = 0;
    rectDst.width = width;
    rectDst.height = height2;
    /* IM_STATUS ret44 = imcheck(src_img, dst_img, rectSrc, rectDst);
     if (IM_STATUS_NOERROR != ret44) {
         LOG_ERROR("%d, check error! " << ret44);
         return -1;
     }*/
    im_rect rect1;
    rect1.x = x;
    rect1.y = y;
    rect1.width = width2;
    rect1.height = height2;
    auto ret45 = imcrop(src_img, dst_img, rect1);
    if (ret45 == IM_STATUS_SUCCESS) {
    } else {
        LOG_ERROR("error " << ret45)
        return -1;
    }
    return 0;

}

int JpegDecoder::crop(const cv::Mat &src, cv::Mat &dst, cv::Rect rect) {
    auto src_buf_size = src.cols * src.rows * 3;
    auto dst_buf_size = dst.cols * dst.rows * 3;

    LOG_INFO(src_buf_size)
    LOG_INFO(dst_buf_size)

    auto src_handle = importbuffer_virtualaddr(src.data, src_buf_size);
    auto dst_handle = importbuffer_virtualaddr(dst.data, dst_buf_size);

    auto src_img = wrapbuffer_handle(src_handle, src.cols, src.rows, RK_FORMAT_BGR_888);
    auto dst_img = wrapbuffer_handle(dst_handle, dst.cols, dst.rows, RK_FORMAT_BGR_888);

    auto ret44 = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret44) {
        LOG_ERROR("%d, check error! " << ret44);
        return -1;
    }
    im_rect rect1;
    rect1.x = rect.x;
    rect1.y = rect.y;
    rect1.width = rect.width;
    rect1.height = rect.height;
    ret44 = imcrop(src_img, dst_img, rect1);
    if (ret44 == IM_STATUS_SUCCESS) {

    } else {
        LOG_ERROR("error " << ret44)
        return -1;
    }
    return 0;
}

int JpegDecoder::resize(char *srcData, char *dstdata, uint16_t width, uint16_t height, float scaleX, float scaleY) {
    int smallWidth = width * scaleX;
    int smallHeight = height * scaleY;
    auto buf_size = width * height * 3;
    auto small_buf_size = smallWidth * smallHeight * 3;

    auto src_handle = importbuffer_virtualaddr(srcData, buf_size);
    auto dst_handle = importbuffer_virtualaddr(dstdata, small_buf_size);
    auto src_img = wrapbuffer_handle(src_handle, width, height, RK_FORMAT_BGR_888);
    auto dst_img = wrapbuffer_handle(dst_handle, smallWidth, smallHeight, RK_FORMAT_BGR_888);
    /* auto ret44 = imcheck(src_img, dst_img, {}, {});
     if (IM_STATUS_NOERROR != ret44) {
         LOG_ERROR("%d, check error! " << ret44);
         return -1;
     }*/
    auto ret = imresize(src_img, dst_img, scaleX, scaleY, INTER_LINEAR, 1);
    if (ret == IM_STATUS_SUCCESS) {
//        LOG_INFO("success")
    } else {
        return -1;
    }
    return 0;
}
