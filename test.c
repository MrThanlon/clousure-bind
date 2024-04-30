#include <stdio.h>
#include <stdlib.h>
#include "clousure_bind.h"

static void do_something(void* param) {
    printf("%p\n", param);
}

int main(void) {
    void (*fp1)(void) = clousure_bind(do_something, (void*)0x123456789abcdef0UL);
    void (*fp2)(void) = clousure_bind(do_something, (void*)0x1145141919810UL);
    if (fp1 != NULL) {
        fp1();
    }
    if (fp2 != NULL) {
        fp2();
    }
    free((void*)fp1);
    free((void*)fp2);
    return 0;
}
