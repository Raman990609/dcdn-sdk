#include <cstdlib>
#include <chrono>
#include <iostream>
#include "MainManager.h"
#include "WebSocketManager.h"

using namespace std::chrono_literals;
using json = nlohmann::json;

NS_BEGIN(dcdn)

WebSocketManager::WebSocketManager(MainManager* man):
    BaseManager(man)
{
}

void WebSocketManager::run()
{
    spdlog::info("WebSocket running");
    auto connectTime = std::chrono::steady_clock::now();
    std::atomic<bool> closed = true;
    while (true) {
        switch (mStatus) {
        case Idle:
            connectTime = std::chrono::steady_clock::now();
            connect();
            break;
        case Connecting:
            if (std::chrono::steady_clock::now() - connectTime >= std::chrono::seconds(30)) {
                mStatus = Idle;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            break;
        case Connected:
            if (!mWebSkt || !mWebSkt->isOpen()) {
                mStatus = Closed;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            break;
        case Closed:
            {
                int secs = rand() % 30 + 1;
                std::this_thread::sleep_for(std::chrono::seconds(secs));
                mStatus = Idle;
            }
            break;
        }
    }
    spdlog::info("WebSocket exit");
}

void WebSocketManager::connect()
{
    try {
        rtc::WebSocket::Configuration config;
        config.disableTlsVerification = true;
        auto ws = std::make_shared<rtc::WebSocket>(std::move(config));
        ws->onOpen([&]() {
            spdlog::info("websocket opened");
            mStatus = Connected;
        });
        ws->onError([&](std::string error) {
            spdlog::info("websocket error: {}", error);
            mStatus = Closed;
        });
        ws->onClosed([&]() {
            spdlog::info("websocket closed");
            mStatus = Closed;
        });
        ws->onMessage([&](std::variant<rtc::binary, std::string> message) {
            if (!std::holds_alternative<std::string>(message)) {
                return;
            }
            std::string str = std::move(std::get<std::string>(message));
            try {
                if (mMan) {
                    json msg = json::parse(str);
                    mMan->PostWebSocketMsg(std::make_shared<json>(std::move(msg)));
                }
            } catch (std::exception& excp) {
                spdlog::warn("websocket parse msg exception: {}", excp.what());
            } catch (...) {
                spdlog::warn("websocket parse msg unknown exception");
            }
        });
        mStatus = Connecting;
        mWebSkt = ws;
        std::string url = mMan->Cfg().ApiRootUrl();
        url += "/ws";
        spdlog::info("websocket try to connect: {}", url);
        mWebSkt->open(url);
    } catch (std::exception& excp) {
        spdlog::warn("websocket exception: {}", excp.what());
        mStatus = Closed;
    } catch (...) {
        spdlog::warn("websocket unknown exception");
        mStatus = Closed;
    }
}

NS_END
