#ifndef _DCDN_WEBRTC_MANAGER_H_
#define _DCDN_WEBRTC_MANAGER_H_

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <spdlog/spdlog.h>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include "common/Config.h"
#include "util/HttpClient.h"
#include "Cert.h"

NS_BEGIN(dcdn)

class MainManager;

class WebRtcManager
{
public:
    using json = nlohmann::json;
public:
    WebRtcManager(MainManager* man);
    void Start();
private:
    void run();
    void runGather();
    void gather();
    void gatherDone();
    void report();
    enum GatherStatus
    {
        GatherIdle,
        GatherRunning,
        GatherDone,
    };
private:
    MainManager* mMan;
    std::shared_ptr<std::thread> mThread;
    std::mutex mMtx;
    std::condition_variable mCv;

    dcdn::util::HttpClient mClient;
    CertificatePair mCert;
    std::atomic<GatherStatus> mGatherStatus = GatherIdle;
    std::shared_ptr<rtc::PeerConnection> mPc;
    std::string mSdp;
};

NS_END

#endif
