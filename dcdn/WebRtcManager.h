#ifndef _DCDN_WEBRTC_MANAGER_H_
#define _DCDN_WEBRTC_MANAGER_H_

#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include "common/Config.h"
#include "util/HttpClient.h"
#include "Cert.h"
#include "BaseManager.h"

NS_BEGIN(dcdn)

class WebRtcManager: public BaseManager
{
public:
    using json = nlohmann::json;
public:
    WebRtcManager(MainManager* man);
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
    std::mutex mMtx;
    std::condition_variable mCv;

    dcdn::util::HttpClient mClient;
    CertificatePair mCert;
    std::chrono::time_point<std::chrono::steady_clock> mLastGatherTime;
    std::atomic<GatherStatus> mGatherStatus = GatherIdle;
    std::shared_ptr<rtc::PeerConnection> mPc;
    std::string mSdp;
};

NS_END

#endif
