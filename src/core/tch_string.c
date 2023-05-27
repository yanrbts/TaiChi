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

#include <tch_config.h>
#include <tch_core.h>

static u_char *tch_sprintf_str(u_char *buf, u_char *last, u_char *src,
    size_t len, tch_uint_t hexadecimal);
static u_char *tch_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero,
    tch_uint_t hexadecimal, tch_uint_t width);


u_char *
tch_cpystrn(u_char *dst, u_char *src, size_t n)
{
    if (n == 0) {
        return dst;
    }

    while (--n) {
        *dst = *src;

        if (*dst == '\0') {
            return dst;
        }

        dst++;
        src++;
    }

    *dst = '\0';

    return dst;
}

#if (TCH_MEMCPY_LIMIT)

void *
tch_memcpy(void *dst, const void *src, size_t n)
{
    if (n > NGX_MEMCPY_LIMIT) {
        //ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "memcpy %uz bytes", n);
    }

    return memcpy(dst, src, n);
}

#endif

/*
 * supported formats:
 *    %[0][width][x][X]O        off_t
 *    %[0][width]T              time_t
 *    %[0][width][u][x|X]z      ssize_t/size_t
 *    %[0][width][u][x|X]d      int/u_int
 *    %[0][width][u][x|X]l      long
 *    %[0][width|m][u][x|X]i    ngx_int_t/ngx_uint_t
 *    %[0][width][u][x|X]D      int32_t/uint32_t
 *    %[0][width][u][x|X]L      int64_t/uint64_t
 *    %[0][width|m][u][x|X]A    ngx_atomic_int_t/ngx_atomic_uint_t
 *    %[0][width][.width]f      double, max valid number fits to %18.15f
 *    %P                        ngx_pid_t
 *    %M                        ngx_msec_t
 *    %r                        rlim_t
 *    %p                        void *
 *    %[x|X]V                   ngx_str_t *
 *    %[x|X]v                   ngx_variable_value_t *
 *    %[x|X]s                   null-terminated string
 *    %*[x|X]s                  length and string
 *    %Z                        '\0'
 *    %N                        '\n'
 *    %c                        char
 *    %%                        %
 *
 *  reserved:
 *    %t                        ptrdiff_t
 *    %S                        null-terminated wchar string
 *    %C                        wchar
 */


u_char * tch_cdecl
tch_sprintf(u_char *buf, const char *fmt, ...)
{
    u_char   *p;
    va_list   args;

    va_start(args, fmt);
    p = tch_vslprintf(buf, (void *) -1, fmt, args);
    va_end(args);

    return p;
}


u_char * tch_cdecl
tch_snprintf(u_char *buf, size_t max, const char *fmt, ...)
{
    u_char   *p;
    va_list   args;

    va_start(args, fmt);
    p = tch_vslprintf(buf, buf + max, fmt, args);
    va_end(args);

    return p;
}


u_char * tch_cdecl
tch_slprintf(u_char *buf, u_char *last, const char *fmt, ...)
{
    u_char   *p;
    va_list   args;

    va_start(args, fmt);
    p = tch_vslprintf(buf, last, fmt, args);
    va_end(args);

    return p;
}

char *
tch_right_trim(char *str)
{
    char        *endp;
    size_t       len;
    
    len = tch_strlen(str);
 
    if(len == 0) 
        return str;
 
    endp = str + len - 1;
    while(isspace(*endp)) {
        endp--;
    } 

    *(endp + 1) = '\0';
    
    return str;
}
// 自定义的格式化输出
// buf:存储数据  last:最大的内存地址  fmt:可变参数开头(format)
// %{格式描述}{输出类型描述}{数据类型描述}
u_char *tch_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args)
{
    u_char                *p, zero;                     // 占位符号 0或者空格
    int                    d;
    double                 f;
    size_t                 slen;
    int64_t                i64;                         // 保存%d
    uint64_t               ui64, frac;                  // 保存%ud
    //ngx_msec_t             ms;
    tch_uint_t             width, sign, hex, max_width, frac_width, scale, n;
    char                  *v;

    while (*fmt && buf < last) {

        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */
        // 处理fmt字符串
        if (*fmt == '%') {

            i64 = 0;
            ui64 = 0;

            zero = (u_char) ((*++fmt == '0') ? '0' : ' ');
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }

            for ( ;; ) {
                switch (*fmt){

                case 'u':
                    sign = 0;
                    fmt++;
                    continue;

                case 'm':
                    max_width = 1;
                    fmt++;
                    continue;

                case 'X':
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;
                
                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;

                case '.':
                    fmt++;

                    while (*fmt >= '0' && *fmt <= '9') {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }

                    break;

                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;

                default:
                    break;
                }
                break;
            }

            switch (*fmt) {
                
            case 'V':
                v = va_arg(args, char *);

                buf = tch_sprintf_str(buf, last, (u_char *)v, tch_strlen(v), hex);
                fmt++;

                continue;

            case 's':
                p = va_arg(args, u_char *);

                buf = tch_sprintf_str(buf, last, p, slen, hex);
                fmt++;

                continue;

            case 'O':
                i64 = (int64_t) va_arg(args, off_t);
                sign = 1;
                break;

            /*case 'P': 
                i64 = (int64_t) va_arg(args, tch_pid_t);
                sign = 1;
                break;*/

            case 'T':
                i64 = (int64_t) va_arg(args, time_t);
                sign = 1;
                break;

            case 'z':
                if (sign) {
                    i64 = (int64_t) va_arg(args, ssize_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, size_t);
                }
                break;

            case 'i':
                if (sign) {
                    i64 = (int64_t) va_arg(args, tch_int_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, tch_uint_t);
                }

                if (max_width) {
                    width = TCH_INT_T_LEN;
                }

                break;

            case 'd':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_int);
                }
                break;

            case 'l':
                if (sign) {
                    i64 = (int64_t) va_arg(args, long);
                } else {
                    ui64 = (uint64_t) va_arg(args, u_long);
                }
                break;

            case 'D':
                if (sign) {
                    i64 = (int64_t) va_arg(args, int32_t);
                } else {
                    ui64 = (uint64_t) va_arg(args, uint32_t);
                }
                break;

            case 'L':
                if (sign) {
                    i64 = va_arg(args, int64_t);
                } else {
                    ui64 = va_arg(args, uint64_t);
                }
                break;

            case 'f':
                f = va_arg(args, double);

                if (f < 0) {
                    *buf++ = '-';
                    f = -f;
                }

                ui64 = (int64_t) f;
                frac = 0;
                // 处理保留小数
                if (frac_width) {

                    scale = 1;
                    for (n = frac_width; n; n--) {
                        scale *= 10;
                    }

                    frac = (uint64_t) ((f - (double) ui64) * scale + 0.5);

                    if (frac == scale) {
                        ui64++;
                        frac = 0;
                    }
                }

                buf = tch_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width) {
                    if (buf < last) {
                        *buf++ = '.';
                    }

                    buf = tch_sprintf_num(buf, last, frac, '0', 0, frac_width);
                }

                fmt++;

                continue;

            case 'p':
                ui64 = (uintptr_t) va_arg(args, void *);
                hex = 2;
                sign = 0;
                zero = '0';
                width = 2 * sizeof(void *);
                break;

            case 'c':
                d = va_arg(args, int);
                *buf++ = (u_char) (d & 0xff);
                fmt++;

                continue;

            case 'Z':
                *buf++ = '\0';
                fmt++;

                continue;

            case 'N':
                *buf++ = LF;
                fmt++;

                continue;

            case '%':
                *buf++ = '%';
                fmt++;

                continue;

            default:
                *buf++ = *fmt++;

                continue;
            }
            // 这里只有一些整型的数字可以走下来
            // 将有符号数字类型都转换为无符号类型类型 并显示到buf中
            if (sign) {
                if (i64 < 0) {
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;

                } else {
                    ui64 = (uint64_t) i64;
                }
            }

            buf = tch_sprintf_num(buf, last, ui64, zero, hex, width);

            fmt++;
        } else {
            *buf++ = *fmt++;
        }
    }

    return buf;
}

static u_char *
tch_sprintf_str(u_char *buf, u_char *last, u_char *src,size_t len, tch_uint_t hexadecimal)
{
    static u_char   hex[] = "0123456789abcdef";
    static u_char   HEX[] = "0123456789ABCDEF";

    if (hexadecimal == 0) {

        if (len == (size_t) -1) {
            while (*src && buf < last) {
                *buf++ = *src++;
            }

        } else {
            len = tch_min((size_t) (last - buf), len);
            buf = tch_cpymem(buf, src, len);
        }

    } else if (hexadecimal == 1) {

        if (len == (size_t) -1) {

            while (*src && buf < last - 1) {
                *buf++ = hex[*src >> 4];
                *buf++ = hex[*src++ & 0xf];
            }

        } else {

            while (len-- && buf < last - 1) {
                *buf++ = hex[*src >> 4];
                *buf++ = hex[*src++ & 0xf];
            }
        }

    } else { /* hexadecimal == 2 */

        if (len == (size_t) -1) {

            while (*src && buf < last - 1) {
                *buf++ = HEX[*src >> 4];
                *buf++ = HEX[*src++ & 0xf];
            }

        } else {

            while (len-- && buf < last - 1) {
                *buf++ = HEX[*src >> 4];
                *buf++ = HEX[*src++ & 0xf];
            }
        }
    }

    return buf;
}

static u_char *
tch_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero,
    tch_uint_t hexadecimal, tch_uint_t width)
{
    u_char         *p, temp[TCH_INT64_LEN + 1];

    size_t          len;
    uint32_t        ui32;
    static u_char   hex[] = "0123456789abcdef";
    static u_char   HEX[] = "0123456789ABCDEF";

    p = temp + TCH_INT64_LEN;

    if (hexadecimal == 0) {

        if (ui64 <= (uint64_t) TCH_MAX_UINT32_VALUE) {
            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divdi3() and [u]moddi3(), they call another function
             * in its turn.  On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined multiplication and shifts.  For example,
             * unsigned "i32 / 10" is compiled to
             *
             *     (i32 * 0xCCCCCCCD) >> 35
             */

            ui32 = (uint32_t)ui64;

            do {
                *--p = (u_char) (ui32 % 10 + '0');
            } while (ui32 /= 10);

        } else {
            do {
                *--p = (u_char) (ui64 % 10 + '0');
            } while (ui64 /= 10);
        }

    } else if (hexadecimal == 1) {

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = hex[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);

    } else { /* hexadecimal == 2 */

        do {

            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = HEX[(uint32_t) (ui64 & 0xf)];

        } while (ui64 >>= 4);
    }

    /* zero or space padding */

    len = (temp + TCH_INT64_LEN) - p;

    while (len++ < width && buf < last) {
        *buf++ = zero;
    }

    /* number safe copy */

    len = (temp + TCH_INT64_LEN) - p;

    if (buf + len > last) {
        len = last - buf;
    }

    return tch_cpymem(buf, p, len);
}

char * 
tch_strrchr(const char *s, const char c)
{
    char *d = NULL;
    char *p;
    p = strrchr(s, c);
    if (p != NULL) {
        d = strndup(s, (p - s));
    }
    return d;
}

char *
tch_strncat(char *dest, const char *src, size_t n)
{
    size_t dlen = strlen(dest);
    size_t i;

    for (i = 0 ; i < n && src[i] != '\0' ; i++)
        dest[dlen + i] = src[i];
    dest[dlen + i] = '\0';

    return dest;
}