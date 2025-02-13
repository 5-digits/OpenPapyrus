/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

//#include <stdlib.h>

#undef c2l
#define c2l(c,l)        (l =((ulong)(*((c)++)))    , \
                         l|=((ulong)(*((c)++)))<< 8L, \
                         l|=((ulong)(*((c)++)))<<16L, \
                         l|=((ulong)(*((c)++)))<<24L)

/* NOTE - c is not incremented as per c2l */
#undef c2ln
#define c2ln(c,l1,l2,n) { \
                        c+=n; \
                        l1=l2=0; \
                        switch (n) { \
                        case 8: l2 =((ulong)(*(--(c))))<<24L; \
                        case 7: l2|=((ulong)(*(--(c))))<<16L; \
                        case 6: l2|=((ulong)(*(--(c))))<< 8L; \
                        case 5: l2|=((ulong)(*(--(c))));     \
                        case 4: l1 =((ulong)(*(--(c))))<<24L; \
                        case 3: l1|=((ulong)(*(--(c))))<<16L; \
                        case 2: l1|=((ulong)(*(--(c))))<< 8L; \
                        case 1: l1|=((ulong)(*(--(c))));     \
                                } \
                        }

#undef l2c
#define l2c(l,c)        (*((c)++)=(uchar)(((l)     )&0xff), \
                         *((c)++)=(uchar)(((l)>> 8L)&0xff), \
                         *((c)++)=(uchar)(((l)>>16L)&0xff), \
                         *((c)++)=(uchar)(((l)>>24L)&0xff))

/* NOTE - c is not incremented as per l2c */
#undef l2cn
#define l2cn(l1,l2,c,n) { \
                        c+=n; \
                        switch (n) { \
                        case 8: *(--(c))=(uchar)(((l2)>>24L)&0xff); \
                        case 7: *(--(c))=(uchar)(((l2)>>16L)&0xff); \
                        case 6: *(--(c))=(uchar)(((l2)>> 8L)&0xff); \
                        case 5: *(--(c))=(uchar)(((l2)     )&0xff); \
                        case 4: *(--(c))=(uchar)(((l1)>>24L)&0xff); \
                        case 3: *(--(c))=(uchar)(((l1)>>16L)&0xff); \
                        case 2: *(--(c))=(uchar)(((l1)>> 8L)&0xff); \
                        case 1: *(--(c))=(uchar)(((l1)     )&0xff); \
                                } \
                        }

/* NOTE - c is not incremented as per n2l */
#define n2ln(c,l1,l2,n) { \
                        c+=n; \
                        l1=l2=0; \
                        switch (n) { \
                        case 8: l2 =((ulong)(*(--(c))))    ; \
                        case 7: l2|=((ulong)(*(--(c))))<< 8; \
                        case 6: l2|=((ulong)(*(--(c))))<<16; \
                        case 5: l2|=((ulong)(*(--(c))))<<24; \
                        case 4: l1 =((ulong)(*(--(c))))    ; \
                        case 3: l1|=((ulong)(*(--(c))))<< 8; \
                        case 2: l1|=((ulong)(*(--(c))))<<16; \
                        case 1: l1|=((ulong)(*(--(c))))<<24; \
                                } \
                        }

/* NOTE - c is not incremented as per l2n */
#define l2nn(l1,l2,c,n) { \
                        c+=n; \
                        switch (n) { \
                        case 8: *(--(c))=(uchar)(((l2)    )&0xff); \
                        case 7: *(--(c))=(uchar)(((l2)>> 8)&0xff); \
                        case 6: *(--(c))=(uchar)(((l2)>>16)&0xff); \
                        case 5: *(--(c))=(uchar)(((l2)>>24)&0xff); \
                        case 4: *(--(c))=(uchar)(((l1)    )&0xff); \
                        case 3: *(--(c))=(uchar)(((l1)>> 8)&0xff); \
                        case 2: *(--(c))=(uchar)(((l1)>>16)&0xff); \
                        case 1: *(--(c))=(uchar)(((l1)>>24)&0xff); \
                                } \
                        }

#undef n2l
#define n2l(c,l)        (l =((ulong)(*((c)++)))<<24L, \
                         l|=((ulong)(*((c)++)))<<16L, \
                         l|=((ulong)(*((c)++)))<< 8L, \
                         l|=((ulong)(*((c)++))))

#undef l2n
#define l2n(l,c)        (*((c)++)=(uchar)(((l)>>24L)&0xff), \
                         *((c)++)=(uchar)(((l)>>16L)&0xff), \
                         *((c)++)=(uchar)(((l)>> 8L)&0xff), \
                         *((c)++)=(uchar)(((l)     )&0xff))

#if (defined(OPENSSL_SYS_WIN32) && defined(_MSC_VER))
# define ROTATE_l32(a,n)     _lrotl(a,n)
# define ROTATE_r32(a,n)     _lrotr(a,n)
#elif defined(__ICC)
# define ROTATE_l32(a,n)     _rotl(a,n)
# define ROTATE_r32(a,n)     _rotr(a,n)
#elif defined(__GNUC__) && __GNUC__>=2 && !defined(__STRICT_ANSI__) && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM) && !defined(PEDANTIC)
# if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#  define ROTATE_l32(a,n)       ({ register unsigned int ret;   \
                                        asm ("roll %%cl,%0"     \
                                                : "=r"(ret)     \
                                                : "c"(n),"0"((uint)(a)) \
                                                : "cc");        \
                                        ret;                    \
                                })
#  define ROTATE_r32(a,n)       ({ register unsigned int ret;   \
                                        asm ("rorl %%cl,%0"     \
                                                : "=r"(ret)     \
                                                : "c"(n),"0"((uint)(a)) \
                                                : "cc");        \
                                        ret;                    \
                                })
# endif
#endif
#ifndef ROTATE_l32
# define ROTATE_l32(a,n)     (((a)<<(n&0x1f))|(((a)&0xffffffff)>>((32-n)&0x1f)))
#endif
#ifndef ROTATE_r32
# define ROTATE_r32(a,n)     (((a)<<((32-n)&0x1f))|(((a)&0xffffffff)>>(n&0x1f)))
#endif

#define RC5_32_MASK     0xffffffffL

#define RC5_16_P        0xB7E1
#define RC5_16_Q        0x9E37
#define RC5_32_P        0xB7E15163L
#define RC5_32_Q        0x9E3779B9L
#define RC5_64_P        0xB7E151628AED2A6BLL
#define RC5_64_Q        0x9E3779B97F4A7C15LL

#define E_RC5_32(a,b,s,n) \
        a^=b; \
        a=ROTATE_l32(a,b); \
        a+=s[n]; \
        a&=RC5_32_MASK; \
        b^=a; \
        b=ROTATE_l32(b,a); \
        b+=s[n+1]; \
        b&=RC5_32_MASK;

#define D_RC5_32(a,b,s,n) \
        b-=s[n+1]; \
        b&=RC5_32_MASK; \
        b=ROTATE_r32(b,a); \
        b^=a; \
        a-=s[n]; \
        a&=RC5_32_MASK; \
        a=ROTATE_r32(a,b); \
        a^=b;
