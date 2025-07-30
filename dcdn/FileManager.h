#ifndef _DCDN_SDK_FILE_MANAGER_H_
#define _DCDN_SDK_FILE_MANAGER_H_

#include <thread>
#include "common/Config.h"
#include "BaseManager.h"

NS_BEGIN(dcdn)

class FileManager: public BaseManager
{
public:
    FileManager(MainManager* man);
private:
    void run();
private:
};


NS_END

#endif
