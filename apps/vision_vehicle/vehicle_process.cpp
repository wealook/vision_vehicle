#include "vehicle_process.h"

#include <memory>
#include "camera_provider.h"
#include "json/json.h"
#include "runtime_setting.h"

namespace ve {

    Process::Process() = default;


    void Process::start(const ve::GeneralSetting &generalSet) {
        httpServer_ = std::make_shared<httplib::Server>();
        httpServer_->Options("/api/runtime_setting", [](const httplib::Request &, httplib::Response &res) {

            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "*");
            res.set_header("Access-Control-Allow-Methods", "*");
            res.set_header("Access-Control-Allow-Credentials", "true");
        });
        httpServer_->Get("/api/runtime_setting", [](const httplib::Request &, httplib::Response &res) {
            Json::Value values;
            values["rtc_push"] = RuntimeSetting::instance().getRtcPush();
            values["push_type"] = RuntimeSetting::instance().getPushType();
            values["detection"] = RuntimeSetting::instance().getDetection();
            Json::Value response;
            response["result"] = values;
            response["state"] = 10000;
            response["msg"] = "";
            Json::FastWriter fast_writer;
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "*");
            res.set_header("Access-Control-Allow-Methods", "*");
            res.set_header("Access-Control-Allow-Credentials", "true");
            res.set_content(fast_writer.write(response), "application/json");
        });

        httpServer_->Post("/api/runtime_setting", [](const httplib::Request &request, httplib::Response &res) {
            Json::Value response;
            Json::Value values;
            response["result"] = values;
            response["state"] = 10000;
            response["msg"] = "";
            Json::FastWriter fast_writer;

            auto contentType = request.get_header_value("Content-Type");
            if ("application/json" != contentType) {
                LOG_WARN("contentType error:" << contentType)
                response["state"] = 10002;
                res.set_content(fast_writer.write(response), "application/json");
                return;
            }
            Json::Reader reader;
            Json::Value params;
            if (!reader.parse(request.body, params)) {
                LOG_ERROR("parse json body fail:" << ",body" << request.body)
                response["state"] = 10003;
                res.set_content(fast_writer.write(response), "application/json");
                return;
            }

            if (!params["detection"].empty() && params["detection"].isBool()) {
                RuntimeSetting::instance().setDetection(params["detection"].asBool());
            }

            if (!params["rtc_push"].empty() && params["rtc_push"].isBool()) {
                RuntimeSetting::instance().setRtcPush(params["rtc_push"].asBool());
            }

            if (!params["push_type"].empty() && params["push_type"].isInt()) {
                RuntimeSetting::instance().setPushType(params["push_type"].asInt());
            }
            response["state"] = 10000;
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "*");
            res.set_header("Access-Control-Allow-Methods", "*");
            res.set_header("Access-Control-Allow-Credentials", "true");
            res.set_content(fast_writer.write(response), "application/json");

        });
        httpServer_->set_mount_point("/", "./www");
        httpThread_ = std::make_shared<std::thread>([this]() {
            httpServer_->listen("0.0.0.0", 50443);
        });

        this->cameraProvider_ = std::make_unique<ve::CameraProvider>();
        this->cameraProvider_->start();
    }

    void Process::run() {

        // 系统信息
        // 温度
        //  cpu占用
    }

    void Process::runAsync() {

    }

    void Process::stop() {
        if (this->httpServer_) {
            this->httpServer_->stop();
        }
        if (this->httpThread_->joinable()) {
            this->httpThread_->join();
        }
        this->cameraProvider_->stop();
    }
}
