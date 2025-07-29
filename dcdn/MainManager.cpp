#include <filesystem>
#include <spdlog/spdlog.h>
#include "MainManager.h"
#include "WebSocketManager.h"
#include "WebRtcManager.h"
#include "FileManager.h"

NS_BEGIN(dcdn)

std::atomic<MainManager*> MainManager::singlet = nullptr;

int MainManager::Init(const MainManagerOption& opt)
{
    MainManager* n = nullptr;
    MainManager* m = new MainManager();
    if (!singlet.compare_exchange_strong(n, m)) {
        delete m;
        return 0;
    }
    return m->init(opt);
}

MainManager::MainManager()
{
    mWebSkt = std::make_shared<WebSocketManager>(this);
    mWebRtc = std::make_shared<WebRtcManager>(this);
    mFileMgr = std::make_shared<FileManager>(this);
}

int MainManager::init(const MainManagerOption& opt)
{
    std::filesystem::path cfgFile(opt.WorkDir);
    cfgFile.append("config.db");
    sqlite3 *db;
    int rc = sqlite3_open(cfgFile.c_str(), &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }
    rc = mCfg.CreateTable(db);
    if (rc < 0) {
        sqlite3_close(db);
        return -1;
    }
    mOpt = opt;
    mCfgDB = db;
    return 1;
}

MainManager::~MainManager()
{
    if (mCfgDB) {
        sqlite3_close(mCfgDB);
    }
}

void MainManager::Start()
{
    mThread = std::make_shared<std::thread>([&](){
        run();
    });
    mThread->detach();
}

void MainManager::PostWebSocketMsg(std::shared_ptr<json> msg)
{
}

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

void MainManager::loadConfigFromDB()
{
    mCfg.LoadFromDB(mCfgDB);
}

void MainManager::login()
{
    try {
        spdlog::debug("login");
        json msg;
        msg["device_id"] = mOpt.DeviceId;
        msg["apikey"] = mOpt.ApiKey;
        json data;
        long code = ApiPost(mClient, "/api/v1/login", msg, data);
        if (code > 0) {
            std::string peerId = data["peer_id"];
            mCfg.SetPeerId(peerId);
            std::string token = data["token"];
            mCfg.SetToken(token);
        } else {
            spdlog::warn("login request fail code: {}", code);
        }
    } catch (std::exception& excp) {
        spdlog::warn("login exception: {}", excp.what());
    } catch (...) {
        spdlog::warn("login unknown exception");
    }
}

void MainManager::run()
{
    spdlog::info("MainManager running");
    mCfg.LoadFromDB(mCfgDB);
    login();
    mWebRtc->Start();
    mWebSkt->Start();
    mFileMgr->Start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    spdlog::info("MainManager exit");
}

NS_END
