#include "UploadManager.h"
#include "MainManager.h"

NS_BEGIN(dcdn)

UploadManager::UploadManager(MainManager* man):
    BaseManager(man)
{
}

void UploadManager::run()
{
    logInfo << "Upload running";
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    logInfo << "Upload exit";
}

NS_END
