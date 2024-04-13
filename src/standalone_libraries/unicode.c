#include "unicode.h"

#ifndef UTF__MemoryZero
    #include <string.h>
    #define UTF__MemoryZero(p, size) memset(p, 0, size)
#endif

UTF_API UTF32_Result UTF_ConvertUTF16ToUTF32(uint16_t *c, int max_advance) {
    UTF32_Result result;
    UTF__MemoryZero(&result, sizeof(result));
    if (max_advance >= 1) {
        result.advance = 1;
        result.out_str = c[0];
        if (c[0] >= 0xD800 && c[0] <= 0xDBFF && c[1] >= 0xDC00 && c[1] <= 0xDFFF) {
            if (max_advance >= 2) {
                result.out_str = 0x10000;
                result.out_str += (uint32_t)(c[0] & 0x03FF) << 10u | (c[1] & 0x03FF);
                result.advance = 2;
            }
            else
                result.error = 2;
        }
    }
    else {
        result.error = 1;
    }
    return result;
}

UTF_API UTF8_Result UTF_ConvertUTF32ToUTF8(uint32_t codepoint) {
    UTF8_Result result;
    UTF__MemoryZero(&result, sizeof(result));

    if (codepoint <= 0x7F) {
        result.len = 1;
        result.out_str[0] = (char)codepoint;
    }
    else if (codepoint <= 0x7FF) {
        result.len = 2;
        result.out_str[0] = 0xc0 | (0x1f & (codepoint >> 6));
        result.out_str[1] = 0x80 | (0x3f & codepoint);
    }
    else if (codepoint <= 0xFFFF) { // 16 bit word
        result.len = 3;
        result.out_str[0] = 0xe0 | (0xf & (codepoint >> 12)); // 4 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 6)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & codepoint);        // 6 bits
    }
    else if (codepoint <= 0x10FFFF) { // 21 bit word
        result.len = 4;
        result.out_str[0] = 0xf0 | (0x7 & (codepoint >> 18));  // 3 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 12)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & (codepoint >> 6));  // 6 bits
        result.out_str[3] = 0x80 | (0x3f & codepoint);         // 6 bits
    }
    else {
        result.error = 1;
    }

    return result;
}

UTF_API UTF32_Result UTF_ConvertUTF8ToUTF32(char *c, int max_advance) {
    UTF32_Result result;
    UTF__MemoryZero(&result, sizeof(result));

    if ((c[0] & 0x80) == 0) { // Check if leftmost zero of first byte is unset
        if (max_advance >= 1) {
            result.out_str = c[0];
            result.advance = 1;
        }
        else result.error = 1;
    }

    else if ((c[0] & 0xe0) == 0xc0) {
        if ((c[1] & 0xc0) == 0x80) { // Continuation byte required
            if (max_advance >= 2) {
                result.out_str = (uint32_t)(c[0] & 0x1f) << 6u | (c[1] & 0x3f);
                result.advance = 2;
            }
            else result.error = 2;
        }
        else result.error = 2;
    }

    else if ((c[0] & 0xf0) == 0xe0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80) { // Two continuation bytes required
            if (max_advance >= 3) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 12u | (uint32_t)(c[1] & 0x3f) << 6u | (c[2] & 0x3f);
                result.advance = 3;
            }
            else result.error = 3;
        }
        else result.error = 3;
    }

    else if ((c[0] & 0xf8) == 0xf0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80 && (c[3] & 0xc0) == 0x80) { // Three continuation bytes required
            if (max_advance >= 4) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 18u | (uint32_t)(c[1] & 0x3f) << 12u | (uint32_t)(c[2] & 0x3f) << 6u | (uint32_t)(c[3] & 0x3f);
                result.advance = 4;
            }
            else result.error = 4;
        }
        else result.error = 4;
    }
    else result.error = 4;

    return result;
}

UTF_API UTF16_Result UTF_ConvertUTF32ToUTF16(uint32_t codepoint) {
    UTF16_Result result;
    UTF__MemoryZero(&result, sizeof(result));
    if (codepoint < 0x10000) {
        result.out_str[0] = (uint16_t)codepoint;
        result.out_str[1] = 0;
        result.len = 1;
    }
    else if (codepoint <= 0x10FFFF) {
        uint32_t code = (codepoint - 0x10000);
        result.out_str[0] = (uint16_t)(0xD800 | (code >> 10));
        result.out_str[1] = (uint16_t)(0xDC00 | (code & 0x3FF));
        result.len = 2;
    }
    else {
        result.error = 1;
    }

    return result;
}

#define UTF__HANDLE_DECODE_ERROR(question_mark)                           \
    {                                                                     \
        if (outlen < buffer_size - 1) buffer[outlen++] = (question_mark); \
        break;                                                            \
    }

UTF_API int64_t UTF_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen) {
    int64_t outlen = 0;
    for (int64_t i = 0; i < inlen && in[i];) {
        UTF32_Result decode = UTF_ConvertUTF16ToUTF32((uint16_t *)(in + i), (int)(inlen - i));
        if (!decode.error) {
            i += decode.advance;
            UTF8_Result encode = UTF_ConvertUTF32ToUTF8(decode.out_str);
            if (!encode.error) {
                for (int64_t j = 0; j < encode.len; j++) {
                    if (outlen < buffer_size - 1) {
                        buffer[outlen++] = encode.out_str[j];
                    }
                }
            }
            else UTF__HANDLE_DECODE_ERROR('?');
        }
        else UTF__HANDLE_DECODE_ERROR('?');
    }

    buffer[outlen] = 0;
    return outlen;
}

UTF_API int64_t UTF_CreateWidecharFromChar(wchar_t *buffer, int64_t buffer_size, char *in, int64_t inlen) {
    int64_t outlen = 0;
    for (int64_t i = 0; i < inlen;) {
        UTF32_Result decode = UTF_ConvertUTF8ToUTF32(in + i, (int)(inlen - i));
        if (!decode.error) {
            i += decode.advance;
            UTF16_Result encode = UTF_ConvertUTF32ToUTF16(decode.out_str);
            if (!encode.error) {
                for (int64_t j = 0; j < encode.len; j++) {
                    if (outlen < buffer_size - 1) {
                        buffer[outlen++] = encode.out_str[j];
                    }
                }
            }
            else UTF__HANDLE_DECODE_ERROR(0x003f);
        }
        else UTF__HANDLE_DECODE_ERROR(0x003f);
    }

    buffer[outlen] = 0;
    return outlen;
}

UTF_API void UTF8_Advance(UTF8_Iter *iter) {
    iter->i += iter->utf8_codepoint_byte_size;
    UTF32_Result r = UTF_ConvertUTF8ToUTF32(iter->str + iter->i, iter->len - iter->i);
    if (r.error) {
        iter->item = 0;
        return;
    }

    iter->utf8_codepoint_byte_size = r.advance;
    iter->item = r.out_str;
}

UTF_API UTF8_Iter UTF8_IterateEx(char *str, int len) {
    UTF8_Iter result;
    UTF__MemoryZero(&result, sizeof(result));
    result.str = str;
    result.len = len;
    if (len) UTF8_Advance(&result);
    return result;
}

UTF_API UTF8_Iter UTF8_Iterate(char *str) {
    int length = 0;
    while (str[length]) length += 1;
    return UTF8_IterateEx(str, length);
}
