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
#include "common/Config.h"
#include "BaseManager.h"

NS_BEGIN(dcdn)

class WebSocketManager: public BaseManager
{
public:
    using json = nlohmann::json;
public:
    WebSocketManager(MainManager* man);
    unsigned long PostMsg(std::shared_ptr<json> msg);
private:
    void run();
    void connect();
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
