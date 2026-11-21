/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * C28x stdint type macros.
 *
 * cl2000 provides correct exact-width typedefs in its own <stdint.h>
 * (int32_t is long, int64_t is long long), so this header only supplies the
 * GCC-style __INTn_TYPE__ macros that Zephyr references directly.
 */

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_STDINT_H_
#define ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_STDINT_H_

#undef __INT8_TYPE__
#undef __UINT8_TYPE__
#undef __INT16_TYPE__
#undef __UINT16_TYPE__
#undef __INT32_TYPE__
#undef __UINT32_TYPE__
#undef __INT64_TYPE__
#undef __UINT64_TYPE__

/* C28x has no 8-bit type: char, short and int are all 16 bits. */
#define __INT8_TYPE__ signed char
#define __UINT8_TYPE__ unsigned char
#define __INT16_TYPE__ short int
#define __UINT16_TYPE__ unsigned short int
#define __INT32_TYPE__ long int
#define __UINT32_TYPE__ unsigned long int
#define __INT64_TYPE__ long long int
#define __UINT64_TYPE__ unsigned long long int

#undef __INT_LEAST8_TYPE__
#undef __UINT_LEAST8_TYPE__
#undef __INT_LEAST16_TYPE__
#undef __UINT_LEAST16_TYPE__
#undef __INT_LEAST32_TYPE__
#undef __UINT_LEAST32_TYPE__
#undef __INT_LEAST64_TYPE__
#undef __UINT_LEAST64_TYPE__
#define __INT_LEAST8_TYPE__ __INT8_TYPE__
#define __UINT_LEAST8_TYPE__ __UINT8_TYPE__
#define __INT_LEAST16_TYPE__ __INT16_TYPE__
#define __UINT_LEAST16_TYPE__ __UINT16_TYPE__
#define __INT_LEAST32_TYPE__ __INT32_TYPE__
#define __UINT_LEAST32_TYPE__ __UINT32_TYPE__
#define __INT_LEAST64_TYPE__ __INT64_TYPE__
#define __UINT_LEAST64_TYPE__ __UINT64_TYPE__

#undef __INT_FAST8_TYPE__
#undef __UINT_FAST8_TYPE__
#undef __INT_FAST16_TYPE__
#undef __UINT_FAST16_TYPE__
#undef __INT_FAST32_TYPE__
#undef __UINT_FAST32_TYPE__
#undef __INT_FAST64_TYPE__
#undef __UINT_FAST64_TYPE__
#define __INT_FAST8_TYPE__ __INT8_TYPE__
#define __UINT_FAST8_TYPE__ __UINT8_TYPE__
#define __INT_FAST16_TYPE__ __INT16_TYPE__
#define __UINT_FAST16_TYPE__ __UINT16_TYPE__
#define __INT_FAST32_TYPE__ __INT32_TYPE__
#define __UINT_FAST32_TYPE__ __UINT32_TYPE__
#define __INT_FAST64_TYPE__ __INT64_TYPE__
#define __UINT_FAST64_TYPE__ __UINT64_TYPE__

#undef __INTPTR_TYPE__
#undef __UINTPTR_TYPE__
#define __INTPTR_TYPE__ long int
#define __UINTPTR_TYPE__ unsigned long int

#undef __INTMAX_TYPE__
#undef __UINTMAX_TYPE__
#define __INTMAX_TYPE__ __INT64_TYPE__
#define __UINTMAX_TYPE__ __UINT64_TYPE__

#undef __INT32_C
#undef __UINT32_C
#undef __INT64_C
#undef __UINT64_C
#define __INT32_C(c) c ## L
#define __UINT32_C(c) c ## UL
#define __INT64_C(c) c ## LL
#define __UINT64_C(c) c ## ULL

#include <stdint.h>

/* TI deliberately omits exact 8-bit typedefs on C28x. Zephyr uses these names
 * for byte-oriented data, so map them to the smallest available 16-bit types.
 * Serialization code must still pack and unpack octets explicitly.
 */
#ifndef _INT8_T_DECLARED
typedef __INT8_TYPE__ int8_t;
#define _INT8_T_DECLARED
#endif
#ifndef _UINT8_T_DECLARED
typedef __UINT8_TYPE__ uint8_t;
#define _UINT8_T_DECLARED
#endif

/* TI RTS <stdint.h> defines *MAX macros only for PRU/C7000.  C28x gets the
 * 16-bit bounds (which coincidentally equal the PRU/C7000 8-bit bounds, so
 * UINT8_MAX == 0xff is correct even though uint8_t maps to a 16-bit type).
 */
#ifndef INT8_MAX
#define INT8_MAX   0x7f
#define INT8_MIN   (-INT8_MAX - 1)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX  0xff
#endif
#ifndef INT_LEAST8_MAX
#define INT_LEAST8_MAX   INT8_MAX
#define INT_LEAST8_MIN   INT8_MIN
#endif
#ifndef UINT_LEAST8_MAX
#define UINT_LEAST8_MAX  UINT8_MAX
#endif
#ifndef INT_FAST8_MAX
#define INT_FAST8_MAX   INT16_MAX
#define INT_FAST8_MIN   INT16_MIN
#endif
#ifndef UINT_FAST8_MAX
#define UINT_FAST8_MAX  UINT16_MAX
#endif

#endif /* ZEPHYR_INCLUDE_TOOLCHAIN_TI_C2000_STDINT_H_ */
