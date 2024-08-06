#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define GLOB_IMPLEMENTATION
#include "glob.h"

#define check_glob(pattern, text, expected) check_glob_located(__FILE__, __LINE__, pattern, text, expected)

void check_glob_located(const char *file, int line, const char *pattern, const char *text, glob_result_code_t expected) {
    glob_result_code_t actual = glob_mbs(pattern, text);
    printf("%12s <=> %-10s => %s\n", pattern, text, glob_result_code_str(actual));
    if (actual != expected) {
        printf("%s:%d: FAILURE! Expected %s.\n", file, line, glob_result_code_str(expected));
        exit(1);
    }
}

int main(int argc, char **argv) {
    check_glob("main.?", "main.c", GLOB_MATCHED);
    printf("\n");
    check_glob("*", "main.c", GLOB_MATCHED);
    check_glob("***", "main.c", GLOB_MATCHED);
    check_glob("*.c", "main.c", GLOB_MATCHED);
    check_glob("*.js", "main.c", GLOB_UNMATCHED);
    printf("\n");
    check_glob("*.[abc]", "main.c", GLOB_MATCHED);
    check_glob("*.[abc]", "main.b", GLOB_MATCHED);
    check_glob("*.[abc]", "main.d", GLOB_UNMATCHED);
    printf("\n");
    check_glob("*.[abc", "main.d", GLOB_SYNTAX_ERROR);
    printf("\n");
    check_glob("[][!]", "]", GLOB_MATCHED);
    check_glob("[][!]", "[", GLOB_MATCHED);
    check_glob("[][!]", "!", GLOB_MATCHED);
    printf("\n");
    check_glob("[a-c]", "a", GLOB_MATCHED);
    check_glob("[a-c]", "b", GLOB_MATCHED);
    check_glob("[a-c]", "c", GLOB_MATCHED);
    check_glob("[a-c]", "A", GLOB_UNMATCHED);
    check_glob("[a-c]", "B", GLOB_UNMATCHED);
    check_glob("[a-c]", "C", GLOB_UNMATCHED);
    printf("\n");
    check_glob("[A-Ca-c]", "A", GLOB_MATCHED);
    check_glob("[A-Ca-c]", "a", GLOB_MATCHED);
    check_glob("[A-Ca-c]", "B", GLOB_MATCHED);
    check_glob("[A-Ca-c]", "b", GLOB_MATCHED);
    check_glob("[A-Ca-c]", "C", GLOB_MATCHED);
    check_glob("[A-Ca-c]", "c", GLOB_MATCHED);
    printf("\n");
    check_glob("Letter[0-9]", "Letter0", GLOB_MATCHED);
    check_glob("Letter[0-9]", "Letter1", GLOB_MATCHED);
    check_glob("Letter[0-9]", "Letter2", GLOB_MATCHED);
    check_glob("Letter[0-9]", "Letter9", GLOB_MATCHED);
    check_glob("Letter[0-9]", "Letters", GLOB_UNMATCHED);
    check_glob("Letter[0-9]", "Letter", GLOB_UNMATCHED);
    check_glob("Letter[0-9]", "Letter10", GLOB_UNMATCHED);
    check_glob("Letter[0-9", "Letter10", GLOB_SYNTAX_ERROR);
    check_glob("Letter[0-", "Letter10", GLOB_SYNTAX_ERROR);
    printf("\n");
    check_glob("[--0]", "-", GLOB_MATCHED); // ASCII 45
    check_glob("[--0]", ".", GLOB_MATCHED);
    check_glob("[--0]", "/", GLOB_MATCHED);
    check_glob("[--0]", "0", GLOB_MATCHED); // ASCII 97
    printf("\n");
    check_glob("[$--]", "$", GLOB_MATCHED);
    check_glob("[$--]", "(", GLOB_MATCHED);
    check_glob("[$--]", ")", GLOB_MATCHED);
    check_glob("[$--]", "-", GLOB_MATCHED);
    check_glob("[$--", "-", GLOB_SYNTAX_ERROR);
    printf("\n");
    check_glob("[a-]", "-", GLOB_MATCHED);
    check_glob("[a-]", "a", GLOB_MATCHED);
    check_glob("[-c]", "-", GLOB_MATCHED);
    check_glob("[-c]", "c", GLOB_MATCHED);
    printf("\n");
    check_glob("[]-]", "]", GLOB_MATCHED);
    check_glob("[]-]", "-", GLOB_MATCHED);
    check_glob("[]-", "-", GLOB_SYNTAX_ERROR);
    printf("\n");
    check_glob("[[-b]", "[", GLOB_MATCHED);
    check_glob("[[-b]", "a", GLOB_MATCHED);
    check_glob("[[-b]", "b", GLOB_MATCHED);
    printf("\n");
    printf("\n");
    check_glob("[!ab]", "a", GLOB_UNMATCHED);
    check_glob("[!ab]", "b", GLOB_UNMATCHED);
    check_glob("[!ab]", "c", GLOB_MATCHED);
    printf("\n");
    check_glob("[!]a-]", "]", GLOB_UNMATCHED);
    check_glob("[!]a-]", "a", GLOB_UNMATCHED);
    check_glob("[!]a-]", "-", GLOB_UNMATCHED);
    check_glob("[!0-9]", "0", GLOB_UNMATCHED);
    check_glob("[!0-9]", "1", GLOB_UNMATCHED);
    check_glob("[!0-9]", "9", GLOB_UNMATCHED);
    check_glob("[!0-9]", "a", GLOB_MATCHED);
    printf("\n");
    check_glob("?", "a", GLOB_MATCHED);
    check_glob("\\?", "a", GLOB_UNMATCHED);
    check_glob("\\?", "?", GLOB_MATCHED);
    check_glob("[", "[", GLOB_SYNTAX_ERROR);
    check_glob("\\[", "[", GLOB_MATCHED);
    check_glob("\\", "\\", GLOB_SYNTAX_ERROR);
    check_glob("\\\\", "\\", GLOB_MATCHED);
    printf("\n");
    setlocale(LC_ALL, "");
    check_glob("[Пп]ривет, [Мм]ир", "Привет, Мир", GLOB_MATCHED);
    check_glob("\u06ff", "\u07ff", GLOB_UNMATCHED);
    return EXIT_SUCCESS;
}
