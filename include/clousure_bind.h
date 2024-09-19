#pragma once

#ifndef __CLOUSURE_BIND_H__
#define __CLOUSURE_BIND_H__

#ifdef __cplusplus
extern "C" {
#endif

void (*clousure_bind(void(* function)(void*), void* context))(void);
void clousure_bind_free(void(* function)(void*));

#ifdef __cplusplus
}
#endif

#endif