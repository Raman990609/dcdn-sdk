#include <iostream>
#include "HttpDownloader.h"


int main(int argc, char* argv[])
{
    using dcdn::util::HttpDownloader;
    HttpDownloader d;
    int ret = d.Init();
    if (ret != 1) {
        std::cout << "Init fail" << std::endl;
        return 1;
    }
    d.Start();
    //std::vector<std::shared_ptr<
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
