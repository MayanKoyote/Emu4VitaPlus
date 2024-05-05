#pragma once
#include <psp2/kernel/threadmgr.h>
#include "thread_base.h"

class Video : public ThreadBase
{
public:
    Video();
    virtual ~Video();

private:
    static int _DrawThread(SceSize args, void *argp);
};