/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * The DSS routines are based on patches supplied by
 * Steven Schoch <schoch@sheba.arc.nasa.gov>.
 */

#ifndef HEADER_DSA_H
#define HEADER_DSA_H

#include <openssl/opensslconf.h>

# ifndef OPENSSL_NO_DSA
# ifdef  __cplusplus
extern "C" {
# endif
#include <openssl/e_os2.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/ossl_typ.h>
#include <openssl/opensslconf.h>
#include <openssl/bn.h>
#if OPENSSL_API_COMPAT < 0x10100000L
	#include <openssl/dh.h>
#endif
#ifndef OPENSSL_DSA_MAX_MODULUS_BITS
	#define OPENSSL_DSA_MAX_MODULUS_BITS   10000
#endif
#define OPENSSL_DSA_FIPS_MIN_MODULUS_BITS 1024
#define DSA_FLAG_CACHE_MONT_P   0x01
#if OPENSSL_API_COMPAT < 0x10100000L
	// Does nothing. Previously this switched off constant time behaviour.
	#define DSA_FLAG_NO_EXP_CONSTTIME       0x00
#endif
/*
 * If this flag is set the DSA method is FIPS compliant and can be used in
 * FIPS mode. This is set in the validated module method. If an application
 * sets this flag in its own methods it is its responsibility to ensure the
 * result is compliant.
 */

#define DSA_FLAG_FIPS_METHOD                    0x0400

/*
 * If this flag is set the operations normally disabled in FIPS mode are
 * permitted it is then the applications responsibility to ensure that the
 * usage is compliant.
 */

#define DSA_FLAG_NON_FIPS_ALLOW                 0x0400
#define DSA_FLAG_FIPS_CHECKED                   0x0800

/* Already defined in ossl_typ.h */
/* typedef struct dsa_st DSA; */
/* typedef struct dsa_method DSA_METHOD; */

typedef struct DSA_SIG_st DSA_SIG;

#define d2i_DSAparams_fp(fp,x) (DSA *)ASN1_d2i_fp((char *(*)())DSA_new, \
                (char *(*)())d2i_DSAparams,(fp),(uchar **)(x))
#define i2d_DSAparams_fp(fp,x) ASN1_i2d_fp(i2d_DSAparams,(fp), \
                (uchar *)(x))
#define d2i_DSAparams_bio(bp,x) ASN1_d2i_bio_of(DSA,DSA_new,d2i_DSAparams,bp,x)
#define i2d_DSAparams_bio(bp,x) ASN1_i2d_bio_of_const(DSA,i2d_DSAparams,bp,x)

DSA *DSAparams_dup(DSA *x);
DSA_SIG *DSA_SIG_new(void);
void DSA_SIG_free(DSA_SIG *a);
int i2d_DSA_SIG(const DSA_SIG *a, uchar **pp);
DSA_SIG *d2i_DSA_SIG(DSA_SIG **v, const uchar **pp, long length);
void DSA_SIG_get0(const DSA_SIG *sig, const BIGNUM **pr, const BIGNUM **ps);
int DSA_SIG_set0(DSA_SIG *sig, BIGNUM *r, BIGNUM *s);

DSA_SIG *DSA_do_sign(const uchar *dgst, int dlen, DSA *dsa);
int DSA_do_verify(const uchar *dgst, int dgst_len,
                  DSA_SIG *sig, DSA *dsa);

const DSA_METHOD *DSA_OpenSSL(void);

void DSA_set_default_method(const DSA_METHOD *);
const DSA_METHOD *DSA_get_default_method(void);
int DSA_set_method(DSA *dsa, const DSA_METHOD *);
const DSA_METHOD *DSA_get_method(DSA *d);

DSA *DSA_new(void);
DSA *DSA_new_method(ENGINE *engine);
void DSA_free(DSA *r);
/* "up" the DSA object's reference count */
int DSA_up_ref(DSA *r);
int DSA_size(const DSA *);
int DSA_bits(const DSA *d);
int DSA_security_bits(const DSA *d);
        /* next 4 return -1 on error */
int DSA_sign_setup(DSA *dsa, BN_CTX *ctx_in, BIGNUM **kinvp, BIGNUM **rp);
int DSA_sign(int type, const uchar *dgst, int dlen, uchar *sig, uint *siglen, DSA *dsa);
int DSA_verify(int type, const uchar *dgst, int dgst_len, const uchar *sigbuf, int siglen, DSA *dsa);
#define DSA_get_ex_new_index(l, p, newf, dupf, freef) CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_DSA, l, p, newf, dupf, freef)
int DSA_set_ex_data(DSA *d, int idx, void *arg);
void *DSA_get_ex_data(DSA *d, int idx);

DSA *d2i_DSAPublicKey(DSA **a, const uchar **pp, long length);
DSA *d2i_DSAPrivateKey(DSA **a, const uchar **pp, long length);
DSA *d2i_DSAparams(DSA **a, const uchar **pp, long length);

/* Deprecated version */
DEPRECATEDIN_0_9_8(DSA *DSA_generate_parameters(int bits, uchar *seed,
	int seed_len, int *counter_ret, unsigned long *h_ret, void (*callback) (int, int, void *), void *cb_arg))
/* New version */
int DSA_generate_parameters_ex(DSA *dsa, int bits, const uchar *seed, int seed_len, int *counter_ret, unsigned long *h_ret, BN_GENCB *cb);

int DSA_generate_key(DSA *a);
int i2d_DSAPublicKey(const DSA *a, uchar **pp);
int i2d_DSAPrivateKey(const DSA *a, uchar **pp);
int i2d_DSAparams(const DSA *a, uchar **pp);

int DSAparams_print(BIO *bp, const DSA *x);
int DSA_print(BIO *bp, const DSA *x, int off);
# ifndef OPENSSL_NO_STDIO
int DSAparams_print_fp(FILE *fp, const DSA *x);
int DSA_print_fp(FILE *bp, const DSA *x, int off);
# endif

#define DSS_prime_checks 50
/*
 * Primality test according to FIPS PUB 186[-1], Appendix 2.1: 50 rounds of
 * Rabin-Miller
 */
#define DSA_is_prime(n, callback, cb_arg) BN_is_prime(n, DSS_prime_checks, callback, NULL, cb_arg)

# ifndef OPENSSL_NO_DH
/*
 * Convert DSA structure (key or just parameters) into DH structure (be
 * careful to avoid small subgroup attacks when using this!)
 */
DH *DSA_dup_DH(const DSA *r);
# endif

#define EVP_PKEY_CTX_set_dsa_paramgen_bits(ctx, nbits) \
        EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA, EVP_PKEY_OP_PARAMGEN, EVP_PKEY_CTRL_DSA_PARAMGEN_BITS, nbits, NULL)

#define EVP_PKEY_CTRL_DSA_PARAMGEN_BITS         (EVP_PKEY_ALG_CTRL + 1)
#define EVP_PKEY_CTRL_DSA_PARAMGEN_Q_BITS       (EVP_PKEY_ALG_CTRL + 2)
#define EVP_PKEY_CTRL_DSA_PARAMGEN_MD           (EVP_PKEY_ALG_CTRL + 3)

void DSA_get0_pqg(const DSA *d, const BIGNUM **p, const BIGNUM **q, const BIGNUM **g);
int DSA_set0_pqg(DSA *d, BIGNUM *p, BIGNUM *q, BIGNUM *g);
void DSA_get0_key(const DSA *d, const BIGNUM **pub_key, const BIGNUM **priv_key);
int DSA_set0_key(DSA *d, BIGNUM *pub_key, BIGNUM *priv_key);
void DSA_clear_flags(DSA *d, int flags);
int DSA_test_flags(const DSA *d, int flags);
void DSA_set_flags(DSA *d, int flags);
ENGINE *DSA_get0_engine(DSA *d);

DSA_METHOD *DSA_meth_new(const char *name, int flags);
void DSA_meth_free(DSA_METHOD *dsam);
DSA_METHOD *DSA_meth_dup(const DSA_METHOD *dsam);
const char *DSA_meth_get0_name(const DSA_METHOD *dsam);
int DSA_meth_set1_name(DSA_METHOD *dsam, const char *name);
int DSA_meth_get_flags(DSA_METHOD *dsam);
int DSA_meth_set_flags(DSA_METHOD *dsam, int flags);
void *DSA_meth_get0_app_data(const DSA_METHOD *dsam);
int DSA_meth_set0_app_data(DSA_METHOD *dsam, void *app_data);
DSA_SIG *(*DSA_meth_get_sign(const DSA_METHOD *dsam))(const uchar *, int, DSA *);
int DSA_meth_set_sign(DSA_METHOD *dsam, DSA_SIG *(*sign) (const uchar *, int, DSA *));
int (*DSA_meth_get_sign_setup(const DSA_METHOD *dsam))(DSA *, BN_CTX *, BIGNUM **, BIGNUM **);
int DSA_meth_set_sign_setup(DSA_METHOD *dsam, int (*sign_setup) (DSA *, BN_CTX *, BIGNUM **, BIGNUM **));
int (*DSA_meth_get_verify(const DSA_METHOD *dsam)) (const uchar *, int , DSA_SIG *, DSA *);
int DSA_meth_set_verify(DSA_METHOD *dsam, int (*verify) (const uchar *, int, DSA_SIG *, DSA *));
int (*DSA_meth_get_mod_exp(const DSA_METHOD *dsam))(DSA *, BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, BN_CTX *, BN_MONT_CTX *);
int DSA_meth_set_mod_exp(DSA_METHOD *dsam, int (*mod_exp) (DSA *, BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, BN_CTX *, BN_MONT_CTX *));
int (*DSA_meth_get_bn_mod_exp(const DSA_METHOD *dsam))(DSA *, BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, BN_CTX *, BN_MONT_CTX *);
int DSA_meth_set_bn_mod_exp(DSA_METHOD *dsam, int (*bn_mod_exp) (DSA *, BIGNUM *, const BIGNUM *, const BIGNUM *, const BIGNUM *, BN_CTX *, BN_MONT_CTX *));
int (*DSA_meth_get_init(const DSA_METHOD *dsam))(DSA *);
int DSA_meth_set_init(DSA_METHOD *dsam, int (*init)(DSA *));
int (*DSA_meth_get_finish(const DSA_METHOD *dsam)) (DSA *);
int DSA_meth_set_finish(DSA_METHOD *dsam, int (*finish) (DSA *));
int (*DSA_meth_get_paramgen(const DSA_METHOD *dsam))(DSA *, int, const uchar *, int, int *, unsigned long *, BN_GENCB *);
int DSA_meth_set_paramgen(DSA_METHOD *dsam, int (*paramgen) (DSA *, int, const uchar *, int, int *, unsigned long *, BN_GENCB *));
int (*DSA_meth_get_keygen(const DSA_METHOD *dsam)) (DSA *);
int DSA_meth_set_keygen(DSA_METHOD *dsam, int (*keygen) (DSA *));

/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_DSA_strings(void);

/* Error codes for the DSA functions. */

/* Function codes. */
#define DSA_F_DSAPARAMS_PRINT                            100
#define DSA_F_DSAPARAMS_PRINT_FP                         101
#define DSA_F_DSA_BUILTIN_PARAMGEN                       125
#define DSA_F_DSA_BUILTIN_PARAMGEN2                      126
#define DSA_F_DSA_DO_SIGN                                112
#define DSA_F_DSA_DO_VERIFY                              113
#define DSA_F_DSA_METH_DUP                               127
#define DSA_F_DSA_METH_NEW                               128
#define DSA_F_DSA_METH_SET1_NAME                         129
#define DSA_F_DSA_NEW_METHOD                             103
#define DSA_F_DSA_PARAM_DECODE                           119
#define DSA_F_DSA_PRINT_FP                               105
#define DSA_F_DSA_PRIV_DECODE                            115
#define DSA_F_DSA_PRIV_ENCODE                            116
#define DSA_F_DSA_PUB_DECODE                             117
#define DSA_F_DSA_PUB_ENCODE                             118
#define DSA_F_DSA_SIGN                                   106
#define DSA_F_DSA_SIGN_SETUP                             107
#define DSA_F_DSA_SIG_NEW                                102
#define DSA_F_OLD_DSA_PRIV_DECODE                        122
#define DSA_F_PKEY_DSA_CTRL                              120
#define DSA_F_PKEY_DSA_KEYGEN                            121

/* Reason codes. */
#define DSA_R_BAD_Q_VALUE                                102
#define DSA_R_BN_DECODE_ERROR                            108
#define DSA_R_BN_ERROR                                   109
#define DSA_R_DECODE_ERROR                               104
#define DSA_R_INVALID_DIGEST_TYPE                        106
#define DSA_R_INVALID_PARAMETERS                         112
#define DSA_R_MISSING_PARAMETERS                         101
#define DSA_R_MODULUS_TOO_LARGE                          103
#define DSA_R_NO_PARAMETERS_SET                          107
#define DSA_R_PARAMETER_ENCODING_ERROR                   105
#define DSA_R_Q_NOT_PRIME                                113
#define DSA_R_SEED_LEN_SMALL                             110

#  ifdef  __cplusplus
}
#  endif
# endif
#endif
