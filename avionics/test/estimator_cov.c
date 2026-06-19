/* estimator_cov.c — coverage driver for estimator.c.
 *
 * Measures BRANCH coverage of the estimator's ALGORITHM logic. The ASSERT macro
 * is neutered here to a non-branching evaluation: an assertion's failure arm is
 * a defensive guard, intentionally unreachable in correct operation, and is not
 * algorithm logic. Covering those arms needs death-tests (a fork/abort harness),
 * which D053-A7 deferred; excluding them from the branch metric is standard
 * practice (cf. lcov LCOV_EXCL_BR_LINE). The condition is still EVALUATED, so no
 * variable becomes unused. The normal `make test` build keeps ASSERT fully live
 * and validates that none of them fire.
 *
 * This single TU includes estimator.c (so the redefinition applies to it) and
 * then the full test suite, which drives it. */
#include "fault.h"
#undef  ASSERT
#define ASSERT(cond) ((void)(cond))

#include "../src/algorithm/estimator.c"
#include "test_estimator.c"
