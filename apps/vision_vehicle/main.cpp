//
// Created by cuibo7 on 2024/2/23.
//
#include "logger.h"
#include "vehicle_process.h"
#include "sys_utils.h"
//#include <opencv2/core/utils/logger.hpp>

#if !defined(_WINDOWS)

#include <signal.h>

#endif


bool exiting_ = false;
//extern int licenseCounter_;

void signal_handler_fun(int signum) {
    exiting_ = true;
    LOG_INFO("catch signal " << signum);
}

int main() {
#if !defined(_WINDOWS)
    signal(SIGINT, signal_handler_fun);
    signal(SIGTERM, signal_handler_fun);
    signal(SIGKILL, signal_handler_fun);
    signal(SIGQUIT, signal_handler_fun);
#endif
//    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    wl::Logger::instance().init("vv", static_cast<wl::Logger::LogTarget>( static_cast<int>(wl::Logger::LogTarget::eLogFile) | static_cast<int>( wl::Logger::LogTarget::eLogSys)),
                                wl::Logger::LogLevel::eLogInfo, "vv");
    ve::GeneralSetting::instance().load(wl::getCurrFilePath() + "/setting.yaml");
    ve::Process process;
    process.start(ve::GeneralSetting::instance());
    while (!exiting_) {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } catch (...) {
            std::cout << "sleep error";
        }
    }
    LOG_INFO("receive exit signal!!!");
    process.stop();
    LOG_INFO("exist success!!!");
#if !defined(_WINDOWS)
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    exit(EXIT_SUCCESS);
#endif
    return 0;
}