#ifndef __LIBSYNC_LINUX
#define __LIBSYNC_LINUX 1

#ifdef __cplusplus
#include <cstdint>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cinttypes>
#include <cstring>
#endif



#ifdef FFQ

#include "ff_queue.h"
#include "ffq_conf.h"

typedef struct ffq_pair_state mp_binding;

#else // UMP
#include "ump_conf.h"
#include "ump_common.h"

typedef struct ump_pair_state mp_binding;

#endif

#include "cycle.h"

typedef pthread_spinlock_t spinlock_t;
typedef uint32_t coreid_t;

typedef uint64_t cycles_t;
cycles_t bench_tscoverhead(void);

cycles_t bench_tsc(void);
coreid_t disp_get_core_id(void);
void domain_span_all_cores(void);

void bench_init(void);

#define BASE_PAGE_SIZE 4096U
#define UMP_QUEUE_SIZE 1000U // 64 aka one page does not work well on 815

#define USER_PANIC(x) {                         \
        printf("PANIC: " x "\n");               \
        exit(1);                                \
    }

#endif

void debug_printf(const char *fmt, ...);

static inline uint64_t rdtscp(void)
{
    uint32_t eax, edx;
    __asm volatile ("rdtscp" : "=a" (eax), "=d" (edx) :: "ecx");
    return ((uint64_t)edx << 32) | eax;
}

#define bench_tsc() rdtscp()


typedef int errval_t;