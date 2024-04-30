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
#elif __riscv
#if __riscv_xlen == 64
    // RV64
    // addi    sp,sp,-32
    // sd      ra,24(sp)
    // sd      s0,16(sp)
    // addi    s0,sp,32
    memcpy(code, "\x11\x01\xec\x06\xe8\x22\x10\x00", 8);
    code += 8;
    // immediate constant context
    uint32_t highbit = (uint64_t)context & 0xfffff000UL;
    uint32_t lowbit = (uint64_t)context & 0xfffUL;
    // addi    a5,x0,%[lowbit]
    // lui     a5,%[highbit]
    // TODO
#else
    // RV32
#error Arch not support
#endif
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
