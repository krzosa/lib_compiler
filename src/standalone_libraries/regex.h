#ifndef FIRST_REGEX_HEADER
#define FIRST_REGEX_HEADER
#include <stdint.h>
#include <stdbool.h>

#ifndef RE_Int
    #define RE_Int int64_t
#endif

#ifndef RE_API
    #ifdef __cplusplus
        #define RE_API extern "C"
    #else
        #define RE_API
    #endif
#endif

#ifndef RE_StaticFunc
    #if defined(__GNUC__) || defined(__clang__)
        #define RE_StaticFunc __attribute__((unused)) static
    #else
        #define RE_StaticFunc static
    #endif
#endif

typedef struct RE_String RE_String;
typedef struct RE_Utf32Result RE_Utf32Result;
typedef struct RE_Parser RE_Parser;
typedef struct RE_Regex RE_Regex;
typedef struct RE_Match RE_Match;

/* @todo
Add \W \D \S oppsites
*/

typedef enum RE_MatchKind {
    RE_MATCH_NULL,
    RE_MATCH_FRONT,
    RE_MATCH_BACK,
    RE_MATCH_WORD,
    RE_MATCH_OR,
    RE_MATCH_GROUP,
    RE_MATCH_SELECTED,
    RE_MATCH_NOT_SELECTED,
    RE_MATCH_RANGE,
    RE_MATCH_ANY,
    RE_MATCH_ANY_WORD,
    RE_MATCH_ANY_DIGIT,
    RE_MATCH_ANY_WHITESPACE,
    RE_MATCH_ONE_OR_MORE,
    RE_MATCH_ZERO_OR_MORE,
    RE_MATCH_ZERO_OR_ONE,
} RE_MatchKind;

struct RE_Regex {
    RE_MatchKind kind;
    RE_Regex *next;
    RE_Regex *prev;

    union {
        struct {
            char word_min;
            char word_max;
        };
        char word;
        uint32_t word32;
        RE_Regex *child;
        struct {
            RE_Regex *left;
            RE_Regex *right;
        };
        struct {
            RE_Regex *first;
            RE_Regex *last;
        } group;
    };
};

struct RE_Match {
    RE_Int pos;
    RE_Int size;
};

RE_API bool RE1_AreEqual(char *regex, char *string);
RE_API bool RE2_AreEqual(RE_Regex *regex, char *string);
RE_API bool RE3_AreEqual(RE_Regex *regex, char *string, RE_Int len);
RE_API RE_Match RE1_Find(char *regex, char *string);
RE_API RE_Match RE2_Find(RE_Regex *regex, char *string);
RE_API RE_Match RE3_Find(RE_Regex *regex, char *string, RE_Int len);
RE_API RE_Match RE2_FindAgain(RE_Regex *regex, char *string, RE_Match prev_match);
RE_API RE_Match RE3_FindAgain(RE_Regex *regex, char *string, RE_Int len, RE_Match prev_match);
RE_API RE_Int RE3_MatchFront(RE_Regex *regex, char *string, RE_Int len, char *string_front);
RE_API RE_Regex *RE1_Parse(char *buff, RE_Int buffsize, char *string);
RE_API RE_Regex *RE2_Parse(char *buff, RE_Int buffsize, char *string, RE_Int len);
#endif