/*
Copyright 2024 Mikko Marttila
Copyright 2023 Alexey Kutepov <reximkut@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GLOB_H_
#define GLOB_H_
#include <wchar.h>
#include <stdbool.h>

/**
 * Match a glob pattern against text.
 *
 * Multibyte strings are supported by converting the inputs to wide character
 * strings. Note that the conversion depends on the current locale.
 *
 * Specialized functions give more control and return a more detailed result:
 * - `glob_mbs()` converts inputs as multibyte strings in the current locale.
 * - `glob_wcs()` takes wide character strings as inputs directly.
 */
bool glob(const char *pattern, const char *text);

typedef enum glob_result_code_e {
    GLOB_UNMATCHED = 0,
    GLOB_MATCHED = 1,
    GLOB_SYNTAX_ERROR,
    GLOB_ENCODING_ERROR,
} glob_result_code_t;

const char *glob_result_code_str(glob_result_code_t result);

glob_result_code_t glob_mbs(const char *pattern, const char *text);
glob_result_code_t glob_wcs(const wchar_t *pattern, const wchar_t *text);

#endif // GLOB_H_


#ifdef GLOB_IMPLEMENTATION
#include <stdlib.h>
#include <assert.h>

bool glob(const char *pattern, const char *text) {
    return glob_mbs(pattern, text) == GLOB_MATCHED;
}

const char *glob_result_code_str(glob_result_code_t result) {
    switch (result) {
        case GLOB_UNMATCHED:       return "GLOB_UNMATCHED";
        case GLOB_MATCHED:         return "GLOB_MATCHED";
        case GLOB_SYNTAX_ERROR:    return "GLOB_SYNTAX_ERROR";
        case GLOB_ENCODING_ERROR:  return "GLOB_ENCODING_ERROR";
    }
    assert(0 && "UNREACHABLE");
}

static wchar_t *strwcs(const char *s) {
    if (s == NULL) return 0;
    size_t len = mbstowcs(NULL, s, 0);
    if (len == -1) return 0; // Invalid multibyte sequence.
    wchar_t *w = malloc((len + 1) * sizeof *w);
    if (w == NULL) return 0;
    mbstowcs(w, s, len + 1);
    return w;
}

glob_result_code_t glob_mbs(const char *pattern, const char *text) {
    glob_result_code_t result = GLOB_ENCODING_ERROR;
    wchar_t *p = strwcs(pattern);
    wchar_t *t = strwcs(text);
    if (p && t) {
        result = glob_wcs(p, t);
    }
    free(p);
    free(t);
    return result;
}

// TODO: wchar_t -> char to skip conversion, but the macros are so awful.
glob_result_code_t glob_wcs(const wchar_t *pattern, const wchar_t *text) {
    while (*pattern != '\0' && *text != '\0') {
        switch (*pattern) {
            case '?': {
                pattern += 1;
                text    += 1;
            } break;
            
            case '*': {
                glob_result_code_t result = glob_wcs(pattern + 1, text);
                switch (result) {
                    case GLOB_ENCODING_ERROR:
                        assert(0 && "UNREACHABLE");
                    case GLOB_MATCHED:
                    case GLOB_SYNTAX_ERROR:
                        return result;
                    case GLOB_UNMATCHED: {
                        text += 1;
                    }
                }
            } break;
            
            case '[': {
                pattern += 1; // Skip [
                
                bool negate_match = false;
                if (*pattern == '!') {
                    negate_match = true;
                    pattern += 1;
                }
                
                // Remember start position for range processing.
                const wchar_t *start = pattern - 1;
                
                bool matched = false;
                do {
                    switch (*pattern) {
                        case '\0':
                            return GLOB_SYNTAX_ERROR;
                        case '-': {
                            const wchar_t *prev = pattern - 1;
                            const wchar_t *next = pattern + 1;
                            if (*next == '\0') return GLOB_SYNTAX_ERROR;
                            if (prev == start || *next == ']') {
                                // Fall through to match first/last literally.
                            } else {
                                matched |= *prev <= *text && *text <= *next;
                                pattern += 2;
                                break;
                            }
                        };
                        default: {
                            matched |= *pattern == *text;
                            pattern += 1;
                        }
                    }
                } while (*pattern != ']');
                
                if (negate_match)
                    matched = !matched;
                if (!matched) return GLOB_UNMATCHED;

                pattern += 1; // Skip ]
                text    += 1;
            } break;

            case '\\': {
                pattern += 1;
                if (*pattern == '\0') return GLOB_SYNTAX_ERROR;
                // Fall through to match next character literally.
            }
            
            default: {
                if (*pattern == *text) {
                    pattern += 1;
                    text    += 1;
                } else return GLOB_UNMATCHED;
            }
        }
    }

    if (*pattern == '*') {
        pattern += 1;
    }

    return *pattern == '\0' && *text == '\0';
};

#endif // GLOB_IMPLEMENTATION
