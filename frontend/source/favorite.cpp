#include <SimpleIni.h>
#include "favorite.h"
#include "log.h"

Favorites *gFavorites = nullptr;

Favorites::Favorites()
{
    Load();
}

Favorites::~Favorites()
{
}

bool Favorites::Load(const char *path)
{
    LogFunctionName;

    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        LogWarn("Load %s failed", path);
        return false;
    }

    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);
    for (auto const &section : sections)
    {
        Favorite fav;
        const char *sec = section.pItem;
        fav.path = ini.GetValue(sec, "path");
        fav.item.name = ini.GetValue(sec, "name");
        fav.item.entry_name = ini.GetValue(sec, "entry");
        fav.item.crc32 = ini.GetLongValue(sec, "crc32");
        this->emplace(fav.item.name, fav);
    }

    return true;
}

bool Favorites::Save(const char *path)
{
    LogFunctionName;

    CSimpleIniA ini;
    char section[8];
    int count = 0;

    for (const auto &fav : *this)
    {
        snprintf(section, 8, "%04d", count);

        ini.SetValue(section, "path", fav.second.path.c_str());
        ini.SetValue(section, "name", fav.second.item.name.c_str());
        ini.SetValue(section, "entry", fav.second.item.entry_name.c_str());
        ini.SetLongValue(section, "crc32", fav.second.item.crc32);

        count++;
    }

    return ini.SaveFile(path) == SI_OK;
}