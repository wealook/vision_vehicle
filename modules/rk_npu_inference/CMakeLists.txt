
project(rk_npu_inference CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
include_directories(
        ./rknpu2/include
        ${librga_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ../CommonUtils
        ../dma_alloc
        ../../3rdparty/librga/include

        ${CMAKE_CURRENT_SOURCE_DIR}
)
add_library(${PROJECT_NAME} STATIC
        common.h
        file_utils.h
        file_utils.c
        image_utils.c
        image_utils.h
        #        ppyoloe/postprocess.h
        #        ppyoloe/postprocess.cc
        #        ppyoloe/inference.cpp
        #        ppyoloe/inference.h
        #
        yolov6/postprocess.h
        yolov6/postprocess.cc
        #        yolov6/inference.cc
        #        yolov6/inference.cc
        rk_detection.h
        rk_detection.cpp
        rk_detection_postprocess.h
)
target_link_libraries(${PROJECT_NAME}
        librknnrt.so
        ${OpenCV_LIBRARIES}
        CommonUtils
        ${3RDPARTY_ROOT_DIR}/librga/libs/Linux/gcc-aarch64/librga.a
        dma_alloc
)
