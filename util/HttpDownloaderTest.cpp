#include <chrono>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Appenders/ConsoleAppender.h>
#include "HttpDownloader.h"

using dcdn::util::HttpDownloaderTask;
using dcdn::util::HttpDownloaderTaskOption;
using dcdn::util::HttpDownloader;
using dcdn::util::HttpClientOption;
struct Task
{
    std::string url;
    std::shared_ptr<HttpDownloaderTask> task;
    std::string filename;
    std::shared_ptr<std::ofstream> file;
};

class Manager
{
public:
    Manager()
    {
    }
    HttpClientOption& Option()
    {
        return mDownloader;
    }
    void Run(const std::vector<std::string> urls)
    {
        int ret = mDownloader.Init();
        if (ret != 1) {
            logWarn << "Init fail" << std::endl;
            return;
        }
        auto lastReportTime = std::chrono::steady_clock::now();
        mDownloader.Start();
        int i = 1;
        for (auto& url : urls) {
            HttpDownloaderTaskOption opt;
            opt.Url = url;
            opt.Notify = notifyCallback;
            opt.Receiver = this;
            Task task;
            task.url = url;
            char path[1024];
            snprintf(path, sizeof(path), "p%d.dat", i++);
            task.filename = path;
            task.file = std::make_shared<std::ofstream>(path, std::ios::binary);
            if (!*task.file) {
                logWarn << "fail to open file: " << path << " for url: " << url;
            }
            auto t = mDownloader.AddTask(opt);
            if (t) {
                task.task = t;
                mTasks[t.get()] = task;
                logInfo << "create http download task for url: " << url << " save to: " << path;
            } else {
                logWarn << "fail to add http download task";
            }
        }
        std::vector<unsigned char> data;
        while (!mTasks.empty()) {
            HttpDownloaderTask* t = nullptr;
            do {
                std::unique_lock<std::mutex> lck(mMtx);
                while (mTaskEvents.empty()) {
                    mCv.wait(lck);
                }
                t = *mTaskEvents.begin();
                mTaskEvents.erase(mTaskEvents.begin());
            } while (!t);
            auto it = mTasks.find(t);
            if (it == mTasks.end()) {
                continue;
            }
            logVerb << "handle task url:" << t->Option().Url;
            auto& task = it->second;
            auto offset = task.task->Read(data);
            if (!data.empty()) {
                task.file->write((const char*)data.data(), data.size());
                if (!*task.file) {
                    logWarn << "fail to write file: " << task.filename;
                    mDownloader.CancelTask(task.task);
                    mTasks.erase(it);
                    continue;
                }
            }
            if (task.task->IsEnd()) {
                auto st = task.task->Status();
                if (st == HttpDownloaderTask::Completed) {
                    logInfo << "success for url: " << task.url
                        << " file: " << task.filename
                        << " type: " << task.task->ContentType()
                        << " length: " << task.task->ContentLength();
                } else {
                    logWarn  << "fail for url: " << task.url << " file: " << task.filename;
                }
                mTasks.erase(it);
            }
            auto now = std::chrono::steady_clock::now();
            if (now - lastReportTime >= std::chrono::seconds(2)) {
                for (auto& it : mTasks) {
                    auto t = it.first;
                    logInfo << "task url:" << t->Option().Url
                        << " progress:(" << t->Size() << "/" << t->ContentLength() << ")";
                }
                lastReportTime = now;
            }
        }
    }
private:
    static void notifyCallback(std::shared_ptr<HttpDownloaderTask> task, void* r)
    {
        static_cast<Manager*>(r)->notify(task.get());
    }
    void notify(HttpDownloaderTask* task)
    {
        std::unique_lock<std::mutex> lck(mMtx);
        mTaskEvents.insert(task);
        mCv.notify_one();
    }
private:
    std::mutex mMtx;
    std::condition_variable mCv;
    HttpDownloader mDownloader;
    std::unordered_map<HttpDownloaderTask*, Task> mTasks;
    std::unordered_set<HttpDownloaderTask*> mTaskEvents;
};

int main(int argc, char* argv[])
{
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::Severity lvl = plog::debug;
    std::string ua;
    int follow = 0;
    std::vector<std::string> urls;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0) {
            lvl = plog::verbose;
        } else if (strcmp(argv[i], "--ua") == 0) {
            if (++i < argc) {
                ua = argv[i];
            }
        } else if (strcmp(argv[i], "--follow") == 0) {
            if (++i < argc) {
                follow = atoi(argv[i]);
            }
        } else {
            urls.push_back(argv[i]);
        }
    }
    if (urls.empty()) {
        std::cout << "Usage: " << argv[0]
            << " [-v]"
            << " [--ua xxx]"
            << " [--follow int]"
            << " <url>..."
            << std::endl;
        std::cout << "eg:\n" << argv[0] << " "
            << "--follow 1 --ua 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36' https://curl.se/download/curl-8.15.0.tar.gz https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz" << std::endl;
        return 1;
    }
    plog::init<DCDN_LOGGER_ID>(lvl, &consoleAppender);
    Manager m;
    if (!ua.empty()) {
        m.Option().SetUserAgent(ua.c_str());
    }
    if (follow > 0) {
        m.Option().SetFollowLocation(follow);
    }
    m.Run(urls);
    return 0;
}
