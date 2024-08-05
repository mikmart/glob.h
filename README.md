# glob.h

Simple glob pattern matching library following [Tsoding's VOD](https://youtu.be/B2VS_zeuTQ4)
and Challenge 15 in the [Modern C book](https://inria.hal.science/hal-02383654v1/file/ModernC.pdf).

## Usage

```c
#include <stdio.h>

#define GLOB_IMPLEMENTATION
#include "glob.h"

int main(void) {
    switch (glob("*.c", "main.c")) {
        case GLOB_MATCHED: {
            printf("OK\n");
        } break;
        case GLOB_UNMATCHED: {
            printf("MATCHED\n");
        } break;
        case GLOB_SYNTAX_ERROR: {
            printf("SYNTAX ERROR\n");
        } break;
    }
}
```

## Testing

```sh
./test.sh
```
