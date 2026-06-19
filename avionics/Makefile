# avionics/Makefile — host (dev-PC) build for native algorithm unit tests.
#
# Target: macOS / Apple Silicon (Apple clang); builds and runs natively on the M3.
# This is NOT the STM32F407 firmware build (that is arm-none-eabi-gcc + CubeMX
# HAL/startup/linker, deferred). It builds only the platform-independent algorithm
# layer + per-module test harnesses, which include nothing below avionics_types.h
# and therefore carry zero HAL dependency.

CC    = cc
INC   = inc
BUILD = build

# Flags — each justified (no silent defaults):
#   -std=c11            code uses C11 (_Static_assert, <stdbool.h>); pin it.
#   -pedantic           reject extensions — the same sources must also compile under
#                       arm-none-eabi-gcc; an extension here would diverge the builds.
#   -Wall -Wextra       baseline diagnostics.
#   -Werror             JPL Power-of-10 Rule 10 (locked standard): zero warnings.
#                       Host-dev knob — relax locally while iterating if it gets in the way.
#   -Wconversion        avionics types mix float/uint; implicit narrowing is a real bug
#     -Wsign-conversion class here. Aggressive — may need dial-back once heavy math lands.
#   -Wshadow -Wpointer-arith -Wcast-qual   JPL-aligned hygiene.
#   -g -O0              host tests exist to debug algorithm math: favour debuggability.
#   -DHOST_TEST         selects fault.h's host ASSERT branch (file/line/func to stderr).
CFLAGS = -std=c11 -I$(INC) -DHOST_TEST -g -O0 \
         -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion \
         -Wshadow -Wpointer-arith -Wcast-qual

SUPPORT = test/test_support.c
ALG     = src/algorithm/estimator.c src/algorithm/fdir.c src/algorithm/control_law.c

EXES = $(BUILD)/estimator_test \
       $(BUILD)/fdir_test \
       $(BUILD)/control_law_test \
       $(BUILD)/pipeline_test

all: $(EXES)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/estimator_test: test/test_estimator.c src/algorithm/estimator.c $(SUPPORT) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/fdir_test: test/test_fdir.c src/algorithm/fdir.c $(SUPPORT) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/control_law_test: test/test_control_law.c src/algorithm/control_law.c $(SUPPORT) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

# Integrated pipeline harness — links the full algorithm layer (test_main.c is the
# integrated entry point; the algorithm objects are present for when it drives them).
$(BUILD)/pipeline_test: test/test_main.c $(ALG) $(SUPPORT) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

test: $(EXES)
	@fail=0; \
	for t in $(EXES); do \
		printf '  %-28s ' "$$t"; \
		if "$$t"; then echo "PASS"; else echo "FAIL"; fail=1; fi; \
	done; \
	if [ $$fail -eq 0 ]; then echo "All host tests passed."; \
	else echo "Some host tests FAILED."; exit 1; fi

clean:
	rm -rf $(BUILD)

# Branch-coverage report per algorithm module. SEPARATE instrumented builds
# (Apple clang source-based coverage) — the strict production build above is
# untouched, so --coverage never pollutes -Werror. G1 bar: >= 90% branch.
# ASSERT is neutered in the *_cov.c drivers (defensive failure arms are not
# algorithm logic; death-tests deferred D053-A7), so the metric reflects the
# reachable algorithm branches only.
coverage: | $(BUILD)
	$(CC) $(CFLAGS) -fprofile-instr-generate -fcoverage-mapping \
	    test/estimator_cov.c $(SUPPORT) -o $(BUILD)/estimator_cov
	cd $(BUILD) && LLVM_PROFILE_FILE=est.profraw ./estimator_cov >/dev/null
	xcrun llvm-profdata merge -sparse $(BUILD)/est.profraw -o $(BUILD)/est.profdata
	xcrun llvm-cov report $(BUILD)/estimator_cov \
	    -instr-profile=$(BUILD)/est.profdata src/algorithm/estimator.c
	$(CC) $(CFLAGS) -fprofile-instr-generate -fcoverage-mapping \
	    test/fdir_cov.c $(SUPPORT) -o $(BUILD)/fdir_cov
	cd $(BUILD) && LLVM_PROFILE_FILE=fdir.profraw ./fdir_cov >/dev/null
	xcrun llvm-profdata merge -sparse $(BUILD)/fdir.profraw -o $(BUILD)/fdir.profdata
	xcrun llvm-cov report $(BUILD)/fdir_cov \
	    -instr-profile=$(BUILD)/fdir.profdata src/algorithm/fdir.c

.PHONY: all test clean coverage
