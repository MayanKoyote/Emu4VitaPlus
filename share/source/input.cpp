#include <psp2/ctrl.h>
#include <string.h>
#include "input.h"
#include "log.h"

#define N_CTRL_PORTS 4
#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64

void Touch::Enable(bool enable)
{
    LogFunctionName;
    _enabled = enable;
    if (enable)
    {
        sceTouchGetPanelInfo(_port, &_info);
        sceTouchSetSamplingState(_port, SCE_TOUCH_SAMPLING_STATE_START);
        LogDebug("%d:\n"
                 "   %d %d %d %d\n"
                 "   %d %d %d %d",
                 _port,
                 _info.minAaX, _info.minAaY, _info.maxAaX, _info.maxAaY,
                 _info.minDispX, _info.minDispY, _info.maxDispX, _info.maxDispY);
    }
    else
    {
        sceTouchSetSamplingState(_port, SCE_TOUCH_SAMPLING_STATE_STOP);
    }
}

void Touch::Poll()
{
    if (!_enabled)
    {
        return;
    }

    SceTouchData touch_data{0};
    if (sceTouchPeek(_port, &touch_data, 1) == 1)
    {

        _last_id = _current_id;
        _current_id = touch_data.report->id;
        _last_axis.x = _axis.x;
        _last_axis.y = _axis.y;
        _axis.x = touch_data.report->x >> 1;
        _axis.y = touch_data.report->y >> 1;
    }
}

Input::Input() : _last_key(0ull),
                 _turbo_key(0ull),
                 _turbo_start_ms(DEFAULT_TURBO_START_TIME),
                 _turbo_interval_ms(DEFAULT_TURBO_INTERVAL),
                 _enable_key_up(true),
                 _left_analog{0},
                 _right_analog{0}
{
    memset(_turbo_key_states, 0, sizeof(_turbo_key_states));

    if (_analog_map_table[0] != ANALOG_PSV_TO_RETRO(0) || _analog_map_table[0xff] != ANALOG_PSV_TO_RETRO(0xff))
    {
        int16_t *p = _analog_map_table;
        {
            for (size_t i = 0; i < 0x100; i++)
            {
                *p++ = ANALOG_PSV_TO_RETRO(i);
            }
        }
    }
}

int16_t Input::_analog_map_table[0x100] = {0};

Input::~Input()
{
}

void Input::SetKeyDownCallback(uint32_t key, InputFunc func, bool turbo)
{
    if (func == nullptr)
    {
        UnsetKeyDownCallback(key);
    }
    else
    {
        if (turbo)
        {
            _turbo_key |= key;
        }

        for (auto &iter : _key_down_callbacks)
        {
            if (iter.key == key)
            {
                iter.func = func;
                return;
            }
        }

        _key_down_callbacks.push_back({key, func});
    }
}

void Input::SetKeyUpCallback(uint32_t key, InputFunc func)
{
    if (func == nullptr)
    {
        UnsetKeyUpCallback(key);
    }
    else
    {
        for (auto &iter : _key_up_callbacks)
        {
            if (iter.key == key)
            {
                iter.func = func;
                return;
            }
        }

        _key_up_callbacks.push_back({key, func});
    }
}

void Input::UnsetKeyUpCallback(uint32_t key)
{
    for (auto iter = _key_up_callbacks.begin(); iter != _key_up_callbacks.end(); iter++)
    {
        if (iter->key == key)
        {
            _key_up_callbacks.erase(iter);
            break;
        }
    }
}

void Input::UnsetKeyDownCallback(uint32_t key)
{
    for (auto iter = _key_down_callbacks.begin(); iter != _key_down_callbacks.end(); iter++)
    {
        if (iter->key == key)
        {
            _key_down_callbacks.erase(iter);
            break;
        }
    }

    _turbo_key &= ~key;
}

void Input::Poll(bool waiting)
{
    SceCtrlData ctrl_data{0};
    if ((waiting ? sceCtrlReadBufferPositiveExt2(0, &ctrl_data, 1) : sceCtrlPeekBufferPositiveExt2(0, &ctrl_data, 1)) > 0)
    {

        uint32_t key = ctrl_data.buttons;
        if (ctrl_data.lx < (ANALOG_CENTER - ANALOG_THRESHOLD))
            key |= SCE_CTRL_LSTICK_LEFT;
        else if (ctrl_data.lx > (ANALOG_CENTER + ANALOG_THRESHOLD))
            key |= SCE_CTRL_LSTICK_RIGHT;

        if (ctrl_data.ly < (ANALOG_CENTER - ANALOG_THRESHOLD))
            key |= SCE_CTRL_LSTICK_UP;
        else if (ctrl_data.ly > (ANALOG_CENTER + ANALOG_THRESHOLD))
            key |= SCE_CTRL_LSTICK_DOWN;

        if (ctrl_data.rx < (ANALOG_CENTER - ANALOG_THRESHOLD))
            key |= SCE_CTRL_RSTICK_LEFT;
        else if (ctrl_data.rx > (ANALOG_CENTER + ANALOG_THRESHOLD))
            key |= SCE_CTRL_RSTICK_RIGHT;

        if (ctrl_data.ry < (ANALOG_CENTER - ANALOG_THRESHOLD))
            key |= SCE_CTRL_RSTICK_UP;
        else if (ctrl_data.ry > (ANALOG_CENTER + ANALOG_THRESHOLD))
            key |= SCE_CTRL_RSTICK_DOWN;

        _left_analog.x = ctrl_data.lx;
        _left_analog.y = ctrl_data.ly;
        _right_analog.x = ctrl_data.rx;
        _right_analog.y = ctrl_data.ry;

        key = _ProcTurbo(key);
        _ProcCallbacks(key);

        _last_key = key;
    }

    _front_touch.Poll();
    _rear_touch.Poll();
}

/*
Up                        ______________________                      _______________
                         |                     |                     |
         _turbo_start_ms |  _turbo_interval_ms |  _turbo_interval_ms | ......
Down   __________________|                     |_____________________|
*/
uint32_t Input::_ProcTurbo(uint32_t key)
{
    if (!_turbo_key)
    {
        return key;
    }

    uint64_t current = sceKernelGetProcessTimeWide();

    for (int i = 0; i < 32; i++)
    {
        uint32_t k = 1 << i;
        if (k & _turbo_key)
        {
            TurboKeyState *state = _turbo_key_states + i;

            if (k & key)
            {
                if ((k & ~_last_key) && state->next_change_state_time == 0ll)
                {
                    // first keydown
                    state->next_change_state_time = current + _turbo_start_ms;
                    state->down = true;
                }
                else if (current >= state->next_change_state_time)
                {
                    // change state
                    state->next_change_state_time += _turbo_interval_ms;
                    state->down = !state->down;
                }

                if (!state->down)
                {
                    key &= ~k;
                }
            }
            else if (state->next_change_state_time != 0ll)
            {
                state->next_change_state_time = 0ll;
                state->down = false;
            }
        }
    }

    return key;
}

void Input::_ProcCallbacks(uint32_t key)
{
    if (key != _last_key)
    {
        // LogDebug("key %08x", key);
        for (const auto &iter : _key_down_callbacks)
        {
            if (TEST_KEY(iter.key, key) && !TEST_KEY(iter.key, _last_key))
            {
                // LogDebug("  call down: %08x %08x", iter.first, iter.second);
                iter.func(this);
                break;
            }
        }

        if (_enable_key_up)
        {
            for (const auto &iter : _key_up_callbacks)
            {
                // LogDebug("_key_up_callbacks %08x %08x", iter.first, _last_key);
                if (TEST_KEY(iter.key, _last_key) && !TEST_KEY(iter.key, key))
                {
                    // LogDebug("  call up: %08x %08x", iter.first, iter.second);
                    iter.func(this);
                    if (key)
                    {
                        _enable_key_up = false;
                    }
                    break;
                }
            }
        }
        else if (!key)
        {
            _enable_key_up = true;
        }
    }
}

void Input::SetTurbo(uint32_t key)
{
    _turbo_key |= key;
}

void Input::UnsetTurbo(uint32_t key)
{
    _turbo_key &= ~key;
}

void Input::SetTurboInterval(uint64_t turbo_start, uint64_t turbo_interval)
{
    _turbo_start_ms = turbo_start;
    _turbo_interval_ms = turbo_interval;
}

void Input::Reset()
{
    _turbo_key = 0;
    _key_down_callbacks.clear();
    _key_up_callbacks.clear();
    _turbo_start_ms = DEFAULT_TURBO_START_TIME;
    _turbo_interval_ms = DEFAULT_TURBO_INTERVAL;
}

void Input::PushCallbacks()
{
    _callback_stack.push(_key_up_callbacks);
    _callback_stack.push(_key_down_callbacks);
    _key_down_callbacks.clear();
    _key_up_callbacks.clear();
}

void Input::PopCallbacks()
{
    _key_down_callbacks.clear();
    _key_up_callbacks.clear();
    if (_callback_stack.size() > 0)
    {
        _key_down_callbacks = _callback_stack.top();
        _callback_stack.pop();
    }
    else
    {
        LogWarn("_callback_stack is empty, can't pop to _key_down_callbacks");
    }

    if (_callback_stack.size() > 0)
    {
        _key_up_callbacks = _callback_stack.top();
        _callback_stack.pop();
    }
    else
    {
        LogWarn("_callback_stack is empty, can't pop to _key_up_callbacks");
    }
}
