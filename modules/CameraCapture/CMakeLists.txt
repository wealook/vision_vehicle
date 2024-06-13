find_package(OpenCV REQUIRED)

include_directories(
        ../CommonUtils
        drivers
        .
        ${CMAKE_CURRENT_BINARY_DIR}
)

list(APPEND CAMERA_SOURCEFILES
        camera.cpp
        camera.h
        camera_driver.h
        easy_camera.hpp
        camera_driver.cpp
        capture_callback.h
        drivers/general_usb_camera.cpp
        drivers/general_usb_camera.h
)

add_library(CameraCapture STATIC ${CAMERA_SOURCEFILES})
target_link_libraries(CameraCapture PUBLIC CommonUtils   opencv_core)
set_target_properties(CameraCapture PROPERTIES FOLDER "common")

