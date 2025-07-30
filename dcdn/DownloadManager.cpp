#include "DownloadManager.h"

#include "DownloadManager.h"
#include "MainManager.h"

NS_BEGIN(dcdn)

DownloadManager::DownloadManager(MainManager* man):
    BaseManager(man)
{
}

void DownloadManager::run()
{
    spdlog::info("Download running");
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    spdlog::info("Download exit");
}

NS_END
