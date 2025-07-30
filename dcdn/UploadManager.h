#ifndef _DCDN_SDK_UPLOAD_MANAGER_H_
#define _DCDN_SDK_UPLOAD_MANAGER_H_

#include "BaseManager.h"

NS_BEGIN(dcdn)

class UploadManager : public BaseManager
{
public:
    UploadManager(MainManager* man);
private:
    void run();
};

NS_END

#endif
