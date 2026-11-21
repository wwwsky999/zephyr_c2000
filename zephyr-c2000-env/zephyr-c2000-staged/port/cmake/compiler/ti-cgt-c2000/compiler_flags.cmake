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

set_compiler_property(PROPERTY warning_base "")
set_compiler_property(PROPERTY debug -g)

# Zephyr 4.5 requests C17. cl2000 21.6 ignores that unsupported option;
# target.cmake selects the supported C11 mode explicitly.
set_compiler_property(PROPERTY cstd -std=)

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
