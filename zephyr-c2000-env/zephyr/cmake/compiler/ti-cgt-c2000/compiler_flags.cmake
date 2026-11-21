# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# Compiler properties used by Zephyr's build system.  Only the flags that
# cl2000 actually implements are populated; the rest stay empty.

set_compiler_property(PROPERTY no_optimization -O0)
set_compiler_property(PROPERTY optimization_debug -O1)
set_compiler_property(PROPERTY optimization_speed -O2)
set_compiler_property(PROPERTY optimization_size -O2)
set_compiler_property(PROPERTY optimization_size_aggressive -O2)
set_compiler_property(PROPERTY optimization_fast -O2)

# Base warning mode: numeric diagnostics are enabled upstream (--diag_warning),
# so warning_base only carries the suppressions for diagnostic families emitted
# by unpatched upstream Zephyr sources/headers (each verified with
# --display_error_number).  Everything suppressible here is either a false
# positive caused by the C28x type widths (int = 16 bits) or cosmetic upstream
# patterns; genuine port/target issues are fixed in the sources instead.
#   62    - "integer operation result is out of range"  (defensive compare in
#           timeutil.c after the timeslicing.c multiply was fixed upstream-side)
#   69    - "integer conversion sign"                   (kernel/pipe.c)
#   112   - "statement is unreachable"                  (kernel/sleep.h, kernel.h)
#   183   - "argument incompatible with format string"  (kernel/mutex.c; C28x
#           int/size_t width mismatch against the %z format)
#   187   - "dynamic initialization in unreachable code" (kernel/mem_slab.c __ASSERT)
#   188   - "pointless comparison of unsigned integer with zero" (sys/timeutil.h
#           overflow guards, lib/os/clock.c, lib/utils/timeutil.c; time_t is
#           unsigned on this target)
#   190   - "enumerated type mixed with another type"   (lib/utils/rb.c)
#   2120  - "static function treated as extern"         (kernel/arch_interface.h hooks)
#   548   - "transfer of control bypasses initialization" (kernel/mutex.c goto)
#   552   - "variable was set but never used"           (kernel/init.c, stack.c,
#           msg_q.c debug counters)
#   1291  - "field is deprecated"                         (kernel/work.c,
#           k_work_schedule_for_queue/k_work_cancel_for_queue API uses)
#   1585  - "function declared noreturn does return"    (kernel/init.c,
#           kernel/sys/thread_entry.c; the C2000 path returns by design)
#   2614  - performance advice "--fp_mode=relaxed"      (lib/utils/timeutil.c)
set_compiler_property(PROPERTY warning_base
	"--diag_suppress=62,69,112,183,187,188,190,548,552,1585,2120,1291,2614")
set_compiler_property(PROPERTY debug -g)

# Zephyr 4.5 requests C17.  cl2000 22.6 has no C17/C23 modes and no
# "-std=" option family, and the requested standard cannot be expressed by
# appending ${CSTD} to a fixed prefix.  Declare no_std_flag so the generic
# "$<TARGET_PROPERTY:compiler,cstd>${CSTD}" option in CMakeLists.txt is
# suppressed; target.cmake selects the supported C11 mode explicitly.
set_compiler_property(PROPERTY cstd -std=)
set_compiler_property(PROPERTY no_std_flag true)

# autoconf.h is force-included with --preinclude <file>.
set_compiler_property(PROPERTY imacros --preinclude)
set_property(TARGET asm PROPERTY imacros --preinclude)
# lnk2000 takes .cmd command files as positional arguments; there is no -T
# equivalent, so keep TOPT empty.
set_compiler_property(PROPERTY linker_script "")

set_compiler_property(PROPERTY no_common "")
set_compiler_property(PROPERTY no_strict_aliasing "")
set_compiler_property(PROPERTY no_position_independent "")
set_compiler_property(PROPERTY no_global_merge "")

# Zephyr builds a freestanding image; cl2000 always links against the C2000
# runtime library, so there is no -ffreestanding equivalent.
set_compiler_property(PROPERTY freestanding "")
set_compiler_property(PROPERTY hosted "")
