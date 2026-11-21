/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Toolchain abstraction for the TI C2000 code generation tools (cl2000).
 *
 * C28x is a word-addressed target: CHAR_BIT is 16, sizeof(int) is 1 (16 bits),
 * sizeof(long) is 2 (32 bits) and pointers are 32 bits wide.  cl2000 is an
 * EDG-based compiler that accepts most GNU __attribute__ spellings but has no
 * packed attribute, no GNU asm operands and no __GNUC__ predefines.
 */

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_H_
#define ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_H_

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_H_
#error Please do not include toolchain-specific headers directly, use <zephyr/toolchain.h> instead
#endif

#define TOOLCHAIN_GCC_VERSION 0
#define TOOLCHAIN_HAS_C_AUTO_TYPE 0
#define TOOLCHAIN_HAS_ZLA 0
#define TOOLCHAIN_HAS_PRAGMA_DIAG 0

/* cl2000 in C11 mode does not treat bool as a keyword.  Pull in the RTS
 * stdbool.h so architecture headers that use bool (e.g. arch_interface.h)
 * compile regardless of include order.
 */
#include <stdbool.h>

/* Pull in the TI RTS stdint.h so standard *MAX macros (UINT8_MAX etc.) are
 * available to files that do not explicitly #include <stdint.h> but rely on
 * them being transitively present through <zephyr/toolchain.h>.
 */
#include <stdint.h>

/* TI RTS errno.h omits several POSIX errno values that Zephyr uses. */
#include <errno.h>
#ifndef EALREADY
#define EALREADY 114
#endif
#ifndef ENOMSG
#define ENOMSG 0x003D
#endif
#ifndef ENODATA
#define ENODATA 0x003A
#endif
#ifndef EOVERFLOW
#define EOVERFLOW 0x0084
#endif

/* TI RTS does not declare the POSIX strnlen() used by Zephyr's printf core
 * (lib/os/cbprintf_complete.c), even though the RTS library provides the
 * symbol.  Declare it here so no translation unit sees an implicit
 * declaration (an implicit "int" return would truncate the 32-bit size_t
 * result on C28x).
 */
#include <stddef.h>
size_t strnlen(const char *s, size_t maxlen);

/* cl2000 defines __LITTLE_ENDIAN__ but not the __ORDER_* / __BYTE_ORDER__
 * macros that zephyr/toolchain.h validates.
 */
#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ 1
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 2
#endif
#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif

#undef BUILD_ASSERT
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define BUILD_ASSERT(EXPR, MSG...) static_assert(EXPR, "" MSG)
#elif !defined(__cplusplus)
#define BUILD_ASSERT(EXPR, MSG...) _Static_assert((EXPR), "" MSG)
#else
#define BUILD_ASSERT(EXPR, MSG...)
#endif

#ifdef __cplusplus
#define ZRESTRICT __restrict
#else
#define ZRESTRICT restrict
#endif

#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, return_type) \
	return_type new_alias(void) ALIAS_OF(real_func)

/* cl2000 has no __builtin_unreachable(). */
#define CODE_UNREACHABLE
#define FUNC_NORETURN __attribute__((noreturn))

/* cl2000 does not support stack-protector control attributes. */
#define FUNC_NO_STACK_PROTECTOR

/* cl2000 cannot place functions in arbitrary named sections through
 * __attribute__((section("..."))) when the section name comes from a
 * Zephyr macro chain.  __boot_func etc. are no-ops for stage 2.
 */
#ifndef __boot_func
#define __boot_func
#endif

#define __GENERIC_SECTION(segment) __attribute__((section(STRINGIFY(segment))))
#define Z_GENERIC_SECTION(segment) __GENERIC_SECTION(segment)
#define __GENERIC_DOT_SECTION(segment) \
	__attribute__((section(STRINGIFY(.segment))))
#define Z_GENERIC_DOT_SECTION(segment) __GENERIC_DOT_SECTION(segment)

#define ___in_section(a, b, c) \
	__attribute__((section("." Z_STRINGIFY(a) "." Z_STRINGIFY(b) "." Z_STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

#define ___in_section_unique(a, b) \
	__attribute__((section("." Z_STRINGIFY(a) "." Z_STRINGIFY(b))))
#define __in_section_unique(seg) ___in_section_unique(seg, __COUNTER__)
#define __in_section_unique_named(seg, name) ___in_section_unique(seg, name)

/* XIP images execute from flash; no RAM function placement in stage 1. */
#define __ramfunc

/* cl2000 has no packed attribute and no #pragma pack.  Structures keep their
 * natural alignment; wire-format structures must not be mapped onto C28x
 * structs directly.
 */
#undef __packed
#define __packed
#undef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#undef __noinline
#define __noinline __attribute__((noinline))
#define __may_alias
#define __printf_like(f, a) __attribute__((format(printf, f, a)))
#undef __used
#define __used __attribute__((used))
#undef __unused
#define __unused __attribute__((unused))
#define __maybe_unused __attribute__((unused))
#define __deprecated __attribute__((deprecated))
#define __deprecated_version(version) __attribute__((deprecated))
#define __attribute_const__ __attribute__((const))
#define __must_check __attribute__((warn_unused_result))
#define __attribute_nonnull(...)
#define __cleanup(x)
#define __fallthrough
#define __no_optimization
#define __weak __attribute__((weak))
#define __noasan
#define __noubsan

#define ARG_UNUSED(x) (void)(x)
#define likely(x) (x)
#define unlikely(x) (x)

/* No __builtin_popcount/clz on cl2000; use portable helpers instead. */
#ifndef _ASMLANGUAGE
static inline unsigned int z_c2000_popcount(unsigned int v)
{
	unsigned int n = 0;

	while (v != 0U) {
		v &= (v - 1U);
		n++;
	}
	return n;
}

static inline unsigned int z_c2000_clz32(unsigned long v)
{
	unsigned int n = 0;

	if (v == 0UL) {
		return 32U;
	}
	while ((v & 0x80000000UL) == 0UL) {
		v <<= 1;
		n++;
	}
	return n;
}

static inline unsigned int z_c2000_ffs32(unsigned long v)
{
	unsigned int n = 1;

	if (v == 0UL) {
		return 0U;
	}
	while ((v & 1UL) == 0UL) {
		v >>= 1;
		n++;
	}
	return n;
}

static inline unsigned int z_c2000_ctz32(unsigned long v)
{
	unsigned int n = 0;

	while ((v & 1UL) == 0UL) {
		v >>= 1;
		n++;
	}
	return n;
}
#endif

#define POPCOUNT(x) z_c2000_popcount(x)
#ifndef _ASMLANGUAGE
#define __builtin_clz(x)  z_c2000_clz32((unsigned long)(x))
#define __builtin_clzl(x) z_c2000_clz32((unsigned long)(x))
#define __builtin_clzll(x) z_c2000_clz32((unsigned long)(x))
#define __builtin_ffs(x)  z_c2000_ffs32((unsigned long)(x))
#define __builtin_ctz(x)  z_c2000_ctz32((unsigned long)(x))
#define __builtin_ctzl(x) z_c2000_ctz32((unsigned long)(x))
#define __builtin_ctzll(x) z_c2000_ctz32((unsigned long)(x))
#endif

/*
 * Absolute symbols for gen_offset_header.py.
 *
 * GCC emits ".equ name, value" through extended asm operands.  cl2000 only
 * accepts literal asm strings, so the value is encoded as the size of a
 * one-dimensional array placed in .zephyr_abs: the ELF symbol size (in
 * octets) is 2 * (value + 1) because one C28x char occupies two octets.
 * scripts/build/gen_offset_header.py decodes this back to a word offset.
 */
#define GEN_OFFSET_EXTERN(name) extern const char name[]

/* Unlike GCC's asm encoding, the array definitions can live at file scope. */
#define GEN_ABS_SYM_BEGIN(name)
#define GEN_ABS_SYM_END

#define GEN_ABSOLUTE_SYM(name, value) \
	const char name[(unsigned int)(value) + 1] \
		__attribute__((section(".zephyr_abs"), used))

#define GEN_ABSOLUTE_SYM_KCONFIG(name, value) GEN_ABSOLUTE_SYM(name, value)

/* cl2000 asm() takes a single string; a comment acts as a scheduling barrier
 * without emitting instructions.
 */
#define compiler_barrier() do { __asm("  ; compiler barrier"); } while (false)

/* Z_POW2_CEIL must be usable in constant expressions (e.g. BUILD_ASSERT on
 * sizeof-derived values), so it cannot call z_c2000_clz32().  Use a ternary
 * chain instead; it is correct for both compile-time and runtime inputs.
 */
#define Z_POW2_CEIL(x) \
	((x) <= 1UL   ? 1UL    : \
	 (x) <= 2UL   ? 2UL    : \
	 (x) <= 4UL   ? 4UL    : \
	 (x) <= 8UL   ? 8UL    : \
	 (x) <= 16UL  ? 16UL   : \
	 (x) <= 32UL  ? 32UL   : \
	 (x) <= 64UL  ? 64UL   : \
	 (x) <= 128UL ? 128UL  : \
	 (x) <= 256UL ? 256UL  : \
	 (x) <= 512UL ? 512UL  : \
	 (x) <= 1024UL ? 1024UL : \
	 (x) <= 2048UL ? 2048UL : \
	 (x) <= 4096UL ? 4096UL : \
	 (x) <= 8192UL ? 8192UL : \
	 (x) <= 16384UL ? 16384UL : \
	 (x) <= 32768UL ? 32768UL : \
	 (x) <= 65536UL ? 65536UL : 0UL)
#define Z_IS_POW2(x) (((x) != 0) && (((x) & ((x)-1)) == 0))

#define __WARN(msg)
#define __DEPRECATED_MACRO

#if defined(_ASMLANGUAGE)
/* TI C28x assembler spellings. */
#define GTEXT(sym) .global sym
#define GDATA(sym) .global sym
#define WTEXT(sym) .weak sym
#define WDATA(sym) .weak sym
#define FUNC_CODE()
#define FUNC_INSTR(a)
#endif

#include <zephyr/toolchain/common.h>

#endif /* ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_H_ */
