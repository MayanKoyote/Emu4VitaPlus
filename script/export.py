#!/usr/bin/env python
from trans import Translation

lang_trans = Translation('language.json').get_trans(index='Tag')
trans_trans = Translation('translation.json').get_trans(index='English')

# Generate language_define.h
languages = None

TEXT_ENUM = []
LANGUAGE_ENUM = []

for k, v in lang_trans.items():
    attr = f'{k},'
    TEXT_ENUM.append(f'    {attr:20s} // "{v["English"]}"')
    if languages is None:
        languages = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), v.keys()))
TEXT_ENUM.append('    TEXT_COUNT,')

for i, language in enumerate(languages):
    LANGUAGE_ENUM.append(f'    LANGUAGE_{language.upper()},')
LANGUAGE_ENUM.append('    LANGUAGE_COUNT,')

TEXT_ENUM = '\n'.join(TEXT_ENUM)
LANGUAGE_ENUM = '\n'.join(LANGUAGE_ENUM)

with open('language_define.h', 'w') as fp:
    fp.write(
        f'''#ifndef LANGUAGE_DEFINE
#define LANGUAGE_DEFINE

#include <unordered_map>
#include <array>
#include <string>

enum TEXT_ENUM{{
{TEXT_ENUM}
}};

enum LANGUAGE{{
{LANGUAGE_ENUM}
}};

#endif

extern const char *gTexts[][TEXT_ENUM::TEXT_COUNT];
extern const char *gLanguageNames[];
typedef std::array<const char *, LANGUAGE::LANGUAGE_COUNT - 1> TRANS;
extern std::unordered_map<std::string, TRANS> gTrans;
'''
    )


# Generate language_define.cpp
TEXTS = []
for language in languages:
    T = []
    for k, v in lang_trans.items():
        s = f'"{v[language]}",'
        T.append(f'    {s:30s} // {k}')
    T = '\n'.join(T)
    TEXTS.append(
        f'''// {language}
{{
{T}
}},
'''
    )
TEXTS = '\n'.join(TEXTS)


NAMES = []
for language in languages:
    NAMES.append(f'    "{language}", ')
NAMES = '\n'.join(NAMES)

TRANS = []
for k, v in trans_trans.items():
    t = []
    for language in languages[1:]:
        t.append(f'"{v[language]}"')
    t = ',\n'.join(t)
    TRANS.append(f'    {{"{k}", {{{t}}},}}')
TRANS = ',\n'.join(TRANS)

with open('language_define.cpp', 'w') as fp:
    fp.write(
        f'''#include "language_define.h"

const char *gTexts[][TEXT_ENUM::TEXT_COUNT] = {{
{TEXTS}
}};

const char *gLanguageNames[] = {{
{NAMES}
}};

std::unordered_map<std::string, TRANS> gTrans = {{
{TRANS}
}};
'''
    )
