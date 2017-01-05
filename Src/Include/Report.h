// REPORT.H
// Copyright (c) A.Sobolev 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2015, 2016
// @codepage windows-1251
//
#ifndef __REPORT_H
#define __REPORT_H

#include <slib.h>

class TVRez;
//
// ���������� ����������
//
#define BIVAR_CURDATE             101
#define BIVAR_CURTIME             102
#define BIVAR_PAGE                103
//
// ���� ��� ������
//
#define FIRST_TEXT_ID            1001
#define RPT_HEAD                   -1
#define RPT_FOOT                   -2
#define PAGE_HEAD                  -3
#define PAGE_FOOT                  -4
#define GROUP_HEAD                 -5
#define GROUP_FOOT                 -6
#define DETAIL_BODY                -7
//
// ������������� ����� �����
//
#define FLDFMT_BOLD            0x0001
#define FLDFMT_ITALIC          0x0002
#define FLDFMT_UNDERLINE       0x0004
#define FLDFMT_SUPERSCRIPT     0x0008
#define FLDFMT_SUBSCRIPT       0x0010
#define FLDFMT_ESC             0x0020
#define FLDFMT_NOREPEATONPG    0x0040
#define FLDFMT_NOREPEATONRPT   0x0080
#define FLDFMT_STRETCHVERT     0x0100
#define FLDFMT_AGGR            0x0800 // ���� �������� ���������� �������
#define FLDFMT_SKIP            0x1000 // ���������� ������ ���
//
// ������������� ����� �����
//
#define GRPFMT_NEWPAGE         0x0001
#define GRPFMT_SWAPHEAD        0x0002
#define GRPFMT_SWAPFOOT        0x0004
#define GRPFMT_REPRINTHEAD     0x0008
#define GRPFMT_RESETPGNMB      0x0010
#define GRPFMT_SUMMARYONBOTTOM 0x0020 // ���� �� ��� ��������
//
// ���������� ������� ��� �����������
//
#define AGGR_NONE                   0
#define AGGR_COUNT                  1
#define AGGR_SUM                    2
#define AGGR_AVG                    3
#define AGGR_MIN                    4
#define AGGR_MAX                    5
//
// ����� ������
//
#define SPRN_EJECTBEFORE       0x00000001 // ������ �������� ����� �������
#define SPRN_EJECTAFTER        0x00000002 // ������ �������� ����� ������
#define SPRN_NLQ               0x00000004
#define SPRN_CPI10             0x00000008
#define SPRN_CPI12             0x00000010
#define SPRN_CONDS             0x00000020
#define SPRN_LANDSCAPE         0x00000040 // ��������� ���������� ����� (����� �������)
#define SPRN_SKIPGRPS          0x00004000 // ���������� ����������� ��� ������
#define SPRN_TRUEPRINTER       0x00008000 // ��������� ���� (�������� �������)
#define SPRN_PREVIEW           0x00010000 // @v6.2.6 ��������������� ��������
#define SPRN_DONTRENAMEFILES   0x00020000 // @v6.3.5 ������, ��� ������ ����� ������ ����������������� ����
	// �� ������ ������ ����������� (������������) ������ ������. ���� ��� ����� ���������� �
	// ������� CrystalReportPrint, �� ����� ����������������� �� ����� (������ ��� �����).
#define SPRN_USEDUPLEXPRINTING 0x00040000 // ������������ ���������� ������
//
// ����� ������� SPrinter::setEffect
//
#define FONT_BOLD              0x0001
#define FONT_ITALIC            0x0002
#define FONT_UNDERLINE         0x0004
#define FONT_SUPERSCRIPT       0x0008
#define FONT_SUBSCRIPT         0x0010

#define SPCMDSET_DEFAULT            0
#define SPCMDSET_EPSON              1
#define SPCMDSET_PCL                2

#define SPMRG_LEFT                  1
#define SPMRG_RIGHT                 2
#define SPMRG_TOP                   3
#define SPMRG_BOTTOM                4

#define SPQLTY_DRAFT                1
#define SPQLTY_NLQ                  2

#define SPCPI_10                    1
#define SPCPI_12                    2
#define SPCPI_COND                  3

#define SPFS_BOLD                   1
#define SPFS_ITALIC                 2
#define SPFS_UNDERLINE              3

class SPrnCmdSet {
public:
	static SPrnCmdSet * SLAPI CreateInstance(long, long = 0);

	SLAPI SPrnCmdSet();
	virtual SLAPI ~SPrnCmdSet();
	//
	// ������ �� ����������� ������� ����� (� �����������) �������
	// ������ ������� � �����, �������� ��������� ����������, �����������
	// ������������������ � ������� ���������� �������� � ����
	// ������������������. �� ������� ��������� �����������
	// ������������������ �����, ��� ��� ����� ��� ����� �����������
	// ������������ ���������.
	//
	virtual int SLAPI InitPrinter(char *)
	{
		return 0;
	}
	virtual int SLAPI ResetPrinter(char *)
	{
		return 0;
	}
	//
	// SetPageLength ������ ����� ����� (����� - �� ������ !)
	//
	virtual int SLAPI SetPageLength(int, char *)
	{
		return 0;
	}
	virtual int SLAPI SetOrientation(int /* 0 - portrait, !0 - landscapce */, char *)
	{
		return 0;
	}
	virtual int SLAPI SetMargin(int what, int, char *) { return 0; }
	virtual int SLAPI SetQuality(int, char *) { return 0; }
	virtual int SLAPI SetCPI(int, char *) { return 0; }
	virtual int SLAPI SetFontStyle(int, int on_off, char *) { return 0; }
	virtual int SLAPI SetLinesPerInch(int, char *) { return 0; }
private:
	int    dummy;
};

struct ReportDescrEntry {
	//
	// Descr: ���� ���������� �������� ������� � report.ini � stdrpt.ini
	//
	enum {
		tUnkn = 0,
		tComment,
		tData,
		tDescr,
		tDiffIdByScope,
		tModifDate,
		tStd,
		tFormat,
		tDestination,
		tSilent,
		tExistFile
	};
	static int FASTCALL GetIniToken(const char * pBuf, SString * pFileName);

	SLAPI  ReportDescrEntry();
	int    SLAPI ParseIniString(const char * pLine, const ReportDescrEntry * pBaseEntry);
	int    SLAPI SetReportFileName(const char * pFileName);

	enum {
		fInheritedTblNames = 0x0001,
		fDiff_ID_ByScope   = 0x0002, // ���� ���� ���� ����������, �� ������������ ����� ���������������
			// ������� � ��������, ��������������� ������ �������� ����� ����������.
			// ��� ����� ���������� ��-�� ����, ��� ������������� ��������� ��� ��������� ��
			// ������������� ������������ ����� ����� ���������� �� ������� ������������� �����������
			// ��������������� �������
		fTddoResource      = 0x0004
	};
	long   Flags;
	SString ReportPath_;
	SString Description_;
	SString DataName_;
	SString OutputFormat;
};

struct PrnDlgAns {
	SLAPI  PrnDlgAns(const char * pReportName);
	SLAPI ~PrnDlgAns();
	int    SLAPI SetupReportEntries(const char * pContextSymb);

	enum {
		aUndef = 0,
		aPrint = 1,
		aExport,
		aPreview,
		aExportXML,
		aPrepareData,
		aPrepareDataAndExecCR,
		aExportTDDO // @v8.7.8
	};
	enum {
		fForceDDF          = 0x0001,
		fEMail             = 0x0002, // ������������ ��� Dest == aExport
		fUseDuplexPrinting = 0x0004  // ���������� ������
	};
	long   Dest;
	int    Selection;
	uint   NumCopies;
	long   Flags;
	const  char * P_ReportName;
	const  char * P_DefPrnForm;
	SString PrepareDataPath;
	SString Printer;
	SString EmailAddr;
	SString ContextSymb; // @v8.8.3
	TSCollection <ReportDescrEntry> Entries;
private:
	int    SLAPI PreprocessReportFileName(const char * pFileName, ReportDescrEntry * pEntry);
};

class SPrinter {
public:
	SLAPI  SPrinter();
	SLAPI ~SPrinter();
	int    SLAPI setupCmdSet(long cmdsetID, long extra = 0);
	int    SLAPI setPort(char *);
	int    SLAPI startDoc();
	int    SLAPI endDoc();
	int    SLAPI abortDoc();
	int    SLAPI startPage();
	int    SLAPI endPage();
	int    SLAPI escape(int, char *);
	int    SLAPI printLine(const char * buf, size_t maxLen);
	int    SLAPI printChar(int c);
	int    SLAPI checkPort();
	uint   SLAPI getStyle();
	int    SLAPI setEffect(int);
	int    SLAPI initDevice();
	int    SLAPI resetDevice();
	int    SLAPI checkNWCapture();
	//
	// Descr: ������� HandlePrintError ����� ���� ������������ ���������� ���������� ��� ������������� ���������
	//   ������ ������. ���� ��� ������� ���������� 0, �� ������ ����� �������� �� ������, � ��������� ������
	//   ��������� ��������� �������. �� ��������� ��� ������� ����� NULL, ��� �������������� ��� ���������� �� ������.
	//
	static int (*HandlePrintError)(int errCode);
	SPrnCmdSet * cmdSet;
	char * device;
	char   port[64];
	int    prnport;
	int    captured; // ������� ������� ����� ������� �������� ������
	int    fd;       // file handler
	int    pgl;
	int    pgw;
	int    leftMarg;
	int    rightMarg;
	int    topMarg;
	int    bottomMarg;
	uint   options;
};

SPrinter * SLAPI getDefaultPrinter();
//
// ������� ReportIterator ������ ���������� ��������� ��������:
// -1 - ����� ��������
//  0 - ������
//  1 - �������� �������� //
//  2 - �������� ��������, ���������� ������� �� �������������
//
typedef int (*ReportIterator)(int first);

class SReport {
public:
	struct Field {
		int16  id;
		int    name; // @ Index in SReport::text buffer or -1
		TYPEID type;
		union {
			long   format;
			int16  len;  // text
		};
		uint   fldfmt;
		union {
			void * data;
			long   offs; // text
		};
		char * lastval;
	};
	struct Aggr {
		int16  fld;
		int16  aggr;  // ���������� ������
		int16  dpnd;  // ����, ������� ��������� ������������
		int16  scope; // -1 ���� �����, 0.. ����� ������
		union {
			double   rtemp;
			double * ptemp;
		};
	};
	struct Group {
		int16   band;
		int16 * fields;
		char  * lastval;
	};
	struct Band {
		int     SLAPI addField(int id);
		int16   kind;
		int16   ht;
		int16   group;
		uint16  options;
		int16 * fields;
	};

	static int defaultIterator(int);
	static SArray * FillRptArray();

	SLAPI  SReport(const char *);
	SLAPI  SReport(uint rezID, long flags /* INIREPF_XXX */);
	SLAPI ~SReport();
	int    SLAPI IsValid() const;
	void   SLAPI disableGrouping();
	int    SLAPI addField(int id, TYPEID typ, long fmt, uint rptfmt, char * nam);
	int    SLAPI setAggrToField(int fld, int aggr, int dpnd);
	int    SLAPI addText(char * txt);
	int    SLAPI addBand(SReport::Band * band, int * grp_fld, uint * pos);
	int    SLAPI calcAggr(int grp, int mode); // 0 - init, 1 - calc, 2 - summary
	int    SLAPI check();
	int    SLAPI setData(int id, void * data);
	int    SLAPI skipField(int id, int enable);
	int    SLAPI setIterator(ReportIterator);
	int    SLAPI setPrinter(SPrinter*);
	int    SLAPI setDefaultPrinter();
	int    SLAPI getNumCopies() const;
	int    SLAPI setNumCopies(int);
	int    SLAPI enumFields(SReport::Field **, SReport::Band *, int *);
	int    SLAPI getFieldName(SReport::Field *, char * buf, size_t buflen);
	int    SLAPI getFieldName(int id, char * buf, size_t buflen);
	int    SLAPI printDataField(SReport::Field * f);
	int    SLAPI printPageHead(int kind, int _newpage);
	int    SLAPI printGroupHead(int kind, int grp);
	int    SLAPI checkval(int16 * flds, char ** ptr);
	int    SLAPI printDetail();
	int    SLAPI printTitle(int kind);
	int    SLAPI writeResource(FILE *, uint);
	int    SLAPI readResource(TVRez *, uint resID);
	SReport::Band * SLAPI searchBand(int kind, int grp);
	int    SLAPI createDataFiles(const char * pDataName, const char * pRptPath);
	const  SString & SLAPI getDataName() const { return DataName; }
	int    PrnDest;
	int    SLAPI Export();
	int    SLAPI preview();
private:
	int    SLAPI prepareData();
	int    SLAPI createBodyDataFile(SString & rFileName, SCollection * fldIDs);
	int    SLAPI createVarDataFile(SString & rFileName, SCollection * fldIDs);
public:
	enum rptFlags {
		DisableGrouping = 0x0001,
		FooterOnBottom  = 0x0002,
		Landscape       = 0x0004,
		// @v8.6.6 @obsolete NoEjectAfter    = 0x0008,
		PrintingNoAsk   = 0x0010,
		NoRepError      = 0x0020, // �� �������� ��������� �� ������
		XmlExport       = 0x0040, // ������� � XML
		Preview         = 0x0080  // ��������������� ��������
	};
	SString Name;
	SString DataName;
	long   main_id;
	int    page;
	int    line;
	int    fldCount;
	Field * fields;
	int    agrCount;
	Aggr * agrs;
	int    grpCount;
	Group * groups;
	int    bandCount;
	Band * bands;
	//
	// ��������� ��� ���� ���������� ��� ������ � ������ � �������� ������ SPrinter
	//
	int    PageLen;
	int    LeftMarg;
	int    PrnOptions;
	int    PageHdHt; // ������ �������� �����������
	int    PageFtHt; // ������ ������� �����������
	int    Count;
	SPrinter * P_Prn;
	ReportIterator iterator;
private:
	int    Error;
	int    NumCopies;
	size_t TextLen;
	char * P_Text;
};

int SLAPI EditPrintParam(PrnDlgAns * pData);
int SLAPI CrystalReportPrint(const char *, const char * pDir, const char * pPrinter, int numCopies, int options);
int SLAPI CrystalReportExport(const char *, const char * pDir, const char * pReportName, const char * pEMailAddr, int options);
int SLAPI getReportBand();

#endif /* __REPORT_H */

