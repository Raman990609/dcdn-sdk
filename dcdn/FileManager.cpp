#include "FileManager.h"
#include "MainManager.h"

NS_BEGIN(dcdn)

FileManager::FileManager(MainManager* man):
    mMan(man)
{
}

void FileManager::Start()
{
    std::thread t([&]() {
        run();
    });
    t.detach();
}

void FileManager::run()
{
    spdlog::info("FileManager running");
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    spdlog::info("FileManager exit");
}

NS_END
