#pragma once
#include <stdint.h>
#include "item_selectable.h"
#include "config.h"
#include "defines.h"
#include "emulator.h"
#include "input_descriptor.h"

void ItemControlOnClick()
{
    gConfig->Save();
    gEmulator->SetupKeys();
}

class ItemControl : public ItemSelectable
{
public:
    ItemControl(ControlMapConfig *control_map)
        : ItemSelectable(Emu4Vita::Config::ControlTextMap.at(control_map->psv), "", ItemControlOnClick),
          _control_map(control_map) {};
    virtual ~ItemControl() {};

    virtual void Show(bool selected) override
    {
        ItemSelectable::Show(selected);
        ImGui::SameLine();
        _control_map->turbo ? ImGui::Text(TEXT(TURBO)) : ImGui::TextDisabled(TEXT(TURBO));
    };

    virtual void OnOption(Input *input) override
    {
        _control_map->turbo = !_control_map->turbo;
        gConfig->Save();
        gEmulator->SetupKeys();
    };

private:
    virtual size_t _GetTotalCount() override
    {
        return RETRO_KEYS_SIZE;
    };

    virtual const char *_GetOptionString(size_t index) override
    {
        return gConfig->input_descriptors.Get(RETRO_KEYS[index]);
    };

    virtual size_t _GetIndex() override
    {
        for (uint32_t i = 0; i < RETRO_KEYS_SIZE; i++)
        {
            if (RETRO_KEYS[i] == _control_map->retro)
            {
                return i;
            }
        }
        return 0;
    };

    virtual void _SetIndex(size_t index) override
    {
        _control_map->retro = RETRO_KEYS[index];
    };

    ControlMapConfig *_control_map;
};