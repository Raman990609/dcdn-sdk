#ifndef _DCDN_SDK_CONFIG_H_
#define _DCDN_SDK_CONFIG_H_

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <sqlite3.h>
#include "common/Config.h"
#include "common/Logger.h"


NS_BEGIN(dcdn)

class Config
{
public:
    Config();
    void LoadFromDB(sqlite3* db);
    int SaveToDB(sqlite3* db);

    std::string ApiRootUrl() const
    {
        std::unique_lock<std::mutex> lck(mMtx);
        auto val = mApiRootUrl;
        return val;
    }
    std::string PeerId() const
    {
        std::unique_lock<std::mutex> lck(mMtx);
        auto val = mPeerId;
        return val;
    }
    std::string Token() const
    {
        std::unique_lock<std::mutex> lck(mMtx);
        auto val = mToken;
        return val;
    }
    std::vector<std::string> StunServers() const
    {
        std::unique_lock<std::mutex> lck(mMtx);
        auto val = mStunServers;
        return val;
    }

    void SetPeerId(const std::string& peerId)
    {
        std::unique_lock<std::mutex> lck(mMtx);
        if (peerId != mPeerId) {
            mPeerId = peerId;
            mKv["peerId"] = peerId;
        }
    }
    void SetToken(const std::string& token)
    {
        std::unique_lock<std::mutex> lck(mMtx);
        if (token != mToken) {
            mToken = token;
            mKv["token"] = token;
        }
    }
    unsigned WebRtcGatherPeriod() const
    {
        std::unique_lock<std::mutex> lck(mMtx);
        unsigned val = mWebRtcGatherPeriod;
        return val;
    }
public:
    static int CreateTable(sqlite3* db);
private:
    static int loadConfigCallback(void* , int argc, char** argv, char** colName);
private:
    mutable std::mutex mMtx;

    std::string mApiRootUrl;
    std::string mPeerId;
    std::string mToken;
    std::vector<std::string> mStunServers;

    unsigned mWebRtcGatherPeriod = 60; //seconds

    std::unordered_map<std::string, std::string> mKv;
};

NS_END


#endif
