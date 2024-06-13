#pragma  once

#include <memory>
#include <thread>
#include "camera_provider.h"
#include "general_setting.h"
#include "httplib.h"

namespace ve {
    class Process {
    public:
        Process();

//        ~Process() {
//        }


        void start(const ve::GeneralSetting &generalSet);

        void run();

        void runAsync();

        void stop();

//        static Process &instance() {
//            static Process process;
//            return process;
//        }

    private:
        std::unique_ptr<ve::CameraProvider> cameraProvider_;
//        std::unique_ptr<std::thread> startCameraThread_;
        bool working_ = false;
//        bool debug_ = false;
        std::shared_ptr<httplib::Server> httpServer_;
        std::shared_ptr<std::thread> httpThread_;
    };
}


