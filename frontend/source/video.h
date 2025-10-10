#pragma once
#include "thread_base.h"

namespace Emu4VitaPlus
{
    class Video : public ThreadBase
    {
    public:
        Video();
        virtual ~Video();

    private:
        static int _DrawThread(SceSize args, void *argp);
    };
}
