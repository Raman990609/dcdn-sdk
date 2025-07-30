#ifndef _DCDN_SDK_MAIN_MANAGER_H_
#define _DCDN_SDK_MAIN_MANAGER_H_

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "util/HttpClient.h"
#include "Config.h"
#include "BaseManager.h"

NS_BEGIN(dcdn)

struct MainManagerOption
{
    std::string WorkDir;
    std::string DeviceId;
    std::string ApiKey;
};

class MainManager: public BaseManager
{
public:
    using json = nlohmann::json;
public:
    void PostWebSocketMsg(std::shared_ptr<json> msg);
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

    void loadConfigFromDB();
    static int loadConfigCallback(void* , int argc, char** argv, char** colName);
    void login();
    void updateConfigToDB();
private:
    std::mutex mMtx;
    std::condition_variable mCv;

    MainManagerOption mOpt;
    Config mCfg;
    sqlite3* mCfgDB = nullptr;
    dcdn::util::HttpClient mClient;

    std::shared_ptr<BaseManager> mWebSkt;
    std::shared_ptr<BaseManager> mWebRtc;
    std::shared_ptr<BaseManager> mFileMgr;
    std::shared_ptr<BaseManager> mUploadMgr;
    std::shared_ptr<BaseManager> mDownloadMgr;
};

NS_END

#endif
