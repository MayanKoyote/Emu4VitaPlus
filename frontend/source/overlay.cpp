#include "SimpleIni.h"
#include "overlay.h"
#include "log.h"
#include "video.h"
#include "defines.h"

Overlays *gOverlays;

Overlay::Overlay() : _texture(nullptr)
{
}

Overlay::~Overlay()
{
    LogFunctionName;
    if (_texture != nullptr)
    {
        gVideo->Lock();
        vita2d_wait_rendering_done();
        vita2d_free_texture(_texture);
        _texture = nullptr;
        gVideo->Unlock();
    }
}

vita2d_texture *Overlay::Get()
{
    if (_texture == nullptr && image_name.size() > 0)
    {
        for (auto const &path : {"app0:assets", CONSOLE_DIR, APP_DATA_DIR})
        {
            const std::string name = std::string(path) + "/" OVERLAYS_DIR_NAME "/" + image_name;

            _texture = vita2d_load_PNG_file(name.c_str());
            if (_texture)
                break;
            _texture = vita2d_load_JPEG_file(name.c_str());
            if (_texture)
                break;
        }
    }
    return _texture;
}

Overlays::Overlays()
{
    Load("app0:assets/" OVERLAYS_DIR_NAME);
    Load((std::string(CONSOLE_DIR) + "/" + OVERLAYS_DIR_NAME).c_str());
    Load((std::string(APP_DATA_DIR) + "/" OVERLAYS_DIR_NAME).c_str());
}

Overlays::~Overlays()
{
    LogFunctionName;
}

bool Overlays::Load(const char *path)
{
    LogFunctionName;

    CSimpleIniA ini;
    if (ini.LoadFile((std::string(path) + "/overlays.ini").c_str()) != SI_OK)
    {
        LogError("failed to load overlays.ini from %s", path);
        return false;
    }

    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);
    this->reserve(this->size() + sections.size());
    for (const auto &section : sections)
    {
        Overlay overlay;
        overlay.name = section.pItem;
        overlay.image_name = ini.GetValue(section.pItem, "image_name", "NULL");
        overlay.viewport_width = ini.GetLongValue(section.pItem, "viewport_width");
        if (overlay.viewport_width == 0)
        {
            continue;
        }
        overlay.viewport_height = ini.GetLongValue(section.pItem, "viewport_height");
        if (overlay.viewport_height == 0)
        {
            continue;
        }
        overlay.viewport_rotate = ini.GetLongValue(section.pItem, "viewport_rotate");
        overlay.viewport_x = ini.GetLongValue(section.pItem, "viewport_x");
        overlay.viewport_y = ini.GetLongValue(section.pItem, "viewport_y");
        this->emplace_back(overlay);
        LogDebug("  %s %s %d %d", section.pItem, overlay.image_name.c_str(), overlay.viewport_width, overlay.viewport_height);
    }

    return true;
}

std::vector<LanguageString> Overlays::GetConfigs()
{
    std::vector<LanguageString> configs;
    configs.emplace_back(LanguageString(NONE));
    for (const auto &overlay : *this)
    {
        configs.emplace_back(overlay.name);
    }
    return configs;
}