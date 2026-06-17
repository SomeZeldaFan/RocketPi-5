#include <stdio.h>
#include <stdlib.h>
#include "fault.h"

/*
 * test_support.c — host-only stand-in for orchestration/fault.c (D053 A7).
 *
 * Supplies the link-time system_safe_halt() (and, under -DHOST_TEST, assert_fail)
 * for the native unit-test executables so algorithm-layer ASSERT() calls link and
 * run on the host WITHOUT pulling in the real orchestration/fault.c — that one
 * drives the actuators and disables interrupts (hardware). Never built into the
 * target firmware.
 */

void system_safe_halt(void)
{
    /* abort(): SIGABRT + nonzero exit marks the test FAILED to `make test` — the
     * honest host analog of the target's halt → IWDG reset. */
    fprintf(stderr, "system_safe_halt() — fault halt on host\n");
    abort();
}

#ifdef HOST_TEST
void assert_fail(const char *file, int line, const char *func)
{
    fprintf(stderr, "ASSERT failed: %s:%d in %s()\n", file, line, func);
    system_safe_halt();
}
#endif
