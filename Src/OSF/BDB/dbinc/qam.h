/*-
 * See the file LICENSE for redistribution information.
 * Copyright (c) 1999, 2011 Oracle and/or its affiliates.  All rights reserved.
 * $Id$
 */
#ifndef	_DB_QAM_H_
#define	_DB_QAM_H_

#if defined(__cplusplus)
extern "C" {
#endif
/*
 * QAM data elements: a status field and the data.
 */
typedef struct _qamdata {
	uint8  flags;	/* 00: delete bit. */
#define	QAM_VALID	0x01
#define	QAM_SET		0x02
	uint8  data[1];	/* Record. */
} QAMDATA;

struct __queue;		typedef struct __queue QUEUE;
struct __qcursor;	typedef struct __qcursor QUEUE_CURSOR;

struct __qcursor {
	/* struct __dbc_internal */
	__DBC_INTERNAL
	/* Queue private part */
	/* Per-thread information: queue private. */
	db_recno_t	 recno;		/* Current record number. */
	uint32	 flags;
};
/* 
	Array of open extents. 
*/
struct __qmpf {
	int pinref;
	DB_MPOOLFILE *mpf;
};			 

typedef struct __mpfarray {
	uint32 n_extent;         /* Number of extents in table. */
	uint32 low_extent;       /* First extent open. */
	uint32 hi_extent;        /* Last extent open. */
	struct __qmpf * mpfarray;   /* Array of open extents. */
} MPFARRAY;
/*
 * The in-memory, per-tree queue data structure.
 */
struct __queue {
	db_pgno_t q_meta;		/* Database meta-data page. */
	db_pgno_t q_root;		/* Database root page. */
	int	  re_pad;		/* Fixed-length padding byte. */
	uint32 re_len;		/* Length for fixed-length records. */
	uint32 rec_page;		/* records per page */
	uint32 page_ext;		/* Pages per extent */
	MPFARRAY array1, array2;	/* File arrays. */
	//
	// Extent file configuration: 
	//
	DBT pgcookie;			/* Initialized pgcookie. */
	DB_PGINFO pginfo;		/* Initialized pginfo struct. */
	char *path;			/* Space allocated to file pathname. */
	char *name;			/* The name of the file. */
	char *dir;			/* The dir of the file. */
	int mode;			/* Mode to open extents. */
};
//
// Format for queue extent names. 
//
#define	QUEUE_EXTENT		"%s%c__dbq.%s.%d"
#define	QUEUE_EXTENT_HEAD	"__dbq.%s."
#define	QUEUE_EXTENT_PREFIX	"__dbq."

typedef struct __qam_filelist {
	DB_MPOOLFILE *mpf;
	uint32 id;
} QUEUE_FILELIST;
/*
 * Calculate the page number of a recno.
 *
 * Number of records per page =
 *	Divide the available space on the page by the record len + header.
 *
 * Page number for record =
 *	divide the physical record number by the records per page
 *	add the root page number
 *	For now the root page will always be 1, but we might want to change
 *	in the future (e.g. multiple fixed len queues per file).
 *
 * Index of record on page =
 *	physical record number, less the logical pno times records/page
 */
#define	CALC_QAM_RECNO_PER_PAGE(dbp) (((dbp)->pgsize - QPAGE_SZ(dbp)) / (uint32)DB_ALIGN((uintmax_t)SSZA(QAMDATA, data) + ((QUEUE *)(dbp)->q_internal)->re_len, sizeof(uint32)))
#define	QAM_RECNO_PER_PAGE(dbp)	     (((QUEUE*)(dbp)->q_internal)->rec_page)
#define	QAM_RECNO_PAGE(dbp, recno)   (((QUEUE *)(dbp)->q_internal)->q_root + (((recno) - 1) / QAM_RECNO_PER_PAGE(dbp)))
#define	QAM_PAGE_EXTENT(dbp, pgno)   (((pgno) - 1) / ((QUEUE *)(dbp)->q_internal)->page_ext)
#define	QAM_RECNO_EXTENT(dbp, recno) QAM_PAGE_EXTENT(dbp, QAM_RECNO_PAGE(dbp, recno))
#define	QAM_RECNO_INDEX(dbp, pgno, recno) (uint32)(((recno) - 1) - (QAM_RECNO_PER_PAGE(dbp) * (pgno - ((QUEUE *)(dbp)->q_internal)->q_root)))

#define	QAM_GET_RECORD(dbp, page, index)				\
    ((QAMDATA *)((uint8 *)(page) + (QPAGE_SZ(dbp) +			\
    (DB_ALIGN((uintmax_t)SSZA(QAMDATA, data) +				\
    ((QUEUE *)(dbp)->q_internal)->re_len, sizeof(uint32)) * index))))

#define	QAM_AFTER_CURRENT(meta, recno)					\
    ((recno) >= (meta)->cur_recno &&					\
    ((meta)->first_recno <= (meta)->cur_recno ||			\
    ((recno) < (meta)->first_recno &&					\
    (recno) - (meta)->cur_recno < (meta)->first_recno - (recno))))

#define	QAM_BEFORE_FIRST(meta, recno)					\
    ((recno) < (meta)->first_recno &&					\
    ((meta)->first_recno <= (meta)->cur_recno ||			\
    ((recno) > (meta)->cur_recno &&					\
    (recno) - (meta)->cur_recno > (meta)->first_recno - (recno))))

#define	QAM_NOT_VALID(meta, recno) (recno == RECNO_OOB || QAM_BEFORE_FIRST(meta, recno) || QAM_AFTER_CURRENT(meta, recno))

#define QAM_WAKEUP(dbc, ret) do {					\
	if(STD_LOCKING(dbc)) {						\
		dbc->lock.pgno = PGNO_INVALID;				\
		dbc->lock.type = DB_PAGE_LOCK;				\
		ret = __lock_wakeup((dbc)->dbp->env, &(dbc)->lock_dbt);	\
	} else								\
		ret = 0;						\
} while (0)

/* Handle wrap around. */
#define QAM_INC_RECNO(recno) do { recno++; } while (recno == RECNO_OOB)
#define QAM_DEC_RECNO(recno) do { recno--; } while (recno == RECNO_OOB)
/*
 * Log opcodes for the mvptr routine.
 */
#define	QAM_SETFIRST		0x01
#define	QAM_SETCUR		0x02
#define	QAM_TRUNCATE		0x04

typedef enum {
	QAM_PROBE_GET,
	QAM_PROBE_PUT,
	QAM_PROBE_DIRTY,
	QAM_PROBE_MPF
} qam_probe_mode;

/*
 * Ops for __qam_nameop.
 */
typedef enum {
	QAM_NAME_DISCARD,
	QAM_NAME_RENAME,
	QAM_NAME_REMOVE
} qam_name_op;

#define	__qam_fget(dbc, pgnoaddr, flags, addrp) __qam_fprobe(dbc, *pgnoaddr, addrp, QAM_PROBE_GET, DB_PRIORITY_UNCHANGED, flags)
#define	__qam_fput(dbc, pgno, addrp, priority)  __qam_fprobe(dbc, pgno, addrp, QAM_PROBE_PUT, priority, 0)
#define	__qam_dirty(dbc, pgno, pagep, priority) __qam_fprobe(dbc, pgno, pagep, QAM_PROBE_DIRTY, priority, 0)

#if defined(__cplusplus)
}
#endif

#include "dbinc_auto/qam_auto.h"
#include "dbinc_auto/qam_ext.h"
#endif /* !_DB_QAM_H_ */
