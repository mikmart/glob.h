# glob.h

A simple glob pattern matching library in C following Alexey Kutepov's [Tsoding session VOD](https://youtu.be/B2VS_zeuTQ4) and [glob.h library](https://github.com/tsoding/glob.h), as well as Challenge 15 in the [Modern C book](https://inria.hal.science/hal-02383654v1/file/ModernC.pdf).

## Usage

```c
#include <stdio.h>
#include <locale.h>
#define GLOB_IMPLEMENTATION
#include "glob.h"

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <pattern> <text>\n", argv[0]);
        return 1;
    }

    setlocale(LC_CTYPE, ""); // For multibyte string encodings.

    const char *pattern = argv[1];
    const char *text = argv[2];

    printf("Matching \"%s\" against \"%s\"... ", pattern, text);
    switch (glob(pattern, text)) {
        case GLOB_MATCHED:
            printf("It's a match!\n");
            break;
        case GLOB_UNMATCHED:
            printf("It doesn't match.\n");
            break;
        default:
            printf("Something went wrong.\n");
    }

    return 0;
}
```

## Testing

```sh
./test.sh
```
