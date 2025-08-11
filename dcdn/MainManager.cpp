#include "MainManager.h"

#include <plog/Initializers/RollingFileInitializer.h>

#include <filesystem>

#include "util/HttpDownloader.h"

#include "DownloadManager.h"
#include "FileManager.h"
#include "UploadManager.h"
#include "WebRtcManager.h"
#include "WebSocketManager.h"

NS_BEGIN(dcdn)

std::atomic<MainManager*> MainManager::singlet = nullptr;

int MainManager::Init(const MainManagerOption& opt)
{
    std::filesystem::path logFile(opt.WorkDir);
    logFile.append("dcdn.log");
    plog::init<DCDN_LOGGER_ID>(plog::debug, logFile.c_str());
    logInfo << "MainManager init";
    rtc::InitLogger(rtc::LogLevel::Debug);
    MainManager* n = nullptr;
    MainManager* m = new MainManager();
    if (!singlet.compare_exchange_strong(n, m)) {
        delete m;
        return 0;
    }
    return m->init(opt);
}

MainManager::MainManager(): BaseManager(this)
{
    mHttpDownloader= std::make_shared<util::HttpDownloader>();
    mApiClient = std::make_shared<ApiClient>(mHttpDownloader.get());

    mWebSkt = std::make_shared<WebSocketManager>(this);
    mWebRtc = std::make_shared<WebRtcManager>(this);
    mFileMgr = std::make_shared<FileManager>(this);
    mUploadMgr = std::make_shared<UploadManager>(this);
    mDownloadMgr = std::make_shared<DownloadManager>(this);

    registerHandler(EventType::AsyncApiRequest, &MainManager::handleAsyncApiRequestEvent);
    registerHandler(EventType::UploadMsg, &MainManager::handleUploadMsgEvent);
    registerHandler(EventType::DeployMsg, &MainManager::handleDeployMsgEvent);
}

int MainManager::init(const MainManagerOption& opt)
{
    mOpt = opt;
    int ret = mCfg.CreateTable(opt.WorkDir);
    if (ret != ErrorCodeOk) {
        logError << "init config fail";
        return ret;
    }
    ret = mHttpDownloader->Init(nullptr);
    if (ret != ErrorCodeOk) {
        logError << "init HttpDownloader fail";
        return ret;
    }
    mCfg.LoadFromDB();
    auto peerId = mCfg.PeerId();
    if (peerId.empty()) {
        // TODO: generate PeerId
    }
    return ErrorCodeOk;
}

MainManager::~MainManager() {}

long MainManager::ApiPost(util::HttpClient& cli, const char* uri, json& arg, util::HttpResponse* resp)
{
    std::string url = mCfg.ApiRootUrl();
    url += uri;
    util::HttpRequest req(url.c_str(), arg.dump(), "application/json");
    long ret = cli.Do(req, resp);
    return ret;
}

long MainManager::ApiPost(util::HttpClient& cli, const char* uri, json& arg, json& result)
{
    util::HttpResponse resp;
    long code = ApiPost(cli, uri, arg, &resp);
    if (code == 200) {
        auto r = json::parse(resp.Body());
        code = r["code"];
        auto it = r.find("data");
        if (it != r.end()) {
            result = std::move(*it);
        } else {
            result.clear();
        }
        return code;
    }
    return -1;
}

void MainManager::login()
{
    try {
        json msg;
        msg["device_id"] = mOpt.DeviceId;
        msg["apikey"] = mOpt.ApiKey;


        mApiClient->Post("/api/v1/login", msg, this, nullptr, nullptr);

        json data;
        long code = ApiPost(mClient, "/api/v1/login", msg, data);
        if (code > 0) {
            std::string peerId = data["peer_id"];
            mCfg.SetPeerId(peerId);
            std::string token = data["token"];
            mCfg.SetToken(token);
        } else {
            logWarn << "login request fail code: " << code;
        }
    } catch (std::exception& excp) {
        logWarn << "login exception: " << excp.what();
    } catch (...) {
        logWarn << "login unknown exception";
    }
}

void MainManager::run()
{
    logInfo << "MainManager running";
    mHttpDownloader->Start();
    login();
    mWebRtc->Start();
    mWebSkt->Start();
    mFileMgr->Start();
    mUploadMgr->Start();
    mDownloadMgr->Start();
    while (true) {
        waitAllEvents(std::chrono::milliseconds(1000));
    }
    logInfo << "MainManager exit";
}

void MainManager::handleAsyncApiRequestEvent(std::shared_ptr<Event> evt)
{
    mApiClient->HandleAsyncApiRequestEvent(evt);
}

void MainManager::handleUploadMsgEvent(std::shared_ptr<Event> evt)
{
    if (mUploadMgr) {
        static_cast<UploadManager*>(mUploadMgr.get())->PostEvent(evt);
    }
}

void MainManager::handleDeployMsgEvent(std::shared_ptr<Event> evt)
{
    if (mDownloadMgr) {
        static_cast<DownloadManager*>(mDownloadMgr.get())->PostEvent(evt);
    }
}

NS_END
