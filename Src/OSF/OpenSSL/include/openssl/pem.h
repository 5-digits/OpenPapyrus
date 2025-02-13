/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_PEM_H
#define HEADER_PEM_H

#include <openssl/pem2.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define PEM_BUFSIZE             1024

#define PEM_STRING_X509_OLD     "X509 CERTIFICATE"
#define PEM_STRING_X509         "CERTIFICATE"
#define PEM_STRING_X509_TRUSTED "TRUSTED CERTIFICATE"
#define PEM_STRING_X509_REQ_OLD "NEW CERTIFICATE REQUEST"
#define PEM_STRING_X509_REQ     "CERTIFICATE REQUEST"
#define PEM_STRING_X509_CRL     "X509 CRL"
#define PEM_STRING_EVP_PKEY     "ANY PRIVATE KEY"
#define PEM_STRING_PUBLIC       "PUBLIC KEY"
#define PEM_STRING_RSA          "RSA PRIVATE KEY"
#define PEM_STRING_RSA_PUBLIC   "RSA PUBLIC KEY"
#define PEM_STRING_DSA          "DSA PRIVATE KEY"
#define PEM_STRING_DSA_PUBLIC   "DSA PUBLIC KEY"
#define PEM_STRING_PKCS7        "PKCS7"
#define PEM_STRING_PKCS7_SIGNED "PKCS #7 SIGNED DATA"
#define PEM_STRING_PKCS8        "ENCRYPTED PRIVATE KEY"
#define PEM_STRING_PKCS8INF     "PRIVATE KEY"
#define PEM_STRING_DHPARAMS     "DH PARAMETERS"
#define PEM_STRING_DHXPARAMS    "X9.42 DH PARAMETERS"
#define PEM_STRING_SSL_SESSION  "SSL SESSION PARAMETERS"
#define PEM_STRING_DSAPARAMS    "DSA PARAMETERS"
#define PEM_STRING_ECDSA_PUBLIC "ECDSA PUBLIC KEY"
#define PEM_STRING_ECPARAMETERS "EC PARAMETERS"
#define PEM_STRING_ECPRIVATEKEY "EC PRIVATE KEY"
#define PEM_STRING_PARAMETERS   "PARAMETERS"
#define PEM_STRING_CMS          "CMS"

#define PEM_TYPE_ENCRYPTED      10
#define PEM_TYPE_MIC_ONLY       20
#define PEM_TYPE_MIC_CLEAR      30
#define PEM_TYPE_CLEAR          40

typedef struct pem_recip_st {
	char * name;
	X509_NAME * dn;
	int cipher;
	int key_enc;
	/*      char iv[8]; unused and wrong size */
} PEM_USER;

typedef struct pem_ctx_st {
	int type;               /* what type of object */
	struct {
		int version;
		int mode;
	} proc_type;
	char * domain;
	struct {
		int cipher;
		/*-
		   unused, and wrong size
		   uchar iv[8]; */
	} DEK_info;

	PEM_USER * originator;

	int num_recipient;
	PEM_USER ** recipient;

/*-
    XXX(ben): don#t think this is used!
        STACK *x509_chain;      / * certificate chain */
	EVP_MD * md;            /* signature type */

	int md_enc;             /* is the md encrypted or not? */
	int md_len;             /* length of md_data */
	char * md_data;         /* message digest, could be pkey encrypted */

	EVP_CIPHER * dec;       /* date encryption cipher */
	int key_len;            /* key length */
	uchar * key;    /* key */
	/*-
	   unused, and wrong size
	   uchar iv[8]; */

	int data_enc;           /* is the data encrypted */
	int data_len;
	uchar * data;
} PEM_CTX;

/*
 * These macros make the PEM_read/PEM_write functions easier to maintain and
 * write. Now they are all implemented with either: IMPLEMENT_PEM_rw(...) or
 * IMPLEMENT_PEM_rw_cb(...)
 */

#ifdef OPENSSL_NO_STDIO
	#define IMPLEMENT_PEM_read_fp(name, type, str, asn1) /**/
	#define IMPLEMENT_PEM_write_fp(name, type, str, asn1) /**/
	#define IMPLEMENT_PEM_write_fp_const(name, type, str, asn1) /**/
	#define IMPLEMENT_PEM_write_cb_fp(name, type, str, asn1) /**/
	#define IMPLEMENT_PEM_write_cb_fp_const(name, type, str, asn1) /**/
#else

	#define IMPLEMENT_PEM_read_fp(name, type, str, asn1) \
	type *PEM_read_ ## name(FILE *fp, type **x, pem_password_cb *cb, void * u) \
	{ return (type*)PEM_ASN1_read((d2i_of_void*)d2i_ ## asn1, str, fp, (void **)x, cb, u); }
	#define IMPLEMENT_PEM_write_fp(name, type, str, asn1) \
	int PEM_write_ ## name(FILE *fp, type *x) { return PEM_ASN1_write((i2d_of_void*)i2d_ ## asn1, str, fp, x, NULL, NULL, 0, NULL, NULL); }

#define IMPLEMENT_PEM_write_fp_const(name, type, str, asn1) \
	int PEM_write_ ## name(FILE *fp, const type *x)	\
		{ return PEM_ASN1_write((i2d_of_void*)i2d_ ## asn1, str, fp, (void *)x, NULL, NULL, 0, NULL, NULL); }
#define IMPLEMENT_PEM_write_cb_fp(name, type, str, asn1) \
	int PEM_write_ ## name(FILE *fp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb,	void * u) \
		{ return PEM_ASN1_write((i2d_of_void*)i2d_ ## asn1, str, fp, x, enc, kstr, klen, cb, u); }
#define IMPLEMENT_PEM_write_cb_fp_const(name, type, str, asn1) \
	int PEM_write_ ## name(FILE *fp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb, void * u) \
		{ return PEM_ASN1_write((i2d_of_void*)i2d_ ## asn1, str, fp, x, enc, kstr, klen, cb, u); }
# endif

#define IMPLEMENT_PEM_read_bio(name, type, str, asn1) \
	type *PEM_read_bio_ ## name(BIO *bp, type **x, pem_password_cb *cb, void * u) \
		{ return (type*)PEM_ASN1_read_bio((d2i_of_void*)d2i_ ## asn1, str, bp, (void **)x, cb, u); } // @sobolev (type *)
#define IMPLEMENT_PEM_write_bio(name, type, str, asn1) \
	int PEM_write_bio_ ## name(BIO *bp, type *x) \
		{ return PEM_ASN1_write_bio((i2d_of_void*)i2d_ ## asn1, str, bp, x, NULL, NULL, 0, NULL, NULL); }
#define IMPLEMENT_PEM_write_bio_const(name, type, str, asn1) \
	int PEM_write_bio_ ## name(BIO *bp, const type *x) \
		{ return PEM_ASN1_write_bio((i2d_of_void*)i2d_ ## asn1, str, bp, (void *)x, NULL, NULL, 0, NULL, NULL); }
#define IMPLEMENT_PEM_write_cb_bio(name, type, str, asn1) \
	int PEM_write_bio_ ## name(BIO *bp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb, void * u) \
		{ return PEM_ASN1_write_bio((i2d_of_void*)i2d_ ## asn1, str, bp, x, enc, kstr, klen, cb, u); }
#define IMPLEMENT_PEM_write_cb_bio_const(name, type, str, asn1)	\
	int PEM_write_bio_ ## name(BIO *bp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb, void * u) \
		{ return PEM_ASN1_write_bio((i2d_of_void*)i2d_ ## asn1, str, bp, (void *)x, enc, kstr, klen, cb, u); }
#define IMPLEMENT_PEM_write(name, type, str, asn1) IMPLEMENT_PEM_write_bio(name, type, str, asn1) IMPLEMENT_PEM_write_fp(name, type, str, asn1)
#define IMPLEMENT_PEM_write_const(name, type, str, asn1) IMPLEMENT_PEM_write_bio_const(name, type, str, asn1) IMPLEMENT_PEM_write_fp_const(name, type, str, asn1)
#define IMPLEMENT_PEM_write_cb(name, type, str, asn1) IMPLEMENT_PEM_write_cb_bio(name, type, str, asn1) IMPLEMENT_PEM_write_cb_fp(name, type, str, asn1)

#define IMPLEMENT_PEM_write_cb_const(name, type, str, asn1) IMPLEMENT_PEM_write_cb_bio_const(name, type, str, asn1)	IMPLEMENT_PEM_write_cb_fp_const(name, type, str, asn1)
#define IMPLEMENT_PEM_read(name, type, str, asn1) IMPLEMENT_PEM_read_bio(name, type, str, asn1) IMPLEMENT_PEM_read_fp(name, type, str, asn1)
#define IMPLEMENT_PEM_rw(name, type, str, asn1)	IMPLEMENT_PEM_read(name, type, str, asn1) IMPLEMENT_PEM_write(name, type, str, asn1)

#define IMPLEMENT_PEM_rw_const(name, type, str, asn1) IMPLEMENT_PEM_read(name, type, str, asn1) IMPLEMENT_PEM_write_const(name, type, str, asn1)
#define IMPLEMENT_PEM_rw_cb(name, type, str, asn1) IMPLEMENT_PEM_read(name, type, str, asn1) IMPLEMENT_PEM_write_cb(name, type, str, asn1)

/* These are the same except they are for the declarations */

#if defined(OPENSSL_NO_STDIO)
	#define DECLARE_PEM_read_fp(name, type) /**/
	#define DECLARE_PEM_write_fp(name, type) /**/
	#define DECLARE_PEM_write_fp_const(name, type) /**/
	#define DECLARE_PEM_write_cb_fp(name, type) /**/
#else
	#define DECLARE_PEM_read_fp(name, type)	type *PEM_read_ ## name(FILE *fp, type **x, pem_password_cb *cb, void * u);
	#define DECLARE_PEM_write_fp(name, type) int PEM_write_ ## name(FILE *fp, type *x);
	#define DECLARE_PEM_write_fp_const(name, type) int PEM_write_ ## name(FILE *fp, const type *x);
	#define DECLARE_PEM_write_cb_fp(name, type) int PEM_write_ ## name(FILE *fp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb, void * u);
#endif

#define DECLARE_PEM_read_bio(name, type) type *PEM_read_bio_ ## name(BIO *bp, type **x, pem_password_cb *cb, void * u);
#define DECLARE_PEM_write_bio(name, type) int PEM_write_bio_ ## name(BIO *bp, type *x);
#define DECLARE_PEM_write_bio_const(name, type)	int PEM_write_bio_ ## name(BIO *bp, const type *x);
#define DECLARE_PEM_write_cb_bio(name, type) int PEM_write_bio_ ## name(BIO *bp, type *x, const EVP_CIPHER *enc, uchar *kstr, int klen, pem_password_cb *cb, void * u);

#define DECLARE_PEM_write(name, type) DECLARE_PEM_write_bio(name, type) DECLARE_PEM_write_fp(name, type)
#define DECLARE_PEM_write_const(name, type) DECLARE_PEM_write_bio_const(name, type)	DECLARE_PEM_write_fp_const(name, type)
#define DECLARE_PEM_write_cb(name, type) DECLARE_PEM_write_cb_bio(name, type) DECLARE_PEM_write_cb_fp(name, type)
#define DECLARE_PEM_read(name, type) DECLARE_PEM_read_bio(name, type) DECLARE_PEM_read_fp(name, type)
#define DECLARE_PEM_rw(name, type) DECLARE_PEM_read(name, type) DECLARE_PEM_write(name, type)
#define DECLARE_PEM_rw_const(name, type) DECLARE_PEM_read(name, type) DECLARE_PEM_write_const(name, type)
#define DECLARE_PEM_rw_cb(name, type) DECLARE_PEM_read(name, type) DECLARE_PEM_write_cb(name, type)
/*@funcdef*/typedef int pem_password_cb(char * buf, int size, int rwflag, void * userdata);

int PEM_get_EVP_CIPHER_INFO(char * header, EVP_CIPHER_INFO * cipher);
int PEM_do_header(EVP_CIPHER_INFO * cipher, uchar * data, long * len, pem_password_cb * callback, void * u);
int PEM_read_bio(BIO * bp, char ** name, char ** header, uchar ** data, long * len);
int PEM_write_bio(BIO * bp, const char * name, const char * hdr, const uchar * data, long len);
int PEM_bytes_read_bio(uchar ** pdata, long * plen, char ** pnm, const char * name, BIO * bp, pem_password_cb * cb, void * u);
void * PEM_ASN1_read_bio(d2i_of_void * d2i, const char * name, BIO * bp, void ** x, pem_password_cb * cb, void * u);
int PEM_ASN1_write_bio(i2d_of_void * i2d, const char * name, BIO * bp, void * x, const EVP_CIPHER * enc, uchar * kstr, int klen, pem_password_cb * cb, void * u);

STACK_OF(X509_INFO) *PEM_X509_INFO_read_bio(BIO *bp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void * u);
int PEM_X509_INFO_write_bio(BIO * bp, X509_INFO * xi, EVP_CIPHER * enc, uchar * kstr, int klen, pem_password_cb * cd, void * u);

#ifndef OPENSSL_NO_STDIO
	int PEM_read(FILE * fp, char ** name, char ** header, uchar ** data, long * len);
	int PEM_write(FILE * fp, const char * name, const char * hdr, const uchar * data, long len);
	void * PEM_ASN1_read(d2i_of_void * d2i, const char * name, FILE * fp, void ** x, pem_password_cb * cb, void * u);
	int PEM_ASN1_write(i2d_of_void * i2d, const char * name, FILE * fp, void * x, const EVP_CIPHER * enc, uchar * kstr, int klen, pem_password_cb * callback, void * u);
	STACK_OF(X509_INFO) *PEM_X509_INFO_read(FILE *fp, STACK_OF(X509_INFO) *sk, pem_password_cb *cb, void * u);
#endif

int PEM_SignInit(EVP_MD_CTX * ctx, EVP_MD * type);
int PEM_SignUpdate(EVP_MD_CTX * ctx, uchar * d, uint cnt);
int PEM_SignFinal(EVP_MD_CTX * ctx, uchar * sigret, uint * siglen, EVP_PKEY * pkey);

int PEM_def_callback(char * buf, int num, int w, void * key);
void PEM_proc_type(char * buf, int type);
void PEM_dek_info(char * buf, const char * type, int len, char * str);

#include <openssl/symhacks.h>

DECLARE_PEM_rw(X509, X509)
DECLARE_PEM_rw(X509_AUX, X509)
DECLARE_PEM_rw(X509_REQ, X509_REQ)
DECLARE_PEM_write(X509_REQ_NEW, X509_REQ)
DECLARE_PEM_rw(X509_CRL, X509_CRL)
DECLARE_PEM_rw(PKCS7, PKCS7)
DECLARE_PEM_rw(NETSCAPE_CERT_SEQUENCE, NETSCAPE_CERT_SEQUENCE)
DECLARE_PEM_rw(PKCS8, X509_SIG)
DECLARE_PEM_rw(PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO)
#ifndef OPENSSL_NO_RSA
	DECLARE_PEM_rw_cb(RSAPrivateKey, RSA)
	DECLARE_PEM_rw_const(RSAPublicKey, RSA)
	DECLARE_PEM_rw(RSA_PUBKEY, RSA)
#endif
#ifndef OPENSSL_NO_DSA
	DECLARE_PEM_rw_cb(DSAPrivateKey, DSA)
	DECLARE_PEM_rw(DSA_PUBKEY, DSA)
	DECLARE_PEM_rw_const(DSAparams, DSA)
#endif
#ifndef OPENSSL_NO_EC
	DECLARE_PEM_rw_const(ECPKParameters, EC_GROUP)
	DECLARE_PEM_rw_cb(ECPrivateKey, EC_KEY)
	DECLARE_PEM_rw(EC_PUBKEY, EC_KEY)
#endif
#ifndef OPENSSL_NO_DH
	DECLARE_PEM_rw_const(DHparams, DH)
	DECLARE_PEM_write_const(DHxparams, DH)
#endif
DECLARE_PEM_rw_cb(PrivateKey, EVP_PKEY)
DECLARE_PEM_rw(PUBKEY, EVP_PKEY)

int PEM_write_bio_PrivateKey_traditional(BIO * bp, EVP_PKEY * x, const EVP_CIPHER * enc, uchar * kstr, int klen, pem_password_cb * cb, void * u);
int PEM_write_bio_PKCS8PrivateKey_nid(BIO * bp, EVP_PKEY * x, int nid, char * kstr, int klen, pem_password_cb * cb, void * u);
int PEM_write_bio_PKCS8PrivateKey(BIO *, EVP_PKEY *, const EVP_CIPHER *, char *, int, pem_password_cb *, void *);
int i2d_PKCS8PrivateKey_bio(BIO * bp, EVP_PKEY * x, const EVP_CIPHER * enc, char * kstr, int klen, pem_password_cb * cb, void * u);
int i2d_PKCS8PrivateKey_nid_bio(BIO * bp, EVP_PKEY * x, int nid, char * kstr, int klen, pem_password_cb * cb, void * u);
EVP_PKEY * d2i_PKCS8PrivateKey_bio(BIO * bp, EVP_PKEY ** x, pem_password_cb * cb, void * u);

#ifndef OPENSSL_NO_STDIO
	int i2d_PKCS8PrivateKey_fp(FILE * fp, EVP_PKEY * x, const EVP_CIPHER * enc, char * kstr, int klen, pem_password_cb * cb, void * u);
	int i2d_PKCS8PrivateKey_nid_fp(FILE * fp, EVP_PKEY * x, int nid, char * kstr, int klen, pem_password_cb * cb, void * u);
	int PEM_write_PKCS8PrivateKey_nid(FILE * fp, EVP_PKEY * x, int nid, char * kstr, int klen, pem_password_cb * cb, void * u);
	EVP_PKEY * d2i_PKCS8PrivateKey_fp(FILE * fp, EVP_PKEY ** x, pem_password_cb * cb, void * u);
	int PEM_write_PKCS8PrivateKey(FILE * fp, EVP_PKEY * x, const EVP_CIPHER * enc, char * kstr, int klen, pem_password_cb * cd, void * u);
#endif
EVP_PKEY * PEM_read_bio_Parameters(BIO * bp, EVP_PKEY ** x);
int PEM_write_bio_Parameters(BIO * bp, EVP_PKEY * x);

#ifndef OPENSSL_NO_DSA
	EVP_PKEY * b2i_PrivateKey(const uchar ** in, long length);
	EVP_PKEY * b2i_PublicKey(const uchar ** in, long length);
	EVP_PKEY * b2i_PrivateKey_bio(BIO * in);
	EVP_PKEY * b2i_PublicKey_bio(BIO * in);
	int i2b_PrivateKey_bio(BIO * out, EVP_PKEY * pk);
	int i2b_PublicKey_bio(BIO * out, EVP_PKEY * pk);
	#ifndef OPENSSL_NO_RC4
		EVP_PKEY * b2i_PVK_bio(BIO * in, pem_password_cb * cb, void * u);
		int i2b_PVK_bio(BIO * out, EVP_PKEY * pk, int enclevel, pem_password_cb * cb, void * u);
	#endif
#endif

/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_PEM_strings(void);

/* Error codes for the PEM functions. */

/* Function codes. */
#define PEM_F_B2I_DSS                                    127
#define PEM_F_B2I_PVK_BIO                                128
#define PEM_F_B2I_RSA                                    129
#define PEM_F_CHECK_BITLEN_DSA                           130
#define PEM_F_CHECK_BITLEN_RSA                           131
#define PEM_F_D2I_PKCS8PRIVATEKEY_BIO                    120
#define PEM_F_D2I_PKCS8PRIVATEKEY_FP                     121
#define PEM_F_DO_B2I                                     132
#define PEM_F_DO_B2I_BIO                                 133
#define PEM_F_DO_BLOB_HEADER                             134
#define PEM_F_DO_PK8PKEY                                 126
#define PEM_F_DO_PK8PKEY_FP                              125
#define PEM_F_DO_PVK_BODY                                135
#define PEM_F_DO_PVK_HEADER                              136
#define PEM_F_I2B_PVK                                    137
#define PEM_F_I2B_PVK_BIO                                138
#define PEM_F_LOAD_IV                                    101
#define PEM_F_PEM_ASN1_READ                              102
#define PEM_F_PEM_ASN1_READ_BIO                          103
#define PEM_F_PEM_ASN1_WRITE                             104
#define PEM_F_PEM_ASN1_WRITE_BIO                         105
#define PEM_F_PEM_DEF_CALLBACK                           100
#define PEM_F_PEM_DO_HEADER                              106
#define PEM_F_PEM_GET_EVP_CIPHER_INFO                    107
#define PEM_F_PEM_READ                                   108
#define PEM_F_PEM_READ_BIO                               109
#define PEM_F_PEM_READ_BIO_DHPARAMS                      141
#define PEM_F_PEM_READ_BIO_PARAMETERS                    140
#define PEM_F_PEM_READ_BIO_PRIVATEKEY                    123
#define PEM_F_PEM_READ_DHPARAMS                          142
#define PEM_F_PEM_READ_PRIVATEKEY                        124
#define PEM_F_PEM_SIGNFINAL                              112
#define PEM_F_PEM_WRITE                                  113
#define PEM_F_PEM_WRITE_BIO                              114
#define PEM_F_PEM_WRITE_PRIVATEKEY                       139
#define PEM_F_PEM_X509_INFO_READ                         115
#define PEM_F_PEM_X509_INFO_READ_BIO                     116
#define PEM_F_PEM_X509_INFO_WRITE_BIO                    117

/* Reason codes. */
#define PEM_R_BAD_BASE64_DECODE                          100
#define PEM_R_BAD_DECRYPT                                101
#define PEM_R_BAD_END_LINE                               102
#define PEM_R_BAD_IV_CHARS                               103
#define PEM_R_BAD_MAGIC_NUMBER                           116
#define PEM_R_BAD_PASSWORD_READ                          104
#define PEM_R_BAD_VERSION_NUMBER                         117
#define PEM_R_BIO_WRITE_FAILURE                          118
#define PEM_R_CIPHER_IS_NULL                             127
#define PEM_R_ERROR_CONVERTING_PRIVATE_KEY               115
#define PEM_R_EXPECTING_PRIVATE_KEY_BLOB                 119
#define PEM_R_EXPECTING_PUBLIC_KEY_BLOB                  120
#define PEM_R_HEADER_TOO_LONG                            128
#define PEM_R_INCONSISTENT_HEADER                        121
#define PEM_R_KEYBLOB_HEADER_PARSE_ERROR                 122
#define PEM_R_KEYBLOB_TOO_SHORT                          123
#define PEM_R_MISSING_DEK_IV                             129
#define PEM_R_NOT_DEK_INFO                               105
#define PEM_R_NOT_ENCRYPTED                              106
#define PEM_R_NOT_PROC_TYPE                              107
#define PEM_R_NO_START_LINE                              108
#define PEM_R_PROBLEMS_GETTING_PASSWORD                  109
#define PEM_R_PVK_DATA_TOO_SHORT                         124
#define PEM_R_PVK_TOO_SHORT                              125
#define PEM_R_READ_KEY                                   111
#define PEM_R_SHORT_HEADER                               112
#define PEM_R_UNEXPECTED_DEK_IV                          130
#define PEM_R_UNSUPPORTED_CIPHER                         113
#define PEM_R_UNSUPPORTED_ENCRYPTION                     114
#define PEM_R_UNSUPPORTED_KEY_COMPONENTS                 126

# ifdef  __cplusplus
}
# endif
#endif
