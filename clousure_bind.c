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
    code += 5;
#elif __riscv
#if __riscv_xlen == 64
    // RV64
    memcpy(
        code,
        // addi    sp,sp,-32
        "\x01\x11"
        // sd      ra,24(sp)
        "\x06\xec"
        // sd      s0,16(sp)
        "\x22\xe8"
        // addi    s0,sp,32
        "\x00\x10",
        8
    );
    code += 8;
    // immediate constant context to a5, store context to the rest of buffer
    *(uint64_t*)(code + 1024) = (uint64_t)function;
    *(uint64_t*)(code + 1032) = (uint64_t)context;
    memcpy(
        code,
        // auipc   a6,0
        "\x17\x08\x00\x00"
        // ld      a5,1024(a6)
        "\x83\x37\x08\x40"
        // ld      a0,1032(a6)
        "\x03\x35\x88\x40"
        // jalr    a5
        "\x82\x97"
        ,
        14
    );
    code += 14;
    // return
    memcpy(
        code,
        // nop
        "\x01\x00"
        // ld      ra,24(sp)
        "\xe2\x60"
        // ld      s0,16(sp)
        "\x42\x64"
        // add     sp,sp,32
        "\x05\x61"
        // ret
        "\x82\x80"
        ,
        10
    );
    code += 10;
#else
    // RV32
#error Arch not support
#endif
#else
#error Arch not support
#endif
}

static int pagesize = -1;

void (*clousure_bind(void(* function)(void*), void* context))(void) {
    if (pagesize < 0) {
        pagesize = getpagesize();
    }
    if (pagesize < 0) {
        perror("getpagesize");
        return NULL;
    }
    unsigned char* ret = memalign(pagesize, pagesize);
    gen_machine_code(ret, function, context);
    if (mprotect(ret, pagesize, PROT_EXEC | PROT_READ) < 0) {
        perror("mprotect");
        return NULL;
    }
    return (void (*)(void))ret;
}

void clousure_bind_free(void(* function)(void*)) {
    mprotect(function, pagesize, PROT_READ | PROT_WRITE);
    free((void*)function);
}