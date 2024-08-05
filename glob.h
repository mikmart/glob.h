#ifndef GLOB_H_
#define GLOB_H_

typedef enum glob_result_code_e {
    GLOB_UNMATCHED = 0,
    GLOB_MATCHED = 1,
    GLOB_SYNTAX_ERROR,
} glob_result_code_t;

const char *glob_result_code_str(glob_result_code_t result);

#define GLOB_FLAGS_EMPTY 0U
#define GLOB_FLAGS_NO_MATCH_FORWARD_SLASH (1U << 0)

#define glob(pattern, text) eglob(pattern, text, GLOB_FLAGS_EMPTY)
glob_result_code_t eglob(const char *pattern, const char *text, unsigned flags);

#endif // GLOB_H_
#ifdef GLOB_IMPLEMENTATION
#include <assert.h>
#include <stdbool.h>

const char *glob_result_code_str(glob_result_code_t result) {
    switch (result) {
        case GLOB_UNMATCHED:       return "GLOB_UNMATCHED";
        case GLOB_MATCHED:         return "GLOB_MATCHED";
        case GLOB_SYNTAX_ERROR:    return "GLOB_SYNTAX_ERROR";
    }
    assert(0 && "UNREACHABLE");
}

/**
 * @returns 0 if the pattern didn't match, 1 if it matched,
 *  or >1 if there was an error during matching.
 */
glob_result_code_t eglob(const char *pattern, const char *text, unsigned flags) {
    while (*pattern != '\0' && *text != '\0') {
        if (flags & GLOB_FLAGS_NO_MATCH_FORWARD_SLASH && *text == '/') {
            return GLOB_UNMATCHED;
        }
        switch (*pattern) {
            case '?': {
                pattern += 1;
                text    += 1;
            } break;
            
            case '*': {
                glob_result_code_t result = eglob(pattern + 1, text, flags);
                switch (result) {
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
                const char *start = pattern - 1;
                
                bool matched = false;
                do {
                    switch (*pattern) {
                        case '\0': return GLOB_SYNTAX_ERROR;
                        case '-': {
                            const char *prev = pattern - 1;
                            const char *next = pattern + 1;
                            if (*next == '\0') return GLOB_SYNTAX_ERROR;
                            if (flags & GLOB_FLAGS_NO_MATCH_FORWARD_SLASH) {
                                if (*prev == '/' || *next == '/') return GLOB_SYNTAX_ERROR;
                            }
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
}

#endif // GLOB_IMPLEMENTATION
