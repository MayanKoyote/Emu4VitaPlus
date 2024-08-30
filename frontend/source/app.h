#pragma once
#include "video.h"
#include "log.h"

enum APP_STATUS
{
    APP_STATUS_BOOT = 1,
    APP_STATUS_SHOW_UI = 1 << 2,
    APP_STATUS_RUN_GAME = 1 << 3,
    APP_STATUS_SHOW_UI_IN_GAME = 1 << 4,
    APP_STATUS_REWIND_GAME = 1 << 5,
    APP_STATUS_RETURN_ARCH = 1 << 6,
    APP_STATUS_EXIT = 1 << 7,
};

class AppStatus
{
public:
    AppStatus() : _status(APP_STATUS_BOOT) {};
    virtual ~AppStatus() {};

    APP_STATUS Get() { return _status; };
    void Set(APP_STATUS status)
    {
        if (_status != status)
        {
            LogDebug("Status from %d to %d", _status, status);
            gVideo->Lock();
            _status = status;
            gVideo->Unlock();
        }
    };

private:
    APP_STATUS _status;
};

extern AppStatus gStatus;

class App
{
public:
    App(int argc, char *const argv[]);
    virtual ~App();

    void Run();

private:
    bool _IsSaveMode();
    void _ParseParams(int argc, char *const argv[]);
};