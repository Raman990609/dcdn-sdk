#ifndef _DCDN_UTIL_HTTP_DOWNLOADER_H_
#define _DCDN_UTIL_HTTP_DOWNLOADER_H_

#include <thread>
#include <tuple>
#include <any>
#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include "common/Common.h"
#include <curl/curl.h>
#include <curl/curl.h>

NS_BEGIN(dcdn)
NS_BEGIN(util)

struct HttpDownloaderTask
{
    std::string Url;
    size_t Start = 0;
    size_t End = 0;
    size_t MaxBuf = 0;

    void (*Notify)(void* recevier);
    void* Receiver;
};

class HttpDownloader
{
public:
    class Task
    {
    public:
        enum StatusType
        {
            Idle,
        };
    public:
        Task(CURL* curl, const HttpDownloaderTask& opt):
            mCurl(curl),
            mMaxBuf(opt.MaxBuf),
            mNotify(opt.Notify),
            mReceiver(opt.Receiver)
        {
        }
        ~Task()
        {
            if (mCurl) {
                curl_easy_cleanup(mCurl);
            }
        }
        StatusType Status() const
        {
            std::unique_lock<std::mutex> lck(mMtx);
            return mStatus;
        }
        const std::string& ContentType() const
        {
            std::unique_lock<std::mutex> lck(mMtx);
            return mContentType;
        }
        size_t ContentLength() const
        {
            std::unique_lock<std::mutex> lck(mMtx);
            return mContentLength;
        }
        size_t Read(std::vector<unsigned char>& data)
        {
            std::unique_lock<std::mutex> lck(mMtx);
            size_t offset = mOffset;
            mOffset += mData.size();
            data.swap(mData);
            return offset;
        }
    private:
        void notify()
        {
            if (mNotify) {
                mNotify(mReceiver);
            }
        }
        void setContent(const std::string& tp, size_t length)
        {
            std::unique_lock<std::mutex> lck(mMtx);
            mContentType = tp;
            mContentLength = length;
        }
        void write(const unsigned char* dat, size_t len)
        {
            std::unique_lock<std::mutex> lck(mMtx);
            mData.insert(mData.end(), dat, dat + len);
        }
    private:
        friend class HttpDownloader;
        StatusType mStatus = Idle;
        void (*mNotify)(void*);
        void* mReceiver = nullptr;
        mutable std::mutex mMtx;
        CURL* mCurl = nullptr;
        size_t mMaxBuf = 0;
        size_t mOffset = 0;
        std::vector<unsigned char> mData;
        size_t mContentLength = 0;
        std::string mContentType;
    };
public:
    HttpDownloader()
    {
    }
    ~HttpDownloader()
    {
    }
    int Init()
    {
        mCM = curl_multi_init();
        if (!mCM) {
            return ErrorCodeErr;
        }
        return ErrorCodeOk;
    }
    void Start(bool detach=true)
    {
        mThread = std::make_shared<std::thread>([&]() {
            run();
        });
        if (detach) {
            mThread->detach();
        }
    }
    std::shared_ptr<std::thread> Thread() {
        return mThread;
    }
    std::shared_ptr<Task> AddTask(const HttpDownloaderTask& opt)
    {
        auto c = curl_easy_init();
        if (!c) {
            return nullptr;
        }
        auto t = std::make_shared<Task>(c, opt);
        curl_easy_setopt(c, CURLOPT_URL, opt.Url.c_str());
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(c, CURLOPT_WRITEDATA, t.get());

        postEvent(EventType::AddTask, t);
        return t;
    }
    void CancelTask(std::shared_ptr<Task> task)
    {
        postEvent(EventType::CancelTask, task);
    }
private:
    enum class EventType
    {
        AddTask,
        CancelTask,
    };
    typedef std::pair<EventType, std::any> Event;
    template<class T>
    void postEvent(EventType tp, const T& arg)
    {
        std::unique_lock<std::mutex> lck(mMtx);
        auto e = std::make_shared<Event>(tp, arg);
        mEvts.push_back(e);
        curl_multi_wakeup(mCM);
    }

    void run()
    {
        std::list<std::shared_ptr<Event>> evts;
        while (true) {
            int num = 0;
            int mc = curl_multi_perform(mCM, &num);
            if (mc != CURLM_OK) {
                //TODO abort all tasks, recovery CURLM
                continue;
            }

            mc = curl_multi_poll(mCM, NULL, 0, 10000, &num);
            if (mc != CURLM_OK) {
                //TODO abort all tasks, recovery CURLM
                continue;
            }

            do {
                std::unique_lock<std::mutex> lck(mMtx);
                evts.swap(mEvts);
            } while (0);
            while (!evts.empty()) {
                auto evt = evts.front();
                evts.pop_front();
                handleEvent(evt);
            }

            for (auto it : mTasks) {
                Task* t = it.first;
                if (!t->mData.empty()) {
                    t->notify();
                }
            }
        }
    }
    void handleEvent(std::shared_ptr<Event> evt)
    {
        switch (evt->first) {
        case EventType::AddTask:
            break;
        case EventType::CancelTask:
            break;
        default:
            break;
        }
    }
    void handleAddTaskEvent(std::shared_ptr<Event> evt)
    {
        auto t = std::any_cast<std::shared_ptr<Task>>(evt->second);
        mTasks[t.get()] = t;
        int ret = curl_multi_add_handle(mCM, t->mCurl);
        if (ret != CURLM_OK) {
            //TODO setTask fail
            return;
        }
    }
    void handleCancelTaskEvent(std::shared_ptr<Event> evt)
    {
        auto t = std::any_cast<std::shared_ptr<Task>>(evt->second);
        auto it = mTasks.find(t.get());
        if (it != mTasks.end()) {
            mTasks.erase(it);
        }
    }
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* data)
    {
        Task* t = static_cast<Task*>(data);
        const unsigned char* p = (const unsigned char*)contents;
        size *= nmemb;
        t->write(p, size);
        return size;
    }
private:
    std::shared_ptr<std::thread> mThread;
    std::mutex mMtx;
    std::list<std::shared_ptr<Event>> mEvts;

    CURLM* mCM;
    std::unordered_map<Task*, std::shared_ptr<Task>> mTasks;
};

NS_END
NS_END

#endif
