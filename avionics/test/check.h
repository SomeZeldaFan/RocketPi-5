#ifndef CHECK_H
#define CHECK_H

/* Minimal shared host-test harness (the CHECK/report header deferred in
 * D053-A7, created here by the first real test). HOST ONLY — not flight code,
 * so printf/float-compare are fine. Each test executable is one TU, so the
 * static counters below live once per test binary. */

#include <stdio.h>
#include <math.h>

static int g_checks = 0;
static int g_fails  = 0;

#define CHECK(cond, msg)                                                  \
    do {                                                                  \
        ++g_checks;                                                       \
        if (!(cond)) {                                                    \
            ++g_fails;                                                    \
            printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, (msg));      \
        }                                                                 \
    } while (0)

/* Absolute-tolerance float compare (single precision). */
#define CHECK_NEAR(a, b, tol, msg) \
    CHECK(fabsf((float)(a) - (float)(b)) <= (float)(tol), (msg))

/* Print the tally and yield a process exit code (0 = all passed). */
#define CHECK_REPORT(name)                                                \
    (printf("%-22s %3d checks, %d failed\n", (name), g_checks, g_fails),  \
     (g_fails == 0) ? 0 : 1)

#endif /* CHECK_H */
