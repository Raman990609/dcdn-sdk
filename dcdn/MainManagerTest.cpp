#include <iostream>
#include <spdlog/spdlog.h>
#include "MainManager.h"

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::debug);
    dcdn::MainManagerOption opt;
    opt.WorkDir = "data";
    int ret = dcdn::MainManager::Init(opt);
    if (ret != 1) {
        spdlog::error("Init MainManager fail");
        return 1;
    }
    dcdn::MainManager* m = dcdn::MainManager::Singlet();
    spdlog::info("Start MainManager");
    m->Start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
