#ifndef _DCDN_SDK_FILE_MANAGER_H_
#define _DCDN_SDK_FILE_MANAGER_H_

#include <thread>
#include "common/Config.h"

NS_BEGIN(dcdn)

class MainManager;
class FileManager
{
public:
    FileManager(MainManager* man);
    void Start();
private:
    void run();
private:
    MainManager* mMan;
};


NS_END

#endif
