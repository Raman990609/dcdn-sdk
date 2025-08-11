#ifndef _DCDN_SDK_MAIN_MANAGER_H_
#define _DCDN_SDK_MAIN_MANAGER_H_

#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "BaseManager.h"
#include "Config.h"
#include "EventLoop.h"
#include "util/HttpClient.h"
#include "ApiClient.h"

NS_BEGIN(dcdn)

struct MainManagerOption
{
    std::string WorkDir;
    std::string DeviceId;
    std::string ApiKey;
};

class MainManager: public BaseManager, public EventLoop<MainManager>
{
public:
    using json = nlohmann::json;

public:
    const MainManagerOption& Option() const
    {
        return mOpt;
    }
    const Config& Cfg() const
    {
        return mCfg;
    }
    long ApiPost(util::HttpClient& cli, const char* uri, json& arg, util::HttpResponse* resp);
    long ApiPost(util::HttpClient& cli, const char* uri, json& arg, json& result);

private:
    void run();

public:
    static int Init(const MainManagerOption& opt);
    static MainManager* Singlet()
    {
        return singlet;
    }

private:
    MainManager();
    MainManager(const MainManager&) = delete;
    MainManager& operator=(const MainManager&) = delete;
    ~MainManager();
    int init(const MainManagerOption& opt);
    static std::atomic<MainManager*> singlet;

    void login();

    void handleAsyncApiRequestEvent(std::shared_ptr<Event> evt);
    void handleUploadMsgEvent(std::shared_ptr<Event> evt);
    void handleDeployMsgEvent(std::shared_ptr<Event> evt);

private:
    std::mutex mMtx;
    std::condition_variable mCv;

    MainManagerOption mOpt;
    Config mCfg;

    util::HttpClient mClient;
    std::shared_ptr<util::HttpDownloader> mHttpDownloader;
    std::shared_ptr<ApiClient> mApiClient;

    std::shared_ptr<BaseManager> mWebSkt;
    std::shared_ptr<BaseManager> mWebRtc;
    std::shared_ptr<BaseManager> mFileMgr;
    std::shared_ptr<BaseManager> mUploadMgr;
    std::shared_ptr<BaseManager> mDownloadMgr;
};

NS_END

#endif
