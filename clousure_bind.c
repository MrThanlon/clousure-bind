#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

static void gen_machine_code(uint8_t* code, void(* function)(void*), void* context) {
#ifdef __x86_64__
    // push %rbq
    // mov %rsp,%rbp
    memcpy(code, "\x55\x48\x89\xe5\x48\xbf", 6);
    code += 6;
    // movabs %[context],%rdi
    memcpy(code, &context, 8);
    code += 8;
    // movabs %[function],$rax
    memcpy(code, "\x48\xb8", 2);
    code += 2;
    memcpy(code, &function, 8);
    code += 8;
    // callq *%rax
    // nop
    // pop %rbq
    // retq
    memcpy(code, "\xff\xd0\x90\x5d\xc3", 5);
#elif __riscv__

#else
#error Arch not support
#endif
}

void (*clousure_bind(void(* function)(void*), void* context))(void) {
    int pagesize = getpagesize();
    if (pagesize < 0) {
        perror("getpagesize");
        return NULL;
    }
    unsigned char* ret = memalign(pagesize, pagesize);
    gen_machine_code(ret, function, context);
    if (mprotect(ret, pagesize, PROT_EXEC | PROT_WRITE | PROT_READ) < 0) {
        perror("mprotect");
        return NULL;
    }
    return (void (*)(void))ret;
}
