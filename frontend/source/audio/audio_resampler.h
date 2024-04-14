#pragma once
#include <stdint.h>
#include "thread_base.h"
#include "circle_buf.h"
#include "audio_define.h"
#include "audio_output.h"
#include "audio_buf.h"
#include "log.h"

extern "C"
{
#include <libswresample/swresample.h>
}

class AudioResampler : public ThreadBase
{
public:
    AudioResampler(uint32_t in_rate, uint32_t out_rate, AudioOutput *output, AudioBuf *buf);
    virtual ~AudioResampler();

    uint32_t GetOutSize(uint32_t in_size);

    void SetRate(uint32_t in_rate, uint32_t out_rate);
    void Process(const int16_t *in, uint32_t in_size);

private:
    static int _ResampleThread(SceSize args, void *argp);

    SwrContext *_swr_ctx;
    uint32_t _in_rate, _out_rate;
    CircleBuf<int16_t, 0x10000> _in_buf;
    AudioOutput *_output;
    AudioBuf *_out_buf;
};