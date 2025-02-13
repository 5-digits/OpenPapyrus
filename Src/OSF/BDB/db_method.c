/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop

static int __db_get_byteswapped(DB*, int *);
static int __db_get_dbname(DB*, const char **, const char **);
static DB_ENV * __db_get_env(DB *);
static void __db_get_msgcall(DB*, void (**)(const DB_ENV*, const char *));
static DB_MPOOLFILE * __db_get_mpf(DB *);
static int __db_get_multiple(DB *);
static int __db_get_transactional(DB *);
static int __db_get_type(DB*, DBTYPE*dbtype);
static int __db_init(DB*, uint32);
static int __db_get_alloc(DB*, void *(**)(size_t), void *(**)(void *, size_t), void (**)(void *));
static int __db_set_alloc(DB*, void *(*)(size_t), void *(*)(void *, size_t), void (*)(void *));
static int __db_get_append_recno(DB*, int (**)(DB*, DBT*, db_recno_t));
static int __db_set_append_recno(DB*, int (*)(DB *, DBT *, db_recno_t));
static int __db_get_cachesize(DB*, uint32*, uint32*, int *);
static int __db_set_cachesize(DB*, uint32, uint32, int);
static int __db_get_create_dir(DB*, const char **);
static int __db_set_create_dir(DB*, const char *);
static int __db_get_dup_compare(DB*, int (**)(DB*, const DBT*, const DBT *));
static int __db_set_dup_compare(DB*, int (*)(DB *, const DBT *, const DBT *));
static int __db_get_encrypt_flags(DB*, uint32 *);
static int __db_set_encrypt(DB*, const char *, uint32);
static int __db_get_feedback(DB*, void (**)(DB*, int, int));
static int __db_set_feedback(DB*, void (*)(DB *, int, int));
static void __db_map_flags(DB*, uint32*, uint32 *);
static int __db_get_pagesize(DB*, uint32 *);
static int __db_set_paniccall(DB*, void (*)(DB_ENV *, int));
static int __db_set_priority(DB*, DB_CACHE_PRIORITY);
static int __db_get_priority(DB*, DB_CACHE_PRIORITY *);
static void __db_get_errcall(DB*, void (**)(const DB_ENV*, const char *, const char *));
static void __db_set_errcall(DB*, void (*)(const DB_ENV *, const char *, const char *));
static void __db_get_errfile(DB*, FILE**);
static void __db_set_errfile(DB*, FILE *);
static void __db_get_errpfx(DB*, const char **);
static void __db_set_errpfx(DB*, const char *);
static void __db_set_msgcall(DB*, void (*)(const DB_ENV *, const char *));
static void __db_get_msgfile(DB*, FILE**);
static void __db_set_msgfile(DB*, FILE *);
static int __db_get_assoc_flags(DB*, uint32 *);
static void __dbh_err(DB*, int, const char *, ...);
static void __dbh_errx(DB*, const char *, ...);
//
// DB constructor.
//
int db_create(DB ** dbpp, DB_ENV * dbenv, uint32 flags)
{
	int ret;
	DB_THREAD_INFO * ip = NULL;
	ENV * env = (dbenv == NULL) ? NULL : dbenv->env;
	// Check for invalid function flags
	switch(flags) {
	    case 0:
			break;
	    case DB_XA_CREATE:
			if(dbenv) {
				__db_errx(env, DB_STR("0504", "XA applications may not specify an environment to db_create"));
				return EINVAL;
			}
			else {
				//
				// If it's an XA database, open it within the XA environment,
				// taken from the global list of environments.  (When the XA
				// transaction manager called our xa_start() routine the
				// "current" environment was moved to the start of the list.
				// 
				env = TAILQ_FIRST(&DB_GLOBAL(envq));
				if(env == NULL) {
					__db_errx(env, DB_STR("0505", "Cannot open XA database before XA is enabled"));
					return EINVAL;
				}
			}
			break;
	    default:
			return __db_ferr(env, "db_create", 0);
	}
	if(env)
		ENV_ENTER(env, ip);
	// 
	// If we are opening an XA database, make sure we don't have a global XA transaction running.
	// 
	if(LF_ISSET(DB_XA_CREATE)) {
		XA_NO_TXN(ip, ret);
		if(ret != 0)
			goto err;
	}
	ret = __db_create_internal(dbpp, env, flags);
err:
	if(env)
		ENV_LEAVE(env, ip);
	return ret;
}
//
// DB constructor internal routine.
//
int __db_create_internal(DB ** dbpp, ENV * env, uint32 flags)
{
	DB * dbp;
	DB_ENV * dbenv;
	DB_REP * db_rep;
	int ret;
	*dbpp = NULL;
	// If we don't have an environment yet, allocate a local one
	if(env == NULL) {
		if((ret = db_env_create(&dbenv, 0)) != 0)
			return ret;
		env = dbenv->env;
		F_SET(env, ENV_DBLOCAL);
	}
	else
		dbenv = env->dbenv;
	// Allocate and initialize the DB handle
	if((ret = __os_calloc(env, 1, sizeof(*dbp), &dbp)) != 0)
		goto err;
	dbp->dbenv = env->dbenv;
	dbp->env = env;
	if((ret = __db_init(dbp, flags)) != 0)
		goto err;
	MUTEX_LOCK(env, env->mtx_dblist);
	++env->db_ref;
	MUTEX_UNLOCK(env, env->mtx_dblist);
	// 
	// Set the replication timestamp; it's 0 if we're not in a replicated
	// environment.  Don't acquire a lock to read the value, even though
	// it's opaque: all we check later is value equality, nothing else.
	// 
	dbp->timestamp = REP_ON(env) ? ((REGENV *)env->reginfo->primary)->rep_timestamp : 0;
	// 
	// Set the replication generation number for fid management; valid
	// replication generations start at 1.  Don't acquire a lock to
	// read the value.  All we check later is value equality.
	//
	db_rep = env->rep_handle;
	dbp->fid_gen = REP_ON(env) ? ((REP *)db_rep->region)->gen : 0;
	// Open a backing DB_MPOOLFILE handle in the memory pool. 
	if((ret = __memp_fcreate(env, &dbp->mpf)) != 0)
		goto err;
	dbp->type = DB_UNKNOWN;
	*dbpp = dbp;
	return 0;
err:
	if(dbp) {
		if(dbp->mpf)
			__memp_fclose(dbp->mpf, 0);
		__os_free(env, dbp);
	}
	if(F_ISSET(env, ENV_DBLOCAL))
		__env_close(dbp->dbenv, 0);
	return ret;
}
//
// Initialize a DB structure.
//
static int __db_init(DB * dbp, uint32 flags)
{
	int ret;
	dbp->locker = NULL;
	dbp->alt_close = NULL;
	LOCK_INIT(dbp->handle_lock);
	TAILQ_INIT(&dbp->free_queue);
	TAILQ_INIT(&dbp->active_queue);
	TAILQ_INIT(&dbp->join_queue);
	LIST_INIT(&dbp->s_secondaries);
	FLD_SET(dbp->am_ok, DB_OK_BTREE|DB_OK_HASH|DB_OK_HEAP|DB_OK_QUEUE|DB_OK_RECNO);
	// DB PUBLIC HANDLE LIST BEGIN 
	dbp->associate = __db_associate_pp;
	dbp->associate_foreign = __db_associate_foreign_pp;
	dbp->close = __db_close_pp;
	dbp->compact = __db_compact_pp;
	dbp->cursor = __db_cursor_pp;
	dbp->del = __db_del_pp;
	dbp->dump = __db_dump_pp;
	dbp->err = __dbh_err;
	dbp->errx = __dbh_errx;
	dbp->exists = __db_exists;
	dbp->fd = __db_fd_pp;
	dbp->get = __db_get_pp;
	dbp->get_alloc = __db_get_alloc;
	dbp->get_append_recno = __db_get_append_recno;
	dbp->get_assoc_flags = __db_get_assoc_flags;
	dbp->get_byteswapped = __db_get_byteswapped;
	dbp->get_cachesize = __db_get_cachesize;
	dbp->get_create_dir = __db_get_create_dir;
	dbp->get_dbname = __db_get_dbname;
	dbp->get_dup_compare = __db_get_dup_compare;
	dbp->get_encrypt_flags = __db_get_encrypt_flags;
	dbp->get_env = __db_get_env;
	dbp->get_errcall = __db_get_errcall;
	dbp->get_errfile = __db_get_errfile;
	dbp->get_errpfx = __db_get_errpfx;
	dbp->get_feedback = __db_get_feedback;
	dbp->get_flags = __db_get_flags;
	dbp->get_lorder = __db_get_lorder;
	dbp->get_mpf = __db_get_mpf;
	dbp->get_msgcall = __db_get_msgcall;
	dbp->get_msgfile = __db_get_msgfile;
	dbp->get_multiple = __db_get_multiple;
	dbp->get_open_flags = __db_get_open_flags;
	dbp->get_partition_dirs = __partition_get_dirs;
	dbp->get_partition_callback = __partition_get_callback;
	dbp->get_partition_keys = __partition_get_keys;
	dbp->get_pagesize = __db_get_pagesize;
	dbp->get_priority = __db_get_priority;
	dbp->get_transactional = __db_get_transactional;
	dbp->get_type = __db_get_type;
	dbp->join = __db_join_pp;
	dbp->key_range = __db_key_range_pp;
	dbp->open = __db_open_pp;
	dbp->pget = __db_pget_pp;
	dbp->put = __db_put_pp;
	dbp->remove = __db_remove_pp;
	dbp->rename = __db_rename_pp;
	dbp->set_alloc = __db_set_alloc;
	dbp->set_append_recno = __db_set_append_recno;
	dbp->set_cachesize = __db_set_cachesize;
	dbp->set_create_dir = __db_set_create_dir;
	dbp->set_dup_compare = __db_set_dup_compare;
	dbp->set_encrypt = __db_set_encrypt;
	dbp->set_errcall = __db_set_errcall;
	dbp->set_errfile = __db_set_errfile;
	dbp->set_errpfx = __db_set_errpfx;
	dbp->set_feedback = __db_set_feedback;
	dbp->set_flags = __db_set_flags;
	dbp->set_lorder = __db_set_lorder;
	dbp->set_msgcall = __db_set_msgcall;
	dbp->set_msgfile = __db_set_msgfile;
	dbp->set_pagesize = __db_set_pagesize;
	dbp->set_paniccall = __db_set_paniccall;
	dbp->set_partition = __partition_set;
	dbp->set_partition_dirs = __partition_set_dirs;
	dbp->set_priority = __db_set_priority;
	dbp->sort_multiple = __db_sort_multiple;
	dbp->stat = __db_stat_pp;
	dbp->stat_print = __db_stat_print_pp;
	dbp->sync = __db_sync_pp;
	dbp->truncate = __db_truncate_pp;
	dbp->upgrade = __db_upgrade_pp;
	dbp->verify = __db_verify_pp;
	// DB PUBLIC HANDLE LIST END 
	// Access method specific.
	if((ret = __bam_db_create(dbp)) != 0)
		return ret;
	if((ret = __ham_db_create(dbp)) != 0)
		return ret;
	if((ret = __heap_db_create(dbp)) != 0)
		return ret;
	if((ret = __qam_db_create(dbp)) != 0)
		return ret;
	COMPQUIET(flags, 0);
	return 0;
}
//
// Error if an unreasonable method is called.
//
int FASTCALL __dbh_am_chk(DB * dbp, uint32 flags)
{
	// 
	// We start out allowing any access methods to be called, and as the
	// application calls the methods the options become restricted.  The
	// idea is to quit as soon as an illegal method combination is called.
	//
	if((LF_ISSET(DB_OK_BTREE) && FLD_ISSET(dbp->am_ok, DB_OK_BTREE)) || (LF_ISSET(DB_OK_HASH) && FLD_ISSET(dbp->am_ok, DB_OK_HASH)) ||
	   (LF_ISSET(DB_OK_HEAP) && FLD_ISSET(dbp->am_ok, DB_OK_HEAP)) || (LF_ISSET(DB_OK_QUEUE) && FLD_ISSET(dbp->am_ok, DB_OK_QUEUE)) ||
	   (LF_ISSET(DB_OK_RECNO) && FLD_ISSET(dbp->am_ok, DB_OK_RECNO))) {
		FLD_CLR(dbp->am_ok, ~flags);
		return 0;
	}
	else {
		__db_errx(dbp->env, DB_STR("0506", "call implies an access method which is inconsistent with previous calls"));
		return EINVAL;
	}
}
/*
 * __dbh_err --
 *	Db.err method.
 */
static void __dbh_err(DB * dbp, int error, const char * fmt, ...)
{
	/* Message with error string, to stderr by default. */
	DB_REAL_ERR(dbp->dbenv, error, DB_ERROR_SET, 1, fmt);
}
/*
 * __dbh_errx --
 *	Db.errx method.
 */
static void __dbh_errx(DB * dbp, const char * fmt, ...)
{
	/* Message without error string, to stderr by default. */
	DB_REAL_ERR(dbp->dbenv, 0, DB_ERROR_NOT_SET, 1, fmt);
}
/*
 * __db_get_byteswapped --
 *	Return if database requires byte swapping.
 */
static int __db_get_byteswapped(DB * dbp, int * isswapped)
{
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->get_byteswapped");
	*isswapped = F_ISSET(dbp, DB_AM_SWAP) ? 1 : 0;
	return 0;
}
/*
 * __db_get_dbname --
 *	Get the name of the database as passed to DB->open.
 */
static int __db_get_dbname(DB * dbp, const char ** fnamep, const char ** dnamep)
{
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->get_dbname");
	ASSIGN_PTR(fnamep, dbp->fname);
	ASSIGN_PTR(dnamep, dbp->dname);
	return 0;
}
/*
 * __db_get_env --
 *	Get the DB_ENV handle that was passed to db_create.
 */
static DB_ENV * __db_get_env(DB * dbp)
{
	return dbp->dbenv;
}
/*
 * __db_get_mpf --
 *	Get the underlying DB_MPOOLFILE handle.
 */
static DB_MPOOLFILE * __db_get_mpf(DB * dbp)
{
	return dbp->mpf;
}
/*
 * get_multiple --
 *	Return whether this DB handle references a physical file with multiple
 *	databases.
 */
static int __db_get_multiple(DB * dbp)
{
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->get_multiple");
	/*
	 * Only return TRUE if the handle is for the master database, not for
	 * any subdatabase in the physical file.  If it's a Btree, with the
	 * subdatabases flag set, and the meta-data page has the right value,
	 * return TRUE.  (We don't need to check it's a Btree, I suppose, but
	 * it doesn't hurt.)
	 */
	return dbp->type == DB_BTREE && F_ISSET(dbp, DB_AM_SUBDB) && dbp->meta_pgno == PGNO_BASE_MD ? 1 : 0;
}
/*
 * get_transactional --
 *	Return whether this database was created in a transaction.
 */
static int __db_get_transactional(DB * dbp)
{
	return F_ISSET(dbp, DB_AM_TXN) ? 1 : 0;
}
/*
 * __db_get_type --
 *	Return type of underlying database.
 */
static int __db_get_type(DB * dbp, DBTYPE * dbtype)
{
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->get_type");
	*dbtype = dbp->type;
	return 0;
}
/*
 * __db_get_append_recno --
 *	Get record number append routine.
 */
static int __db_get_append_recno(DB * dbp, int (**funcp)__P((DB*, DBT*, db_recno_t)))
{
	DB_ILLEGAL_METHOD(dbp, DB_OK_QUEUE|DB_OK_RECNO);
	ASSIGN_PTR(funcp, dbp->db_append_recno);
	return 0;
}
/*
 * __db_set_append_recno --
 *	Set record number append routine.
 */
static int __db_set_append_recno(DB * dbp, int (*func)__P((DB*, DBT*, db_recno_t)))
{
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_append_recno");
	DB_ILLEGAL_METHOD(dbp, DB_OK_QUEUE|DB_OK_RECNO);
	dbp->db_append_recno = func;
	return 0;
}
/*
 * __db_get_cachesize --
 *	Get underlying cache size.
 */
static int __db_get_cachesize(DB * dbp, uint32 * cache_gbytesp, uint32 * cache_bytesp, int * ncachep)
{
	DB_ILLEGAL_IN_ENV(dbp, "DB->get_cachesize");
	return __memp_get_cachesize(dbp->dbenv, cache_gbytesp, cache_bytesp, ncachep);
}
/*
 * __db_set_cachesize --
 *	Set underlying cache size.
 */
static int __db_set_cachesize(DB * dbp, uint32 cache_gbytes, uint32 cache_bytes, int ncache)
{
	DB_ILLEGAL_IN_ENV(dbp, "DB->set_cachesize");
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_cachesize");
	return __memp_set_cachesize(dbp->dbenv, cache_gbytes, cache_bytes, ncache);
}

static int __db_set_create_dir(DB * dbp, const char * dir)
{
	int i;
	DB_ENV * dbenv = dbp->dbenv;
	for(i = 0; i < dbenv->data_next; i++)
		if(strcmp(dir, dbenv->db_data_dir[i]) == 0)
			break;
	if(i == dbenv->data_next) {
		__db_errx(dbp->env, DB_STR_A("0507", "Directory %s not in environment list.", "%s"), dir);
		return EINVAL;
	}
	dbp->dirname = dbenv->db_data_dir[i];
	return 0;
}

static int __db_get_create_dir(DB * dbp, const char ** dirp)
{
	*dirp = dbp->dirname;
	return 0;
}
/*
 * __db_get_dup_compare --
 *	Get duplicate comparison routine.
 */
static int __db_get_dup_compare(DB * dbp, int (**funcp)(DB*, const DBT*, const DBT *))
{
	DB_ILLEGAL_METHOD(dbp, DB_OK_BTREE|DB_OK_HASH);
	if(funcp) {
#ifdef HAVE_COMPRESSION
		if(DB_IS_COMPRESSED(dbp)) {
			*funcp = static_cast<BTREE *>(dbp->bt_internal)->compress_dup_compare;
		}
		else
#endif
		*funcp = dbp->dup_compare;
	}
	return 0;
}
/*
 * __db_set_dup_compare --
 *	Set duplicate comparison routine.
 */
static int __db_set_dup_compare(DB * dbp, int (*func)(DB*, const DBT*, const DBT *))
{
	int ret;
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_dup_compare");
	DB_ILLEGAL_METHOD(dbp, DB_OK_BTREE|DB_OK_HASH);
	if((ret = __db_set_flags(dbp, DB_DUPSORT)) != 0)
		return ret;
#ifdef HAVE_COMPRESSION
	if(DB_IS_COMPRESSED(dbp)) {
		dbp->dup_compare = __bam_compress_dupcmp;
		static_cast<BTREE *>(dbp->bt_internal)->compress_dup_compare = func;
	}
	else
#endif
	dbp->dup_compare = func;
	return 0;
}
/*
 * __db_get_encrypt_flags --
 */
static int __db_get_encrypt_flags(DB * dbp, uint32 * flagsp)
{
	DB_ILLEGAL_IN_ENV(dbp, "DB->get_encrypt_flags");
	return __env_get_encrypt_flags(dbp->dbenv, flagsp);
}
/*
 * __db_set_encrypt --
 *	Set database passwd.
 */
static int __db_set_encrypt(DB * dbp, const char * passwd, uint32 flags)
{
	DB_CIPHER * db_cipher;
	int ret;
	DB_ILLEGAL_IN_ENV(dbp, "DB->set_encrypt");
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_encrypt");
	if((ret = __env_set_encrypt(dbp->dbenv, passwd, flags)) != 0)
		return ret;
	/*
	 * In a real env, this gets initialized with the region.  In a local
	 * env, we must do it here.
	 */
	db_cipher = dbp->env->crypto_handle;
	if(!F_ISSET(db_cipher, CIPHER_ANY) &&
	   (ret = db_cipher->init(dbp->env, db_cipher)) != 0)
		return ret;
	return __db_set_flags(dbp, DB_ENCRYPT);
}

static void __db_get_errcall(DB * dbp, void (**errcallp)__P((const DB_ENV*, const char *, const char *)))
{
	__env_get_errcall(dbp->dbenv, errcallp);
}

static void __db_set_errcall(DB * dbp, void (*errcall)__P((const DB_ENV*, const char *, const char *)))
{
	__env_set_errcall(dbp->dbenv, errcall);
}

static void __db_get_errfile(DB * dbp, FILE ** errfilep)
{
	__env_get_errfile(dbp->dbenv, errfilep);
}

static void __db_set_errfile(DB * dbp, FILE * errfile)
{
	__env_set_errfile(dbp->dbenv, errfile);
}

static void __db_get_errpfx(DB * dbp, const char ** errpfxp)
{
	__env_get_errpfx(dbp->dbenv, errpfxp);
}

static void __db_set_errpfx(DB * dbp, const char * errpfx)
{
	__env_set_errpfx(dbp->dbenv, errpfx);
}

static int __db_get_feedback(DB * dbp, void (**feedbackp)__P((DB*, int, int)))
{
	ASSIGN_PTR(feedbackp, dbp->db_feedback);
	return 0;
}

static int __db_set_feedback(DB * dbp, void (*feedback)__P((DB*, int, int)))
{
	dbp->db_feedback = feedback;
	return 0;
}
/*
 * __db_map_flags --
 *	Maps between public and internal flag values.
 *    This function doesn't check for validity, so it can't fail.
 */
static void __db_map_flags(DB * dbp, uint32 * inflagsp, uint32 * outflagsp)
{
	COMPQUIET(dbp, 0);
	if(FLD_ISSET(*inflagsp, DB_CHKSUM)) {
		FLD_SET(*outflagsp, DB_AM_CHKSUM);
		FLD_CLR(*inflagsp, DB_CHKSUM);
	}
	if(FLD_ISSET(*inflagsp, DB_ENCRYPT)) {
		FLD_SET(*outflagsp, DB_AM_ENCRYPT|DB_AM_CHKSUM);
		FLD_CLR(*inflagsp, DB_ENCRYPT);
	}
	if(FLD_ISSET(*inflagsp, DB_TXN_NOT_DURABLE)) {
		FLD_SET(*outflagsp, DB_AM_NOT_DURABLE);
		FLD_CLR(*inflagsp, DB_TXN_NOT_DURABLE);
	}
}
/*
 * __db_get_assoc_flags --
 */
static int __db_get_assoc_flags(DB * dbp, uint32 * flagsp)
{
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->get_assoc_flags");
	*flagsp = dbp->s_assoc_flags;
	return 0;
}
/*
 * __db_get_flags --
 *	The DB->get_flags method.
 *
 * PUBLIC: int __db_get_flags __P((DB *, uint32 *));
 */
int __db_get_flags(DB * dbp, uint32 * flagsp)
{
	static const uint32 db_flags[] = {
		DB_CHKSUM,
		DB_DUP,
		DB_DUPSORT,
		DB_ENCRYPT,
#ifdef HAVE_QUEUE
		DB_INORDER,
#endif
		DB_RECNUM,
		DB_RENUMBER,
		DB_REVSPLITOFF,
		DB_SNAPSHOT,
		DB_TXN_NOT_DURABLE,
		0
	};
	uint32 f, mapped_flag;
	int i;
	uint32 flags = 0;
	for(i = 0; (f = db_flags[i]) != 0; i++) {
		mapped_flag = 0;
		__db_map_flags(dbp, &f, &mapped_flag);
		__bam_map_flags(dbp, &f, &mapped_flag);
		__ram_map_flags(dbp, &f, &mapped_flag);
#ifdef HAVE_QUEUE
		__qam_map_flags(dbp, &f, &mapped_flag);
#endif
		DB_ASSERT(dbp->env, f == 0);
		if(F_ISSET(dbp, mapped_flag) == mapped_flag)
			LF_SET(db_flags[i]);
	}
	*flagsp = flags;
	return 0;
}
/*
 * __db_set_flags --
 *	DB->set_flags.
 *
 * PUBLIC: int  __db_set_flags __P((DB *, uint32));
 */
int __db_set_flags(DB * dbp, uint32 flags)
{
	int ret;
	ENV * env = dbp->env;
	if(LF_ISSET(DB_ENCRYPT) && !CRYPTO_ON(env)) {
		__db_errx(env, DB_STR("0508", "Database environment not configured for encryption"));
		return EINVAL;
	}
	if(LF_ISSET(DB_TXN_NOT_DURABLE))
		ENV_REQUIRES_CONFIG(env, env->tx_handle, "DB_NOT_DURABLE", DB_INIT_TXN);
	__db_map_flags(dbp, &flags, &dbp->flags);
	if((ret = __bam_set_flags(dbp, &flags)) != 0)
		return ret;
	if((ret = __ram_set_flags(dbp, &flags)) != 0)
		return ret;
#ifdef HAVE_QUEUE
	if((ret = __qam_set_flags(dbp, &flags)) != 0)
		return ret;
#endif
	return flags == 0 ? 0 : __db_ferr(env, "DB->set_flags", 0);
}

/*
 * __db_get_lorder --
 *	Get whether lorder is swapped or not.
 *
 * PUBLIC: int  __db_get_lorder __P((DB *, int *));
 */
int __db_get_lorder(DB * dbp, int * db_lorderp)
{
	int ret;
	/* Flag if the specified byte order requires swapping. */
	switch(ret = __db_byteorder(dbp->env, 1234)) {
	    case 0:
		*db_lorderp = F_ISSET(dbp, DB_AM_SWAP) ? 4321 : 1234;
		break;
	    case DB_SWAPBYTES:
		*db_lorderp = F_ISSET(dbp, DB_AM_SWAP) ? 1234 : 4321;
		break;
	    default:
		return ret;
		/* NOTREACHED */
	}
	return 0;
}
/*
 * __db_set_lorder --
 *	Set whether lorder is swapped or not.
 *
 * PUBLIC: int  __db_set_lorder __P((DB *, int));
 */
int __db_set_lorder(DB * dbp, int db_lorder)
{
	int ret;
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_lorder");
	/* Flag if the specified byte order requires swapping. */
	switch(ret = __db_byteorder(dbp->env, db_lorder)) {
	    case 0:
		F_CLR(dbp, DB_AM_SWAP);
		break;
	    case DB_SWAPBYTES:
		F_SET(dbp, DB_AM_SWAP);
		break;
	    default:
		return ret;
		/* NOTREACHED */
	}
	return 0;
}

static int __db_get_alloc(DB * dbp, void *(**mal_funcp)__P((size_t)), void *(**real_funcp)__P((void *, size_t)), void (**free_funcp)__P((void *)))
{
	DB_ILLEGAL_IN_ENV(dbp, "DB->get_alloc");
	return __env_get_alloc(dbp->dbenv, mal_funcp, real_funcp, free_funcp);
}

static int __db_set_alloc(DB * dbp, void *(*mal_func)__P((size_t)), void *(*real_func)__P((void *, size_t)), void (*free_func)__P((void *)))
{
	DB_ILLEGAL_IN_ENV(dbp, "DB->set_alloc");
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_alloc");
	return __env_set_alloc(dbp->dbenv, mal_func, real_func, free_func);
}

static void __db_get_msgcall(DB * dbp, void (**msgcallp)__P((const DB_ENV*, const char *)))
{
	__env_get_msgcall(dbp->dbenv, msgcallp);
}

static void __db_set_msgcall(DB * dbp, void (*msgcall)__P((const DB_ENV*, const char *)))
{
	__env_set_msgcall(dbp->dbenv, msgcall);
}

static void __db_get_msgfile(DB * dbp, FILE ** msgfilep)
{
	__env_get_msgfile(dbp->dbenv, msgfilep);
}

static void __db_set_msgfile(DB * dbp, FILE * msgfile)
{
	__env_set_msgfile(dbp->dbenv, msgfile);
}

static int __db_get_pagesize(DB * dbp, uint32 * db_pagesizep)
{
	*db_pagesizep = dbp->pgsize;
	return 0;
}
/*
 * __db_set_pagesize --
 *	DB->set_pagesize
 *
 * PUBLIC: int  __db_set_pagesize __P((DB *, uint32));
 */
int __db_set_pagesize(DB * dbp, uint32 db_pagesize)
{
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_pagesize");
	if(db_pagesize < DB_MIN_PGSIZE) {
		__db_errx(dbp->env, DB_STR_A("0509", "page sizes may not be smaller than %lu", "%lu"), (ulong)DB_MIN_PGSIZE);
		return EINVAL;
	}
	if(db_pagesize > DB_MAX_PGSIZE) {
		__db_errx(dbp->env, DB_STR_A("0510", "page sizes may not be larger than %lu", "%lu"), (ulong)DB_MAX_PGSIZE);
		return EINVAL;
	}
	/*
	 * We don't want anything that's not a power-of-2, as we rely on that
	 * for alignment of various types on the pages.
	 */
	if(!POWER_OF_TWO(db_pagesize)) {
		__db_errx(dbp->env, DB_STR("0511", "page sizes must be a power-of-2"));
		return EINVAL;
	}
	/*
	 * XXX
	 * Should we be checking for a page size that's not a multiple of 512,
	 * so that we never try and write less than a disk sector?
	 */
	dbp->pgsize = db_pagesize;
	return 0;
}

static int __db_set_paniccall(DB * dbp, void (*paniccall)__P((DB_ENV*, int)))
{
	return __env_set_paniccall(dbp->dbenv, paniccall);
}

static int __db_set_priority(DB * dbp, DB_CACHE_PRIORITY priority)
{
	dbp->priority = priority;
	return 0;
}

static int __db_get_priority(DB * dbp, DB_CACHE_PRIORITY * priority)
{
	if(dbp->priority == DB_PRIORITY_UNCHANGED)
		return __memp_get_priority(dbp->mpf, priority);
	else
		*priority = dbp->priority;
	return 0;
}
