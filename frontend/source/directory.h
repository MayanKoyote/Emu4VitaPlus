#pragma once
#include <vector>
#include <unordered_set>
#include <string>

struct DirItem
{
    std::string name;
    bool is_dir;
    uint32_t crc32 = 0;
    std::string entry_name = "";
};

class Directory
{
public:
    Directory(const char *path = nullptr,
              const char *ext_filters = NULL,
              char split = '|');
    virtual ~Directory();

    void SetExtensionFilter(const char *exts, char split);
    bool SetCurrentPath(const std::string &path);
    const std::string &GetCurrentPath() const { return _current_path; };
    const DirItem &GetItem(int index) const { return _items[index]; };
    const std::string &GetItemName(int index) const { return _items[index].name; };
    const bool IsDir(int index) const { return _items[index].is_dir; };
    size_t GetSize();
    size_t Search(const char *s);
    bool BeFound(size_t index);
    const std::string &GetSearchString() const { return _search_str; };
    const std::vector<size_t> &GetSearchResults() const { return _search_results; };

private:
    std::vector<DirItem> _items;
    std::unordered_set<std::string> _ext_filters;
    std::vector<size_t> _search_results;
    std::string _current_path;
    std::string _search_str;

    bool _LeagleTest(const char *name, DirItem *item = nullptr);
    bool _ToRoot();
};