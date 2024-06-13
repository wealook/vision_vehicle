cmake_minimum_required(VERSION 3.5.0)
project(VisionVehicle LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif ()
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(OpenSSL REQUIRED)
# 2. link Paddle-Lite directory
add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
find_package(OpenSSL REQUIRED)
set(OpenSSL_INCLUDE_DIRS "${OPENSSL_ROOT_DIR}/include")
set(OpenSSL_LIBRARY_DIRS "${OPENSSL_ROOT_DIR}/lib")
include_directories(
        ${OpenSSL_INCLUDE_DIRS}
)


include_directories(
        ../../3rdparty/rkmpp/inc
        ../../3rdparty/librga/include
        ../../modules/CameraCapture
        ../../modules/CommonUtils
        ../../modules/detection_lite/include
        ../../modules/rkde
        ../../modules/rk_npu_inference
        ../../modules/rk_npu_inference/rknpu2/include
        ../../modules/mp4v2/include
        ${OpenCV_INCLUDE_DIRS}
        ${jsoncpp_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${jsoncpp_INCLUDE_DIR}
        ${rkmpp_INCLUDE_DIRS}/rockchip
        ${librga_INCLUDE_DIRS}
        ../../modules/dma_alloc
        ../../modules/httplib
)

list(APPEND COMTEST_SRC_FILES
        main.cpp
        vehicle_process.cpp
        vehicle_process.h
        general_setting.cpp
        general_setting.h
        runtime_setting.cpp
        runtime_setting.h
        camera_provider.h
        camera_provider.cpp
        detection_service.cpp
        detection_service.h
        comm_service.cpp
        comm_service.h
        video_saver.cpp
        video_saver.h
        vision_provider.cpp
        vision_provider.h
        ../../modules/httplib/httplib.h
        ../../modules/httplib/httplib.cc
)

add_executable(${PROJECT_NAME} ${COMTEST_SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        CommonUtils
        CameraCapture
        #        httplib
        ${SYSTEM_LIBS}
        opencv_core
        yaml-cpp
        rkde
        ${jsoncpp_STATIC_LIBS}
        rk_npu_inference
        ${3RDPARTY_ROOT_DIR}/librga/libs/Linux/gcc-aarch64/librga.a
        dma_alloc
        mp4v2
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
)
list(APPEND _FILES
        ./setting.yaml
        ./model_det/model.rknn
        ../../3rdparty/rknpu2/Linux/aarch64/librknnrt.so
)
install(
        FILES
        ${_FILES}
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
