/*
 * Project:
 *  ___________      .___________ .__    .__ 
 * \__    ___/____  |__\_   ___ \|  |__ |__|
 *   |    |  \__  \ |  /    \  \/|  |  \|  |
 *   |    |   / __ \|  \     \___|   Y  \  |
 *   |____|  (____  /__|\______  /___|  /__|
 *                \/           \/     \/   
 *
 * Copyright (C) 2021 - 2022, Yan RuiBing, <772166784@qq.com>, et al.
 *
 */

#ifndef _TCH_CONFIG_H_INCLUDED_
#define _TCH_CONFIG_H_INCLUDED_

#include <tch_linux_config.h>

#define tch_cdecl

typedef intptr_t        tch_int_t;
typedef uintptr_t       tch_uint_t;

#define TCH_INT32_LEN   (sizeof("-2147483648") - 1)
#define TCH_INT64_LEN   (sizeof("-9223372036854775808") - 1)

#if (TCH_PTR_SIZE == 4)
#define TCH_INT_T_LEN        TCH_INT32_LEN
#define TCH_MAX_INT_T_VALUE  2147483647

#else
#define TCH_INT_T_LEN        TCH_INT64_LEN
#define TCH_MAX_INT_T_VALUE  9223372036854775807 /*7FFF,FFFF,FFFF,FFFF 这是计算机运算中最大的64位带符号整型 */
#endif

#define TCH_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define TCH_MAX_INT32_VALUE   (uint32_t) 0x7fffffff

#ifndef tch_inline
#define tch_inline           inline
#endif

#define TCH_MACNAME          255
#define TCH_UNUSED(x)        (void)(x)
#define CHUNK_SIZE           250000

#endif