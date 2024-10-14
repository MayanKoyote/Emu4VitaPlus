#pragma once
#include <minizip/mz.h>
#include <minizip/mz_zip.h>
#include <minizip/mz_strm.h>
#include <minizip/mz_zip_rw.h>
#include "archive_reader.h"

class ArchiveReaderZip : public ArchiveReader
{
public:
    ArchiveReaderZip();
    virtual ~ArchiveReaderZip();

    bool Open(const char *path);
    void Close();
    bool First();
    bool Next();
    bool ExtractCurrent(const char *dst_path);
    const char *GetCurrentName();
    uint32_t GetCurrentCrc32();
    size_t GetCurrentSize();

private:
    void *_handle;
};