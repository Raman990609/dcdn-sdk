#include "WebRtcManager.h"
#include "MainManager.h"

NS_BEGIN(dcdn)

WebRtcManager::WebRtcManager(MainManager* man):
    mMan(man)
{
    mCert = generate_ecdsa_certificate();
}

void WebRtcManager::Start()
{
    mThread = std::make_shared<std::thread>([&](){
        run();
    });
    mThread->detach();
}

void WebRtcManager::run()
{
    spdlog::info("WebRtc running");
    while (true) {
        runGather();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    spdlog::info("WebRtc exit");
}

void WebRtcManager::runGather()
{
    switch (mGatherStatus) {
    case GatherIdle:
        gather();
        break;
    case GatherRunning:
        break;
    case GatherDone:
        gatherDone();
        break;
    }
}

void WebRtcManager::gather()
{
    try {
        mGatherStatus = GatherRunning;
        rtc::Configuration config;
        config.enableIceUdpMux = true;
        config.certificatePemFile = mCert.certPem;
        config.keyPemFile = mCert.keyPem;
        auto ss = mMan->Cfg().StunServers();
        for (auto s : ss) {
            auto idx = s.find(':');
            if (idx == std::string::npos) {
                continue;
            }
            std::string host = s.substr(0, idx);
            int port = atoi(s.c_str() + idx + 1);
            if (port > 0 && port < 65536) {
                rtc::IceServer serv(host, port);
                serv.type = rtc::IceServer::Type::Stun;
                config.iceServers.push_back(serv);
            }
        }
        auto pc = std::make_shared<rtc::PeerConnection>(config);
        pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
            if (state == rtc::PeerConnection::GatheringState::Complete) {
                mGatherStatus = GatherDone;
            }
        });
        auto dc = pc->createDataChannel("detect");//start gathering
        mPc = pc;
    } catch (std::exception& excp) {
        spdlog::warn("webrtc gather excpetion: {}", excp.what());
    } catch (...) {
        spdlog::warn("webrtc gather unknown excpetion");
    }

}

void WebRtcManager::gatherDone()
{
    try {
        auto dsOpt = mPc->localDescription();
        if (dsOpt) {
            std::string sdp(dsOpt.value());
            mSdp = sdp;
        }
    } catch (std::exception& excp) {
    } catch (...) {
    }
}

void WebRtcManager::report()
{
    try {
        json msg;
        msg["description"] = mSdp;
        mMan->ApiPost(mClient, "/api/v1/report_net_info", msg, nullptr);
    } catch (std::exception& excp) {
    } catch (...) {
    }
}

NS_END
