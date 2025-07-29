#ifndef _DCDN_SDK_MAIN_MANAGER_H_
#define _DCDN_SDK_MAIN_MANAGER_H_

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include "common/Config.h"
#include "util/HttpClient.h"
#include "Config.h"

NS_BEGIN(dcdn)

class WebSocketManager;
class WebRtcManager;
class FileManager;

struct MainManagerOption
{
    std::string WorkDir;
    std::string DeviceId;
    std::string ApiKey;
};

class MainManager
{
public:
    using json = nlohmann::json;
public:
    void Start();
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
    std::shared_ptr<std::thread> mThread;
    std::mutex mMtx;
    std::condition_variable mCv;

    MainManagerOption mOpt;
    Config mCfg;
    sqlite3* mCfgDB = nullptr;
    dcdn::util::HttpClient mClient;

    std::shared_ptr<WebSocketManager> mWebSkt;
    std::shared_ptr<WebRtcManager> mWebRtc;
    std::shared_ptr<FileManager> mFileMgr;
};

NS_END

#endif
