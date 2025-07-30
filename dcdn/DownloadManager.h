#ifndef _DCDN_SDK_DOWNLOAD_MANAGER_H_
#define _DCDN_SDK_DOWNLOAD_MANAGER_H_

#include "BaseManager.h"

NS_BEGIN(dcdn)

class DownloadManager: public BaseManager
{
public:
    DownloadManager(MainManager* man);
private:
    void run();
};

NS_END

#endif
