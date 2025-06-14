#include <vita2d.h>
#include <string>
#include <math.h>
#include <zlib.h>
#include <map>
#include "tab_browser.h"
#include "emulator.h"
#include "video.h"
#include "app.h"
#include "ui.h"
#include "file.h"
#include "log.h"
#include "favorite.h"
#include "config.h"
#include "icons.h"
#include "state_manager.h"
#include "misc.h"
#include "utils.h"
#include "ra_lpl.h"
#include "global.h"

#define HISTROY_SIZE 10

TabBrowser::TabBrowser() : TabSeletable(LANG_BROWSER),
                           _texture(nullptr),
                           _texture_max_width(BROWSER_TEXTURE_MAX_WIDTH),
                           _texture_max_height(BROWSER_TEXTURE_MAX_HEIGHT),
                           _text_dialog(nullptr),
                           _name(nullptr),
                           _dialog(nullptr)
{
    LogFunctionName;

    const char *path = gConfig->last_rom.c_str();
    _directory = new Directory(File::GetDir(path).c_str(), gEmulator->GetValidExtensions());
    std::string name = File::GetName(path);
    for (size_t i = 0; i < _directory->GetSize(); i++)
    {
        if (_directory->GetItemName(i) == name)
        {
            _index = i;
            break;
        }
    }

    _confirm_dialog = new Dialog{"",
                                 {LANG_OK, LANG_CANCEL},
                                 std::bind(&TabBrowser::_OnConfirmDialog, this, std::placeholders::_1, std::placeholders::_2)};

    _name_map.Load();
    _Update();
}

TabBrowser::~TabBrowser()
{
    LogFunctionName;

    delete _directory;
    delete _confirm_dialog;

    if (_text_dialog != nullptr)
    {
        delete _text_dialog;
    }

    if (_dialog != nullptr)
    {
        delete _dialog;
    }
}

void TabBrowser::SetInputHooks(Input *input)
{
    TabSeletable::SetInputHooks(input);
    input->SetKeyUpCallback(CancelButton, std::bind(&TabBrowser::_OnKeyCross, this, input));
    input->SetKeyUpCallback(SCE_CTRL_TRIANGLE, std::bind(&TabBrowser::_OnKeyTriangle, this, input));
    input->SetKeyUpCallback(SCE_CTRL_SQUARE, std::bind(&TabBrowser::_OnKeySquare, this, input));
    input->SetKeyUpCallback(SCE_CTRL_START, std::bind(&TabBrowser::_OnKeyStart, this, input));
    input->SetKeyUpCallback(SCE_CTRL_SELECT, std::bind(&TabBrowser::_OnKeySelect, this, input));
    _input = input;
}

void TabBrowser::UnsetInputHooks(Input *input)
{
    TabSeletable::UnsetInputHooks(input);
    input->UnsetKeyUpCallback(CancelButton);
    input->UnsetKeyUpCallback(SCE_CTRL_TRIANGLE);
    input->UnsetKeyUpCallback(SCE_CTRL_SQUARE);
    input->UnsetKeyUpCallback(SCE_CTRL_START);
    input->UnsetKeyUpCallback(SCE_CTRL_SELECT);
}

void TabBrowser::_Show()
{
    ImVec2 size = {0.f, 0.f};
    if (_status_text.size() > 0)
    {
        ImVec2 s = ImGui::CalcTextSize(_status_text.c_str());
        size.y = -s.y * (s.x / ImGui::GetContentRegionAvailWidth() + 1);
    }

    if (ImGui::BeginChild(TEXT(_title_id), size))
    {
        ImGui::Columns(2, NULL, false);

        std::string current_path = _directory->GetCurrentPath();
        if (_directory->GetSearchString().size() > 0 && _directory->GetSearchResults().size() > 0)
        {
            current_path += std::string(" " ICON_SERACH) + _directory->GetSearchString();
        }

        ImGui::TextUnformatted(current_path.c_str());
        ImVec2 current_pos;
        if (ImGui::ListBoxHeader("", ImGui::GetContentRegionAvail()))
        {
            if (_in_refreshing)
            {
                _spin_text.Show();
            }
            else
            {
                for (size_t i = 0; i < _directory->GetSize(); i++)
                {
                    const DirItem &item = _directory->GetItem(i);

                    std::string name(item.name);

                    if (!item.is_dir)
                    {
                        ImU32 color;
                        if (_directory->BeFound(i))
                            color = IM_COL32(255, 255, 33, 255);
                        else
                            color = IM_COL32(0, 255, 0, 255);

                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        if (gFavorites->find(name) != gFavorites->end())
                            name.insert(0, ICON_EMPTY_STAR_SPACE);
                    }

                    if (i == _index)
                    {
                        current_pos = ImGui::GetCursorScreenPos();
                        My_ImGui_Selectable(name.c_str(), true, &_moving_status);
                    }
                    else
                        ImGui::Selectable(name.c_str());

                    if (!item.is_dir)
                        ImGui::PopStyleColor();

                    if (i == _index && ImGui::GetScrollMaxY() > 0.f)
                        ImGui::SetScrollHereY((float)_index / (float)_directory->GetSize());
                }
            }

            ImGui::ListBoxFooter();
        }
        ImGui::NextColumn();
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        _texture_max_width = avail_size.x;
        _texture_max_height = avail_size.y;
        ImVec2 pos = ImGui::GetCursorScreenPos();

        if (_texture != nullptr)
        {
            ImVec2 texture_pos = pos;
            texture_pos.x += ceilf(fmax(0.0f, (avail_size.x - _texture_width) * 0.5f));
            texture_pos.y += ceilf(fmax(0.0f, (avail_size.y - _texture_height) * 0.5f));
            ImGui::SetCursorScreenPos(texture_pos);
            ImGui::Image(_texture, {_texture_width, _texture_height});
        }

        if (_name != nullptr)
        {
            _name_moving_status.Update(_name);

            ImVec2 text_size = ImGui::CalcTextSize(_name);
            ImVec2 text_pos = pos;
            text_pos.x += fmax(0, (avail_size.x - text_size.x) / 2) + _name_moving_status.pos;
            text_pos.y += (_texture == nullptr ? (avail_size.y - text_size.y) / 2 : 10);
            if (_texture)
            {
                My_ImGui_HighlightText(_name, text_pos, IM_COL32_GREEN, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));
            }
            else
            {
                ImGui::SetCursorScreenPos(text_pos);
                ImGui::TextUnformatted(_name);
            }
        }

        if (_info.size() > 0)
        {
            ImVec2 text_size = ImGui::CalcTextSize(_info.c_str());
            ImVec2 text_pos = pos;

            if (_texture)
            {
                text_pos.y += avail_size.y - text_size.y - 10;
                My_ImGui_HighlightText(_info.c_str(), text_pos, IM_COL32_GREEN, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));
            }
            else
            {
                text_pos.y += (avail_size.y - text_size.y) / 2;
                if (_name)
                {
                    text_pos.y += text_size.y;
                }
                ImGui::SetCursorScreenPos(text_pos);
                ImGui::TextUnformatted(_info.c_str());
            }
        }

        ImGui::NextColumn();

        ImGui::Columns(1);
    }

    ImGui::EndChild();

    if (_status_text.size() > 0)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xcc, 0xcc, 0xcc, 255));
        ImGui::TextWrapped(_status_text.c_str());
        ImGui::PopStyleColor();
    }
}

void TabBrowser::Show(bool selected)
{
    if (_text_dialog != nullptr && _text_dialog->GetStatus())
    {
        if (_cmd == CMD_RENAME)
        {
            std::string src_path = _GetCurrentFullPath();
            const char *dst_path = _text_dialog->GetInput();
            File::MoveFile(src_path.c_str(), dst_path);
            _directory->Refresh();
            _cmd = 0xff;
        }
        else
        {
            const char *s = _text_dialog->GetInput();
            if (*s)
            {
                _Search(s);
            }
        }

        delete _text_dialog;
        _text_dialog = nullptr;
        SetInputHooks(_input);
    }

    TabBase::Show(selected);

    if (_dialog && _dialog->IsActived())
    {
        _dialog->Show();
    }

    if (_confirm_dialog->IsActived())
    {
        _confirm_dialog->Show();
    }
}

void TabBrowser::_SaveIndexHistory()
{
    _dir_history[_directory->GetCurrentPath()] = _index;
    if (_dir_history.size() > HISTROY_SIZE)
    {
        _dir_history.erase(_dir_history.begin());
    }
    LogDebug("  save history: %s %d", _directory->GetCurrentPath().c_str(), _index);
}

void TabBrowser::_SetIndexFromHistory()
{
    const auto iter = _dir_history.find(_directory->GetCurrentPath());
    if (iter == _dir_history.end())
    {
        _index = 0;
    }
    else
    {
        _index = iter->second;
        LogDebug("  get hitory: %s %d", _directory->GetCurrentPath().c_str(), _index);
    }
}

void TabBrowser::_OnActive(Input *input)
{
    LogFunctionName;
    if (_index >= _directory->GetSize())
    {
        return;
    }

    auto item = _directory->GetItem(_index);

    _SaveIndexHistory();

    if (item.is_dir)
    {
        _in_refreshing = true;

        LogDebug("%d %s", _directory->GetCurrentPath().size(), item.name.c_str());
        if (_directory->GetCurrentPath().size() == 0)
        {
            _directory->SetCurrentPath(item.name);
        }
        else
        {
            _directory->SetCurrentPath(_directory->GetCurrentPath() + "/" + item.name);
        }
        _in_refreshing = false;

        _SetIndexFromHistory();
        _Update();
    }
    else
    {
        if (gEmulator->LoadRom((_directory->GetCurrentPath() + "/" + item.name).c_str(), item.entry_name.c_str(), item.crc32) && gStatus.Get() == APP_STATUS_RUN_GAME)
        {
            UnsetInputHooks(input);
        }
    }
}

void TabBrowser::_OnKeyCross(Input *input)
{
    LogFunctionName;
    auto path = _directory->GetCurrentPath();

    _SaveIndexHistory();
    _in_refreshing = true;
    if (path.size() <= 5)
    {
        _directory->SetCurrentPath("");
    }
    else
    {
        size_t pos = path.rfind('/');
        if (pos != std::string::npos)
        {
            _directory->SetCurrentPath(path.substr(0, pos));
        }
    }

    _in_refreshing = false;

    _index = 0;

    _SetIndexFromHistory();
    _Update();
}

void TabBrowser::_OnKeyStart(Input *input)
{
    if (_index >= _directory->GetSize())
    {
        return;
    }

    DirItem item = _directory->GetItem(_index);
    if (item.is_dir)
    {
        return;
    }

    const auto &iter = gFavorites->find(item.name);
    if (iter == gFavorites->end())
    {
        gFavorites->emplace(item.name, Favorite{item, _directory->GetCurrentPath(), _name ? _name : ""});
    }
    else
    {
        gFavorites->erase(item.name);
    }

    gFavorites->Save();
    _UpdateStatus();
}

void TabBrowser::_OnKeySelect(Input *input)
{
    if (_dialog)
    {
        delete _dialog;
        _dialog = nullptr;
    }

    std::vector<LanguageString> options;
    if (!_clipboard.Empty())
    {
        options.push_back(LANG_PASTE);
    }

    if (!_directory->GetItem(_index).is_dir)
    {
        options.push_back(LANG_COPY);
        options.push_back(LANG_CUT);
        options.push_back(LANG_DELETE);
        options.push_back(LANG_RENAME);
    }

    if (options.size() == 0)
    {
        return;
    }

    _dialog = new Dialog("",
                         options,
                         std::bind(&TabBrowser::_OnDialog, this, std::placeholders::_1, std::placeholders::_2));

    _dialog->OnActive(input);
}

void TabBrowser::_OnDialog(Input *input, int index)
{
    LogFunctionName;
    if (!_clipboard.Empty())
    {
        index--;
    }

    _cmd = index;
    switch (index)
    {
    case CMD_PASTE:
        LogDebug("Paste");
        _PasteFile(false);
        break;

    case CMD_COPY:
        LogDebug("Copy");
        _clipboard.path = _GetCurrentFullPath();
        _clipboard.cut = false;
        break;

    case CMD_CUT:
        LogDebug("Cut");
        _clipboard.path = _GetCurrentFullPath();
        _clipboard.cut = true;
        break;

    case CMD_DELETE:
        LogDebug("Delete");
        _confirm_dialog->SetText(std::string(TEXT(LANG_DELETE_FILE_CONFIRM)) + "\n" + _GetCurrentFullPath());
        _confirm_dialog->OnActive(input);
        break;

    case CMD_RENAME:
        LogDebug("Rename");
        if (_index >= _directory->GetSize())
        {
            break;
        }

        if (_text_dialog != nullptr)
        {
            delete _text_dialog;
        }

        _text_dialog = new InputTextDialog(TEXT(LANG_SEARCH), _directory->GetItemName(_index).c_str());
        if (_text_dialog->Init())
        {
            _input = input;
            UnsetInputHooks(input);
        }
        else
        {
            delete _text_dialog;
            _text_dialog = nullptr;
        }

        break;

    default:
        break;
    }
}

void TabBrowser::_OnConfirmDialog(Input *input, int index)
{
    LogFunctionName;
    if (index != 0)
    {
        return;
    }

    LogDebug("_cmd %d", _cmd);
    switch (_cmd)
    {
    case CMD_PASTE:
        LogDebug("Paste");
        _PasteFile(true);
        break;

    case CMD_DELETE:
        LogDebug("Delete");
        {
            const std::string path = _GetCurrentFullPath();
            if (File::Remove(path.c_str()))
            {
                _clipboard.Reset();
                _directory->Refresh();
                _Update();
                gUi->SetHint((std::string(TEXT(LANG_DELETED)) + "\n" + path).c_str());
            }
            else
            {
                gUi->SetHint((std::string(TEXT(LANG_DELETION_FAILED)) + "\n" + path).c_str());
            }
        }
        break;

    default:
        break;
    }
}

void TabBrowser::_PasteFile(bool overwrite)
{
    LogFunctionName;

    if (_clipboard.path.empty())
    {
        return;
    }

    std::string name = File::GetName(_clipboard.path.c_str());
    std::string dst_path = _directory->GetCurrentPath() + "/" + name;

    if (_clipboard.path == dst_path)
    {
        LogWarn("Cannot paste in the same directory");
        gUi->SetHint(TEXT(LANG_SAME_DIR_WARN));
        return;
    }

    if (File::Exist(dst_path.c_str()) && !overwrite)
    {
        _confirm_dialog->SetText(std::string(TEXT(LANG_OVERWRITE_FILE_CONFIRM)) + "\n" + dst_path);
        _confirm_dialog->OnActive(_input);
        return;
    }

    if (_clipboard.cut)
    {
        LogDebug("cut");
        if (File::Exist(dst_path.c_str()))
            File::Remove(dst_path.c_str());
        File::MoveFile(_clipboard.path.c_str(), dst_path.c_str());
    }
    else
    {
        LogDebug("copy");
        File::CopyFile(_clipboard.path.c_str(), dst_path.c_str());
    }

    _clipboard.Reset();
    _directory->Refresh();
    _index = _directory->GetIndex(name.c_str());
    _Update();
}

void TabBrowser::_UpdateTexture()
{
    if (_texture != nullptr)
    {
        gVideo->Lock();
        vita2d_wait_rendering_done();
        vita2d_free_texture(_texture);
        _texture = nullptr;
        gVideo->Unlock();
    }

    if (_index >= _directory->GetSize())
    {
        return;
    }

    const DirItem &item = _directory->GetItem(_index);
    if (item.is_dir)
    {
        return;
    }

    const std::string full_path = _GetCurrentFullPath();
    if (gPlaylists->IsValid())
    {
        _texture = gPlaylists->GetPreviewImage(full_path.c_str());
    }

    if (_texture == nullptr)
    {
        _texture = GetRomPreviewImage(_directory->GetCurrentPath().c_str(), item.name.c_str());
    }

    if (_texture)
    {
        CalcFitSize(vita2d_texture_get_width(_texture),
                    vita2d_texture_get_height(_texture),
                    _texture_max_width,
                    _texture_max_height,
                    &_texture_width,
                    &_texture_height);
    }
}

void TabBrowser::_UpdateStatus()
{
    if (_index >= _directory->GetSize())
    {
        gVideo->Lock();
        _status_text.clear();
        gVideo->Unlock();
        return;
    }

    const DirItem &item = _directory->GetItem(_index);

    gVideo->Lock();
    _status_text = EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CIRCLE : BUTTON_CROSS;
    _status_text += item.is_dir ? TEXT(LANG_ENTER_DIR) : TEXT(LANG_START_GAME);
    _status_text += "\t";
    _status_text += EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CROSS : BUTTON_CIRCLE;
    _status_text += TEXT(LANG_BACK_DIR);
    _status_text += "\t";
    _status_text += BUTTON_TRIANGLE;
    _status_text += TEXT(LANG_SEARCH);
    _status_text += "\t";

    if (_directory->GetSearchString().size() > 1)
    {
        _status_text += BUTTON_SQUARE;
        _status_text += TEXT(LANG_NEXT);
        _status_text += "\t";
    }

    if (!item.is_dir)
    {
        _status_text += BUTTON_START;
        if (gFavorites->find(item.name) == gFavorites->end())
        {
            _status_text += TEXT(LANG_ADD_FAVORITE);
        }
        else
        {
            _status_text += TEXT(LANG_REMOVE_FAVORITE);
        }
        _status_text += "\t";
    }

    if ((!item.is_dir) || !_clipboard.Empty())
    {
        _status_text += BUTTON_SELECT;
        _status_text += TEXT(LANG_FILE_MANAGE);
    }

    gVideo->Unlock();
}

void TabBrowser::_UpdateName()
{
    // LogFunctionName;
    if (_name)
    {
        gVideo->Lock();
        _name = nullptr;
        _name_moving_status.Reset();
        gVideo->Unlock();
    }

    if (_index >= _directory->GetSize())
    {
        return;
    }

    const DirItem &item = _directory->GetItem(_index);
    if (item.is_dir)
    {
        return;
    }

    if (gPlaylists->IsValid())
    {
        const std::string full_path = _GetCurrentFullPath();
        const char *label = gPlaylists->GetLabel(full_path.c_str());
        if (label)
        {
            gVideo->Lock();
            _name = label;
            gVideo->Unlock();
            return;
        }
    }

    if (!_name_map.Valid())
    {
        return;
    }

    if (item.crc32 != 0) // It's a zip or 7z package, we have crc32
    {
        gVideo->Lock();
        _name_map.GetName(item.crc32, &_name);
        gVideo->Unlock();
        return;
    }

    const ArcadeManager *arc_manager = gEmulator->GetArcadeManager();
    if (arc_manager)
    {
        // arcade rom, calc the crc32 with rom name
        char path[SCE_FIOS_PATH_MAX];
        strcpy(path, _GetCurrentFullPath().c_str());
        const char *rom_name = arc_manager->GetRomName(path);
        std::string real_name = File::GetName(rom_name);
        gVideo->Lock();
        if (!_name_map.GetName(crc32(0, (Bytef *)real_name.c_str(), real_name.size()), &_name))
        {
            real_name += ".zip";
            _name_map.GetName(crc32(0, (Bytef *)real_name.c_str(), real_name.size()), &_name);
        }
        gVideo->Unlock();
    }
    else
    {
        // calc the crc32 with read file
        SceUID thread_id = sceKernelCreateThread(__PRETTY_FUNCTION__, GetNameThread, SCE_KERNEL_DEFAULT_PRIORITY_USER, 0x4000, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL);
        uint32_t p = (uint32_t)this;
        sceKernelStartThread(thread_id, sizeof(this), (void *)&p);
    }
}

void TabBrowser::_UpdateInfo()
{
    if (_info.size() > 0)
    {
        gVideo->Lock();
        _info = "";
        gVideo->Unlock();
    }

    bool is_dir;
    const std::string full_path = _GetCurrentFullPath(&is_dir);
    if (!is_dir)
    {
        gVideo->Lock();
        _info = GetFileInfoString(full_path.c_str());
        gVideo->Unlock();
    }
}

void TabBrowser::_Update()
{
    _UpdateTexture();
    _UpdateStatus();
    _UpdateName();
    _UpdateInfo();
    _moving_status.Reset();
    _name_moving_status.Reset();
}

void TabBrowser::ChangeLanguage(uint32_t language)
{
    LogFunctionName;
    _UpdateStatus();
}

void TabBrowser::_OnKeyTriangle(Input *input)
{
    LogFunctionName;
    if (_text_dialog != nullptr)
    {
        delete _text_dialog;
    }

    _text_dialog = new InputTextDialog(TEXT(LANG_SEARCH));
    if (_text_dialog->Init())
    {
        UnsetInputHooks(input);
    }
    else
    {
        delete _text_dialog;
        _text_dialog = nullptr;
    }

    _name_map.Load();
}

void TabBrowser::_OnKeySquare(Input *input)
{
    LogFunctionName;

    auto results = _directory->GetSearchResults();
    if (results.size() == 0)
    {
        return;
    }
    else if (results.size() == 1)
    {
        _index = *(results.begin());
    }
    else
    {
        for (const auto &r : results)
        {
            if (r > _index)
            {
                _index = r;
                return;
            }
        }
        _index = *(results.begin());
    }
}

void TabBrowser::_Search(const char *s)
{

    size_t count = _directory->Search(s);
    if (count == 0)
    {
        gUi->SetHint(TEXT(LANG_NOT_FOUND));
    }
    else if (count == 1)
    {
        char utf8[64];
        snprintf(utf8, 64, "%s 1 %s", TEXT(LANG_FOUND), TEXT(LANG_FILE));
        gUi->SetHint(utf8);
    }
    else
    {
        char utf8[64];
        snprintf(utf8, 64, "%s %d %s", TEXT(LANG_FOUND), count, TEXT(LANG_FILES));
        gUi->SetHint(utf8);
    }

    _OnKeySquare(_input);
    _UpdateStatus();
}

const std::string TabBrowser::_GetCurrentFullPath(bool *is_dir)
{
    if (_index >= _directory->GetSize())
    {
        return "";
    }

    const DirItem &item = _directory->GetItem(_index);
    if (is_dir)
        *is_dir = item.is_dir;
    return _directory->GetCurrentPath() + "/" + item.name;
}

static std::map<std::string, std::string> NameCache;
#define MAX_NAME_CACHE 64

int32_t GetNameThread(uint32_t args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(TabBrowser, browser, argp);
    bool is_dir;
    const std::string full_path = browser->_GetCurrentFullPath(&is_dir);
    if (is_dir || full_path.size() == 0 || File::GetSize(full_path.c_str()) > 5000000)
    {
        sceKernelExitDeleteThread(0);
        return 0;
    }

    const char *name = nullptr;
    const auto iter = NameCache.find(full_path);
    if (iter != NameCache.end())
    {
        name = iter->second.c_str();
    }
    else if (browser->_name_map.GetName(File::GetCrc32(full_path.c_str()), &name))
    {
        if (NameCache.size() >= MAX_NAME_CACHE)
        {
            NameCache.erase(NameCache.begin());
        }
        NameCache[full_path] = name;
    }

    if (name != nullptr && full_path == browser->_GetCurrentFullPath())
    {
        gVideo->Lock();
        browser->_name = name;
        gVideo->Unlock();
    }

    sceKernelExitDeleteThread(0);
    return 0;
}