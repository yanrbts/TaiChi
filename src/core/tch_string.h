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

#ifndef _TCH_STRING_INCLUDE_
#define _TCH_STRING_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>

#define tch_strncmp(s1, s2, n)      strncmp((const char *) s1, (const char *) s2, n)
#define tch_strcmp(s1, s2)          strcmp((const char *) s1, (const char *) s2)
#define tch_strlen(s)               strlen((const char *) s)
#define tch_malloc(n)               malloc((n))
#define tch_memzero(buf, n)         (void) memset(buf, 0, n)

#if (TCH_MEMCPY_LIMIT)

void *ngx_memcpy(void *dst, const void *src, size_t n);
#define ngx_cpymem(dst, src, n)   (((u_char *) ngx_memcpy(dst, src, n)) + (n))

#else

/*
 * gcc3, msvc, and icc7 compile memcpy() to the inline "rep movs".
 * gcc3 compiles memcpy(d, s, 4) to the inline "mov"es.
 * icc8 compile memcpy(d, s, 4) to the inline "mov"es or XMM moves.
 */
#define tch_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define tch_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))
#endif

u_char *tch_cpystrn(u_char *dst, u_char *src, size_t n);
u_char *tch_cdecl tch_sprintf(u_char *buf, const char *fmt, ...);
u_char *tch_cdecl tch_snprintf(u_char *buf, size_t max, const char *fmt, ...);
u_char *tch_cdecl tch_slprintf(u_char *buf, u_char *last, const char *fmt, ...);
u_char *tch_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);

char *tch_right_trim(char *str);
char *tch_strrchr(const char *s, const char c);
char *tch_strncat(char *dest, const char *src, size_t n);

#endif