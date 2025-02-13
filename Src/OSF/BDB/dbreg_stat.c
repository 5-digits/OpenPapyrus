/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop

#ifdef HAVE_STATISTICS
static int __dbreg_print_all __P((ENV*, uint32));
/*
 * __dbreg_stat_print --
 *	Print the dbreg statistics.
 */
int __dbreg_stat_print(ENV * env, uint32 flags)
{
	int ret;
	if(LF_ISSET(DB_STAT_ALL) && (ret = __dbreg_print_all(env, flags)) != 0)
		return ret;
	return 0;
}
/*
 * __dbreg_print_fname --
 *	Display the contents of an FNAME structure.
 */
void __dbreg_print_fname(ENV * env, FNAME * fnp)
{
	static const FN fn[] = {
		{ DB_FNAME_DURABLE,     "DB_FNAME_DURABLE" },
		{ DB_FNAME_NOTLOGGED,   "DB_FNAME_NOTLOGGED" },
		{ DB_FNAME_CLOSED,      "DB_FNAME_CLOSED" },
		{ DB_FNAME_RECOVER,     "DB_FNAME_RECOVER" },
		{ 0,                    NULL }
	};
	__db_msg(env, "%s", DB_GLOBAL(db_line));
	__db_msg(env, "DB handle FNAME contents:");
	STAT_LONG("log ID", fnp->id);
	STAT_ULONG("Meta pgno", fnp->meta_pgno);
	__db_print_fileid(env, fnp->ufid, "\tFile ID");
	STAT_ULONG("create txn", fnp->create_txnid);
	STAT_ULONG("refcount", fnp->txn_ref);
	__db_prflags(env, NULL, fnp->flags, fn, NULL, "\tFlags");
}
/*
 * __dbreg_print_all --
 *	Display the ENV's list of files.
 */
static int __dbreg_print_all(ENV * env, uint32 flags)
{
	DB * dbp;
	FNAME * fnp;
	int32 * stack;
	int del, first;
	uint32 i;
	DB_LOG * dblp = env->lg_handle;
	LOG * lp = (LOG *)dblp->reginfo.primary;
	__db_msg(env, "LOG FNAME list:");
	__mutex_print_debug_single(env, "File name mutex", lp->mtx_filelist, flags);
	STAT_LONG("Fid max", lp->fid_max);
	STAT_LONG("Log buffer size", lp->buffer_size);
	MUTEX_LOCK(env, lp->mtx_filelist);
	first = 1;
	SH_TAILQ_FOREACH(fnp, &lp->fq, q, __fname) {
		if(first) {
			first = 0;
			__db_msg(env, "ID\tName\t\tType\tPgno\tPid\tTxnid\tFlags\tRef\tDBP-info");
		}
		dbp = fnp->id >= dblp->dbentry_cnt ? NULL : dblp->dbentry[fnp->id].dbp;
		del = fnp->id >= dblp->dbentry_cnt ? 0 : dblp->dbentry[fnp->id].deleted;
		__db_msg(env, "%ld\t%-8s%s%-8s%s\t%lu\t%lu\t%lx\t%lx\t%lx\t%s",
			(long)fnp->id,
			fnp->fname_off == INVALID_ROFF ? "" : (char *)R_ADDR(&dblp->reginfo, fnp->fname_off),
			fnp->dname_off == INVALID_ROFF ? "" : ":",
			fnp->dname_off == INVALID_ROFF ? "" : (char *)R_ADDR(&dblp->reginfo, fnp->dname_off),
			__db_dbtype_to_string(fnp->s_type),
			(ulong)fnp->meta_pgno, (ulong)fnp->pid,
			(ulong)fnp->create_txnid, (ulong)fnp->flags,
			(ulong)fnp->txn_ref,
			dbp == NULL ? "No DBP" : "DBP");
		if(dbp)
			__db_msg(env, " (%d %lx %lx)", del, P_TO_ULONG(dbp), (ulong)(dbp == NULL ? 0 : dbp->flags));
	}
	MUTEX_UNLOCK(env, lp->mtx_filelist);
	__db_msg(env, "%s", DB_GLOBAL(db_line));
	__db_msg(env, "LOG region list of free IDs.");
	if(lp->free_fid_stack == INVALID_ROFF)
		__db_msg(env, "Free id stack is empty.");
	else {
		STAT_ULONG("Free id array size", lp->free_fids_alloced);
		STAT_ULONG("Number of ids on the free stack", lp->free_fids);
		stack = (int32 *)R_ADDR(&dblp->reginfo, lp->free_fid_stack);
		for(i = 0; i < lp->free_fids; i++)
			STAT_LONG("fid", stack[i]);
	}
	return 0;
}
#endif
