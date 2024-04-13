#ifndef FIRST_UTF_HEADER
#define FIRST_UTF_HEADER
#define UTF_HEADER
#include <stdint.h>
typedef struct UTF32_Result UTF32_Result;
typedef struct UTF8_Result UTF8_Result;
typedef struct UTF16_Result UTF16_Result;
typedef struct UTF8_Iter UTF8_Iter;

#ifndef UTF_API
    #ifdef __cplusplus
        #define UTF_API extern "C"
    #else
        #define UTF_API
    #endif
#endif

struct UTF32_Result {
    uint32_t out_str;
    int advance;
    int error;
};

struct UTF8_Result {
    uint8_t out_str[4];
    int len;
    int error;
};

struct UTF16_Result {
    uint16_t out_str[2];
    int len;
    int error;
};

struct UTF8_Iter {
    char *str;
    int len;
    int utf8_codepoint_byte_size;
    int i;
    uint32_t item;
};

UTF_API UTF32_Result UTF_ConvertUTF16ToUTF32(uint16_t *c, int max_advance);
UTF_API UTF8_Result UTF_ConvertUTF32ToUTF8(uint32_t codepoint);
UTF_API UTF32_Result UTF_ConvertUTF8ToUTF32(char *c, int max_advance);
UTF_API UTF16_Result UTF_ConvertUTF32ToUTF16(uint32_t codepoint);
UTF_API int64_t UTF_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen);
UTF_API int64_t UTF_CreateWidecharFromChar(wchar_t *buffer, int64_t buffer_size, char *in, int64_t inlen);
UTF_API void UTF8_Advance(UTF8_Iter *iter);
UTF_API UTF8_Iter UTF8_IterateEx(char *str, int len);
UTF_API UTF8_Iter UTF8_Iterate(char *str);

#define UTF8_For(name, str, len) for (UTF8_Iter name = UTF8_IterateEx(str, (int)len); name.item; UTF8_Advance(&name))
#endif