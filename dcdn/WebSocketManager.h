#ifndef _DCDN_WEBSOCKET_MANAGER_H_
#define _DCDN_WEBSOCKET_MANAGER_H_

#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <list>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>
#include "BaseManager.h"
#include "EventLoop.h"

NS_BEGIN(dcdn)

class WebSocketManager: public BaseManager, public EventLoop<WebSocketManager>
{
public:
    using json = nlohmann::json;
public:
    WebSocketManager(MainManager* man);
private:
    void run();
    void connect();
    void handleRecvMsg(std::variant<rtc::binary, std::string>& msg);
    void handleAckMsgEvent(std::shared_ptr<Event> evt);
private:
    enum WebSktStatus
    {
        Idle,
        Connecting,
        Connected,
        Closed,
    };
private:
    std::mutex mMtx;
    std::condition_variable mCv;
    std::atomic<int> mStatus;
    std::shared_ptr<rtc::WebSocket> mWebSkt;
};

NS_END

#endif
