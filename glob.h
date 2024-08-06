#ifndef GLOB_H_
#define GLOB_H_
#include <wctype.h>

typedef enum glob_result_code_e {
    GLOB_UNMATCHED = 0,
    GLOB_MATCHED = 1,
    GLOB_SYNTAX_ERROR,
    GLOB_ENCODING_ERROR,
} glob_result_code_t;

const char *glob_result_code_str(glob_result_code_t result);

/**
 * Match a pattern against text.
 *
 * Multibyte strings are supported by converting the inputs to wide character
 * strings. Note that the conversion depends on the current locale.
 * 
 * See `strglob()` that skips the conversion but only works on single byte
 * strings, or `wcsglob()` for working directly with wide character strings.
 */
glob_result_code_t glob(const char *pattern, const char *text);
glob_result_code_t strglob(const char *pattern, const char *text);
glob_result_code_t wcsglob(const wchar_t *pattern, const wchar_t *text);

#endif // GLOB_H_


#ifdef GLOB_IMPLEMENTATION
#include <assert.h>
#include <stdbool.h>
#include <wchar.h>

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

glob_result_code_t glob(const char *pattern, const char *text) {
    glob_result_code_t result = GLOB_ENCODING_ERROR;
    wchar_t *p = strwcs(pattern);
    wchar_t *t = strwcs(text);
    if (p && t) {
        result = wcsglob(p, t);
    }
    free(p);
    free(t);
    return result;
}

static glob_result_code_t generic_glob(size_t stride, const void *p, const void *t) {
    const char *pattern = p;
    const char *text = t;
    
    while (*pattern != '\0' && *text != '\0') {
        switch (*pattern) {
            case '?': {
                pattern += stride;
                text    += stride;
            } break;
            
            case '*': {
                glob_result_code_t result = generic_glob(stride, pattern + stride, text);
                switch (result) {
                    case GLOB_ENCODING_ERROR:
                        assert(0 && "UNREACHABLE");
                    case GLOB_MATCHED:
                    case GLOB_SYNTAX_ERROR:
                        return result;
                    case GLOB_UNMATCHED: {
                        text += stride;
                    }
                }
            } break;
            
            case '[': {
                pattern += stride; // Skip [
                
                bool negate_match = false;
                if (*pattern == '!') {
                    negate_match = true;
                    pattern += stride;
                }
                
                // Remember start position for range processing.
                const char *start = pattern - stride;
                
                bool matched = false;
                do {
                    switch (*pattern) {
                        case '\0':
                            return GLOB_SYNTAX_ERROR;
                        case '-': {
                            const char *prev = pattern - stride;
                            const char *next = pattern + stride;
                            if (*next == '\0') return GLOB_SYNTAX_ERROR;
                            if (prev == start || *next == ']') {
                                // Fall through to match first/last literally.
                            } else {
                                matched |= *prev <= *text && *text <= *next;
                                pattern += stride * 2;
                                break;
                            }
                        };
                        default: {
                            matched |= *pattern == *text;
                            pattern += stride;
                        }
                    }
                } while (*pattern != ']');
                
                if (negate_match)
                    matched = !matched;
                if (!matched) return GLOB_UNMATCHED;

                pattern += stride; // Skip ]
                text    += stride;
            } break;

            case '\\': {
                pattern += stride;
                if (*pattern == '\0') return GLOB_SYNTAX_ERROR;
                // Fall through to match next character literally.
            }
            
            default: {
                if (*pattern == *text) {
                    pattern += stride;
                    text    += stride;
                } else return GLOB_UNMATCHED;
            }
        }
    }

    if (*pattern == '*') {
        pattern += stride;
    }

    return *pattern == '\0' && *text == '\0';
}

// These are not macros because I don't want to expose generic_glob().

glob_result_code_t strglob(const char *pattern, const char *text) {
    return generic_glob(sizeof(char), (void *)pattern, (void *)text);
};

glob_result_code_t wcsglob(const wchar_t *pattern, const wchar_t *text) {
    return generic_glob(sizeof(wchar_t), (void *)pattern, (void *)text);
};

#endif // GLOB_IMPLEMENTATION
