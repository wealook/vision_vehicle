project(rkde)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(
        ../CommonUtils
        ${rkmpp_INCLUDE_DIRS}/rockchip
        ../../3rdparty/rkmpp/inc
        ../../3rdparty/librga/include
        ${OpenCV_INCLUDE_DIRS}
        ${librga_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)


add_library(${PROJECT_NAME} STATIC
        jpeg_decoder.cpp
        jpeg_decoder.h
        h264_encoder.cpp
        h264_encoder.h

)


target_link_libraries(${PROJECT_NAME} PRIVATE
        CommonUtils
        ${3RDPARTY_ROOT_DIR}/librga/libs/Linux/gcc-aarch64/librga.a
        librockchip_mpp.so
)

