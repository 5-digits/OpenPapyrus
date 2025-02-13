// PPDBUTIL.CPP
// Copyright (c) A.Sobolev 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2016, 2017, 2018, 2019
// @codepage windows-1251
//
#include <pp.h>
#pragma hdrstop
// @v9.6.3 #include <idea.h>
// @v9.6.3 #include <dos.h>
//#include <sys\stat.h>
//
// ������ ������ ���������� ��������� ����� � ����� pp.ini
//
// backup_name=db_name,path,period,flags(1-with compression),max_copies

static const char * DefaultScenName = "Default";
static const char * BACKUP = "BACKUP";

DBTable * FASTCALL __PreprocessCreatedDbTable(DBTable * pT)
{
	if(!pT || !pT->IsOpened()) {
		ZDELETE(pT);
		PPSetErrorDB();
	}
	return pT;
}

// Prototype
static int SLAPI ProtectDatabase(DbLoginBlock * pDlb, int protect, char * pPw, char * pNewPw);
//
//
//
class PPRecoverParam : public BRecoverParam {
public:
	SLAPI PPRecoverParam() : BRecoverParam(), Stop(0)
	{
	}
	virtual int SLAPI callbackProc(int ev, const void * lp1, const void * lp2, const void * vp);
	int    Stop;
	SString LogFileName;
};

static int SLAPI LoadRcvrMsg(int msgID, SString & rBuf)
{
	return PPLoadText(msgID, rBuf);
}

int SLAPI PPRecoverParam::callbackProc(int ev, const void * lp1, const void * lp2, const void * vp)
{
	int    ok = 1, do_log_msg = 0;
	SString fmt_buf, msg_buf;
	switch(ev) {
		case BREV_START:
			if(!PPCheckUserBreak())
				ok = 0;
			else if(LoadRcvrMsg(PPTXT_RCVR_START, fmt_buf))
				msg_buf.Printf(fmt_buf, static_cast<const char*>(lp1), static_cast<const char*>(lp2));
			break;
		case BREV_FINISH:
			if(!PPCheckUserBreak())
				ok = 0;
			else if(LoadRcvrMsg(PPTXT_RCVR_FINISH, fmt_buf))
				msg_buf.Printf(fmt_buf, ActNumRecs, OrgNumRecs);
			break;
		case BREV_PROGRESS:
			if(!PPCheckUserBreak())
				ok = 0;
			else
				PPWaitPercent(reinterpret_cast<long>(lp1), reinterpret_cast<long>(lp2), static_cast<const char *>(vp));
			break;
		case BREV_ERRCREATE:
			if(LoadRcvrMsg(PPTXT_RCVR_ERRCREATE, fmt_buf))
				msg_buf.Printf(fmt_buf, static_cast<const char *>(lp1));
			break;
		case BREV_ERRINS:
			if(LoadRcvrMsg(PPTXT_RCVR_ERRINS, fmt_buf))
				msg_buf.Printf(fmt_buf, BtrError, reinterpret_cast<RECORDNUMBER>(lp1));
			break;
		case BREV_ERRSTEP:
			if(LoadRcvrMsg(PPTXT_RCVR_ERRSTEP, fmt_buf))
				msg_buf.Printf(fmt_buf, BtrError, reinterpret_cast<RECORDNUMBER>(lp1));
			break;
		case BREV_ERRDELPREV:
			if(LoadRcvrMsg(PPTXT_RCVR_ERRDELPREV, fmt_buf))
				msg_buf.Printf(fmt_buf, static_cast<const char *>(lp1));
			break;
		case BREV_ERRRENAME:
			if(LoadRcvrMsg(PPTXT_RCVR_ERRRENAME, fmt_buf))
				msg_buf.Printf(fmt_buf, static_cast<const char *>(lp1), static_cast<const char *>(lp2));
			break;
		default:
			break;
	}
	if(ok == 0) {
		Stop = 1;
		if(LoadRcvrMsg(PPTXT_RCVR_USRBRK, fmt_buf))
			msg_buf = fmt_buf;
	}
	if(msg_buf.NotEmptyS())
		PPLogMessage(LogFileName, msg_buf, LOGMSGF_TIME);
	return ok;
}

struct PPRecoverInfo {
	long   OrgNumRecs;
	long   ActNumRecs;
	long   NotRcvrdNumRecs;
	char   TableName[48];
};

static int SLAPI _Recover(BTBLID tblID, PPRecoverParam * pParam, SArray * pRecoverInfoAry)
{
	int    r = 1;
	int64  disk_total = 0, disk_avail = 0;
	DbTableStat ts;
	SString path, msg_buf, fmt_buf, tbl_name;
	DbProvider * p_db = CurDict;
	p_db->GetTableInfo(tblID, &ts);
	path = ts.Location;
	SLibError = SLERR_FILENOTFOUND;
	THROW_PP_S(p_db->IsFileExists_(p_db->MakeFileName_(ts.TblName, path)) > 0, PPERR_SLIB, path);
	{
		PPRecoverInfo r_info;
		MEMSZERO(r_info);
		SFileUtil::Stat st;
		SFileUtil::GetStat(path, &st);
		SFileUtil::GetDiskSpace(path, &disk_total, &disk_avail);
		r = BIN(disk_avail > (st.Size * 2)); // *2 - ����������� ������
		DBErrCode = SDBERR_BU_NOFREESPACE;
		THROW_DB(r);
		if(!p_db->RecoverTable(tblID, pParam)) {
			if(LoadRcvrMsg(PPTXT_RCVR_ERROR, fmt_buf)) {
				msg_buf.Printf(fmt_buf, BtrError);
				PPLogMessage(pParam->LogFileName, msg_buf, LOGMSGF_TIME);
			}
		}
		else {
			DbTableStat ts;
			p_db->GetTableInfo(tblID, &ts);
			msg_buf.Printf(PPLoadTextS(PPTXT_SUCCRCVRTBL, fmt_buf), ts.TblName.cptr());
			PPLogMessage(pParam->LogFileName, msg_buf, LOGMSGF_TIME);
		}
		r_info.OrgNumRecs = pParam->OrgNumRecs;
		r_info.ActNumRecs = pParam->ActNumRecs;
		r_info.NotRcvrdNumRecs = r_info.OrgNumRecs - r_info.ActNumRecs;
		{
			SPathStruc ps(path);
			ps.Nam.CopyTo(r_info.TableName, sizeof(r_info.TableName));
		}
		pRecoverInfoAry->insert(&r_info);
	}
	CATCH
		PPGetLastErrorMessage(1, msg_buf);
		PPLogMessage(pParam->LogFileName, msg_buf, LOGMSGF_TIME);
	ENDCATCH
	return (pParam->Stop || !r) ? 0 : 1;
}
//
//
//
PPBackupScen::PPBackupScen()
{
	THISZERO();
}

int SLAPI PPBackupScen::ToStr(SString & rBuf) const
{
	rBuf.Z().Cat(DBName).CatDiv(',', 0).Cat(BackupPath).CatDiv(',', 0).
		Cat(Period).CatDiv(',', 0).Cat(Flags).CatDiv(',', 0).Cat(numCopies);
	return 1;
}

int CallbackCompress(long a, long b, const char * c, int stop)
{
	PPWaitPercent(a, b, c);
	return (!stop || PPCheckUserBreak()) ? 1 : 0;
}

static int CallbackBuLog(int event, const char * pInfo, long initParam)
{
	int    ok = 1;
	int    msg_code = 0;
	long   log_options = LOGMSGF_TIME;
	SString inv_addinfo, err_msg_buf;
	const  char * p_addinfo = isempty(pInfo) ? PPLoadTextS(PPTXT_BACKUPLOG_INVADVOPT, inv_addinfo) : pInfo;
	switch(event) {
		case BACKUPLOG_BEGIN:            msg_code = PPTXT_BACKUPLOG_BEGIN;            break;
		case BACKUPLOG_END:              msg_code = PPTXT_BACKUPLOG_END;              break;
		case BACKUPLOG_SUC_COPY:         msg_code = PPTXT_BACKUPLOG_SUC_COPY;         break;
		case BACKUPLOG_ERR_COPY:         msg_code = PPTXT_BACKUPLOG_ERR_COPY;         break;
		case BACKUPLOG_SUC_RESTORE:      msg_code = PPTXT_BACKUPLOG_SUC_RESTORE;      break;
		case BACKUPLOG_ERR_RESTORE:      msg_code = PPTXT_BACKUPLOG_ERR_RESTORE;      break;
		case BACKUPLOG_ERR_GETFILEPARAM: msg_code = PPTXT_BACKUPLOG_ERR_GETFILEPARAM; break;
		case BACKUPLOG_ERR_COMPRESS:     msg_code = PPTXT_BACKUPLOG_ERR_COMPRESS;     break;
		case BACKUPLOG_ERR_DECOMPRESS:   msg_code = PPTXT_BACKUPLOG_ERR_DECOMPRESS;   break;
		case BACKUPLOG_SUC_REMOVE:       msg_code = PPTXT_BACKUPLOG_SUC_REMOVE;       break;
		case BACKUPLOG_ERROR:
			{
				PPGetLastErrorMessage(1, err_msg_buf);
				p_addinfo = err_msg_buf;
				msg_code = PPTXT_BACKUPLOG_ERROR;
			}
			break;
		case BACKUPLOG_ERR_DECOMPRESSCRC: msg_code = PPTXT_BACKUPLOG_ERR_DECOMPRESSCRC; break;
		default: ok = -1;
	}
	if(msg_code) {
		SString fmt_buf, msg_buf;
		if(p_addinfo)
			msg_buf.Printf(PPLoadTextS(msg_code, fmt_buf), p_addinfo);
		else
			PPLoadText(msg_code, msg_buf);
		PPLogMessage(PPFILNAM_BACKUP_LOG, msg_buf, log_options);
	}
	return ok;
}

// static
PPBackup * SLAPI PPBackup::CreateInstance(const PPDbEntrySet2 * dbes)
{
	PPBackup * ppb = 0;
	PPID   db_id = dbes->GetSelection();
	DbLoginBlock dlb;
	if(dbes->GetByID(dbes->GetSelection(), &dlb)) {
		SString db_name;
		dlb.GetAttr(DbLoginBlock::attrDbSymb, db_name);
		if(DS.OpenDictionary2(&dlb, 0)) {
			ppb = new PPBackup(db_name, CurDict);
			if(!ppb || !ppb->IsValid())
				ZDELETE(ppb);
		}
	}
	return ppb;
}

SLAPI PPBackup::PPBackup(const char * pDbName, DbProvider * pDb) : DBBackup(), State(stValid), P_ScenList(0), P_Sync(0)
{
	SString data_path;
	THROW_MEM(P_Sync = new PPSync);
	pDb->GetDataPath(data_path);
	THROW(P_Sync->Init(data_path));
	SetDictionary(pDb);
	STRNSCPY(DBName, pDbName);
	THROW_MEM(P_ScenList = new SArray(sizeof(PPBackupScen)));
	GetScenList(P_ScenList);
	CATCH
		State &= ~stValid;
	ENDCATCH
}

SLAPI PPBackup::~PPBackup()
{
	delete P_ScenList;
	delete P_Sync;
}

int SLAPI PPBackup::IsValid() const
{
	return BIN(State & stValid);
}

int SLAPI PPBackup::CBP_CopyProcess(const char * pSrcFile, const char * /*pDestFile*/,
	int64 totalSize, int64 /*fileSize*/, int64 totalBytesReady, int64 /*fileBytesReady*/)
{
	long   pct = (long)(100L * totalBytesReady / totalSize);
	PPWaitPercent(pct, pSrcFile);
	return SPRGRS_CONTINUE;
}

int SLAPI PPBackup::GetDefaultBackupPath(char * pPath) const
{
	pPath[0] = 0;
	SString data_path;
	P_Db->GetDataPath(data_path);
	strcat(setLastSlash(strcpy(pPath, data_path)), BACKUP);
	return 1;
}

int SLAPI PPBackup::GetDefaultScen(PPBackupScen * pScen)
{
	int    ok = 1;
	if(pScen) {
		memzero(pScen, sizeof(PPBackupScen));
		pScen->ID = 0;
		STRNSCPY(pScen->Name, DefaultScenName);
		STRNSCPY(pScen->DBName, DBName);
		pScen->Period = 1;
		pScen->numCopies = 1;
		GetDefaultBackupPath(pScen->BackupPath);
	}
	else
		ok = PPSetErrorInvParam();
	return ok;
}

int SLAPI PPBackup::GetScenList(SArray * pScenList)
{
	int    ok = 1;
	if(pScenList) {
		uint   pos = 0;
		const  char * p_sect = BACKUP;
		SString buf;
		StringSet temp;
		PPIniFile ini_file;
		THROW(ini_file.IsValid());
		pScenList->freeAll();
		ini_file.GetEntries(p_sect, &temp);
		for(pos = 0; temp.get(&pos, buf);) {
			uint   i = 0;
			PPBackupScen entry;
			STRNSCPY(entry.Name, buf);
			ini_file.GetParam(p_sect, entry.Name, buf);
			StringSet ss(',', buf);
			ss.get(&i, entry.DBName, sizeof(entry.DBName));
			if(stricmp866(entry.DBName, DBName) == 0) {
				int    r = ss.get(&i, entry.BackupPath, sizeof(entry.BackupPath));
				if(!r || entry.BackupPath[0] == 0)
					GetDefaultBackupPath(entry.BackupPath);
				if(r) {
					if(ss.get(&i, buf)) {
						entry.Period = buf.ToLong();
						if(entry.Period < 1 || entry.Period > 365)
							entry.Period = 1;
					}
					else
						entry.Period = 1;
					if(ss.get(&i, buf)) {
						entry.Flags = buf.ToLong();
						if(entry.Flags != 0)
							entry.Flags = 1;
					}
					else
						entry.Flags = 0;
					if(ss.get(&i, buf)) {
						entry.numCopies = buf.ToLong();
						if(entry.numCopies <= 0)
							entry.numCopies = 1;
					}
					else
						entry.numCopies = 1;
				}
				THROW_SL(pScenList->insert(&entry));
			}
		}
		if(pScenList->getCount() == 0) {
			PPBackupScen entry;
			THROW(GetDefaultScen(&entry));
			THROW_SL(pScenList->insert(&entry));
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPBackup::EnumScen(long * pPos, PPBackupScen * pScen)
{
	int    ok = 1;
	uint   p = (uint)*pPos;
	if(P_ScenList && p < P_ScenList->getCount()) {
		*pScen = *(PPBackupScen*)P_ScenList->at(p);
		(*pPos)++;
	}
	else
		ok = 0;
	return ok;
}

int SLAPI PPBackup::GetScen(long id, PPBackupScen * pScen)
{
	int    ok = 1;
	if(P_ScenList && id > 0 && id <= (long)P_ScenList->getCount())
		*pScen = *static_cast<const PPBackupScen *>(P_ScenList->at((uint)(id - 1)));
	else
		ok = 0;
	return ok;
}

int SLAPI PPBackup::GetLastScenCopy(PPBackupScen * pScen, BCopyData * bcdata)
{
	BCopyData bcd;
	BCopyData * p_pbcd = &bcd;
	BCopySet bcset(pScen->Name);
	GetCopySet(&bcset);
	bcset.Sort(BCopySet::ordByDateDesc);
	uint i = 0;
	if(bcset.enumItems(&i, (void **)&p_pbcd)) {
		*bcdata = *p_pbcd;
		return 1;
	}
	else
		return 0;
}
//
// Config backup - from PPConfig
//
int SLAPI getScenList(SArray * scenList)
{
	int    ok = 1;
	uint   pos = 0;
	SString buf;
	StringSet   temp;
	scenList->freeAll();
	PPIniFile ini_file;
	ini_file.GetEntries(BACKUP, &temp);
	for(pos = 0; temp.get(&pos, buf);) {
		uint   i = 0;
		PPBackupScen entry;
		STRNSCPY(entry.Name, buf);
		ini_file.GetParam(BACKUP, entry.Name, buf);
		StringSet ss(',', buf);
		ss.get(&i, entry.DBName, sizeof(entry.DBName));
		int    r = ss.get(&i, entry.BackupPath, sizeof(entry.BackupPath));
		if(r) {
			if(ss.get(&i, buf)) {
				entry.Period = buf.ToLong();
				if(entry.Period < 1 || entry.Period > 365)
					entry.Period = 1;
			}
			else
				entry.Period = 1;
			if(ss.get(&i, buf)) {
				entry.Flags = buf.ToLong();
				if(entry.Flags != 0)
					entry.Flags = 1;
			}
			else
				entry.Flags = 0;
			if(ss.get(&i, buf)) {
				entry.numCopies = buf.ToLong();
				if(entry.numCopies <= 0)
					entry.numCopies = 1;
			}
			else
				entry.numCopies = 1;
		}
		THROW_SL(scenList->insert(&entry));
	}
	CATCHZOK
	return ok;
}

static int SLAPI SetupListBox(TView * pList, uint sz, uint fl, uint lbfl)
{
	if(pList) {
		pList->ViewOptions |= lbfl;
		StringListBoxDef * def = new StringListBoxDef(sz, NZOR(fl, (lbtDisposeData | lbtDblClkNotify)));
		if(def == 0)
			return PPSetErrorNoMem();
		else {
			static_cast<SmartListBox *>(pList)->setDef(def);
			return 1;
		}
	}
	return -1;
}

class ConfigBackupDialog : public TDialog {
public:
	ConfigBackupDialog(PPIniFile * pIniFile) : TDialog(DLG_BUCFG_SELECT), P_IniFile(pIniFile)
	{
		P_List = static_cast<SmartListBox *>(getCtrlView(CTL_BUCFG_SCNAME));
		SetupListBox(P_List, 64, lbtFocNotify|lbtDisposeData|lbtDblClkNotify, ofFramed);
		DBES.ReadFromProfile(P_IniFile, 0);
		P_ScenList = new SArray(sizeof(PPBackupScen));
		updateList();
	}
	virtual ~ConfigBackupDialog()
	{
		delete P_ScenList;
	}
private:
	DECL_HANDLE_EVENT;
	int    editEntry(int isNewEntry, PPBackupScen *);
	int    updateList();
	int    addEntry();
	int    updateEntry();
	int    deleteEntry();
	PPDbEntrySet2 DBES;
	SmartListBox * P_List;
	SArray       * P_ScenList;
	PPIniFile    * P_IniFile;
};

int ConfigBackupDialog::updateList()
{
	if(P_ScenList && P_List) {
		getScenList(P_ScenList);
		P_List->freeAll();
		SString n, pn;
		for(uint i = 0; i < P_ScenList->getCount(); i++) {
			char sub[128];
			StringSet ss(SLBColumnDelim);
			PPBackupScen * p_entry = (PPBackupScen *)P_ScenList->at(i);
			ss.add(strip(strcpy(sub, p_entry->Name)));
			DBES.GetAttr(p_entry->DBName, DbLoginBlock::attrDbSymb, n);
			DBES.GetAttr(p_entry->DBName, DbLoginBlock::attrDbFriendlyName, pn);
			pn.SetIfEmpty(n);
			ss.add(pn.Strip());
			P_List->addItem(i+1, ss.getBuf());
		}
		P_List->Draw_();
	}
	return 1;
}

int ConfigBackupDialog::editEntry(int isNewEntry, PPBackupScen * pEntry)
{
	int    ok = -1, valid_data = 0;
	SString cc, temp_buf;
	TDialog * dlg = new TDialog(DLG_BUCFG_EDIT);
	PPBackupScen entry = *pEntry;
	THROW(CheckDialogPtr(&dlg));
	dlg->setCtrlData(CTL_BUCFG_CFGNAME,  entry.Name);
	dlg->setCtrlData(CTL_BUCFG_PERIOD,   &(entry.Period));
	dlg->setCtrlData(CTL_BUCFG_COPIES,   &(entry.numCopies));
	dlg->setCtrlData(CTL_BUCFG_COMPRESS, &(entry.Flags));
	dlg->setCtrlData(CTL_BUCFG_PATH,     entry.BackupPath);
	SetupDBEntryComboBox(dlg, CTLSEL_BUCFG_DBNAME, &DBES);
	dlg->disableCtrl(CTL_BUCFG_CFGNAME, !isNewEntry);
	FileBrowseCtrlGroup::Setup(dlg, CTLBRW_BUCFG_PATH, CTL_BUCFG_PATH, 1, PPTXT_TITLE_SELBACKUPPATH, 0, FileBrowseCtrlGroup::fbcgfPath);
	while(!valid_data && ExecView(dlg) == cmOK) {
		uint   sel = 0;
		PPID   dbid = 0;
		int    err_text = 0;
		char   temp_scen_name[64];
		dlg->getCtrlData(CTLSEL_BUCFG_DBNAME, &dbid);
		DBES.GetAttr(dbid, DbLoginBlock::attrDbSymb, cc);
		STRNSCPY(entry.DBName, cc.Strip());
		dlg->getCtrlData(CTL_BUCFG_CFGNAME, temp_scen_name);
		dlg->getCtrlData(CTL_BUCFG_PERIOD, &(entry.Period));
		dlg->getCtrlData(CTL_BUCFG_COPIES, &(entry.numCopies));
		dlg->getCtrlData(CTL_BUCFG_COMPRESS, &(entry.Flags));
		dlg->getCtrlData(CTL_BUCFG_PATH, entry.BackupPath);
		if(entry.numCopies <= 0 || entry.numCopies > 99) {
			sel = CTL_BUCFG_COPIES;
			err_text = PPINF_BADCOPYNO;
		}
		else if(entry.Period <= 0 || entry.Period > 365) {
			sel = CTL_BUCFG_PERIOD;
			err_text = PPINF_BADPERIOD;
		}
		else if(*strip(entry.BackupPath) == 0) {
			sel = CTL_BUCFG_PATH;
			err_text = PPINF_SAVEDIR;
		}
		else if(!dbid) {
			sel = CTL_BUCFG_DBNAME;
			err_text = PPINF_COPYDB;
		}
		else if(entry.Name[0] == 0) {
			if(*strip(temp_scen_name) == 0) {
				sel = CTL_BUCFG_CFGNAME;
				err_text = PPINF_SCENNAME;
			}
			else if(P_IniFile->GetParam(BACKUP, temp_scen_name, temp_buf) > 0) {
				sel = CTL_BUCFG_CFGNAME;
				err_text = PPINF_SCENEXIST;
			}
			else {
				STRNSCPY(entry.Name, temp_scen_name);
				valid_data = 1;
			}
		}
		else
			valid_data = 1;
		if(!valid_data) {
			if(err_text)
				PPMessage(mfInfo, err_text);
			dlg->selectCtrl(sel);
		}
		else {
			ok = 1;
			*pEntry = entry;
		}
	}
	delete dlg;
	CATCHZOK
	return ok;
}

int ConfigBackupDialog::addEntry()
{
	int    ok = -1;
	PPBackupScen entry;
	entry.Period = 1;
	entry.numCopies = 3;
	if(editEntry(1, &entry) > 0) {
		SString temp_buf;
		entry.ToStr(temp_buf);
		P_IniFile->AppendParam(BACKUP, entry.Name, temp_buf, 1);
		updateList();
		ok = 1;
	}
	return ok;
}

int ConfigBackupDialog::updateEntry()
{
	int    ok = -1;
	if(P_ScenList && P_IniFile) {
		PPID   ssid = 0;
		SString org_entry_name, buf;
		getCtrlData(CTL_BUCFG_SCNAME, &ssid);
		if(ssid) {
			PPBackupScen * p_entry = (PPBackupScen *)P_ScenList->at((int)ssid-1);
			org_entry_name = p_entry->Name;
			DBES.SetSelection(DBES.GetBySymb(p_entry->DBName, 0));
			if(editEntry(1, p_entry) > 0) {
				p_entry->ToStr(buf);
				P_IniFile->RemoveParam(BACKUP, org_entry_name);
				P_IniFile->AppendParam(BACKUP, p_entry->Name, buf, 1);
				updateList();
				ok = 1;
			}
		}
	}
	return ok;
}

int ConfigBackupDialog::deleteEntry()
{
	int    ok = -1;
	if(P_ScenList && P_IniFile) {
		PPID   ssid = 0;
		getCtrlData(CTL_BUCFG_SCNAME, &ssid);
		if(ssid) {
			PPBackupScen * p_entry = (PPBackupScen *)P_ScenList->at((int)ssid-1);
			if(PPMessage(mfConf, PPCFM_REMOVECFG, p_entry->Name) == cmYes) {
				P_IniFile->RemoveParam(BACKUP, p_entry->Name);
				updateList();
				ok = 1;
			}
		}
	}
	return ok;
}

IMPL_HANDLE_EVENT(ConfigBackupDialog)
{
	TDialog::handleEvent(event);
	if(TVCOMMAND) {
		if(TVCMD == cmaInsert)
			addEntry();
		else if(TVCMD == cmaEdit)
			updateEntry();
		else if(TVCMD == cmaDelete)
			deleteEntry();
		else
			return;
	}
}

int ConfigBackup()
{
	int    ok = 1;
	ConfigBackupDialog * dlg = 0;
	PPIniFile ini_file;
	THROW_SL(ini_file.IsValid());
	dlg = new ConfigBackupDialog(&ini_file);
	THROW(CheckDialogPtr(&dlg));
	ExecViewAndDestroy(dlg);
	CATCHZOKPPERR
	return ok;
}
//
// Dump database
//
class PrcssrDbDump {
public:
	enum {
		spcobNone = 0,
		spcobQuot
	};
	struct Param {
		long   Mode;       // 0 - read, 1 - write
		long   TblID;      // ������������� ������������ �������, ������� ������� ����������.
		int32  SpcOb;      // @v10.0.05 ����������� ������ ������������ (spcobXXX)
		SString DbSymb;
		SString TableName;
		SString FileName;
	};

	explicit PrcssrDbDump(PPDbEntrySet2 * pDbes);
	~PrcssrDbDump();
	int    SLAPI IsValid() const;
	int    SLAPI GetTableListInFile(const char * pFileName, StrAssocArray * pList);
	int	   SLAPI InitParam(Param *);
	int	   SLAPI EditParam(Param *);
	int	   SLAPI Init(const Param *);
	int	   SLAPI Run();
private:
	struct DumpHeader {
		uint32 Signature;   // PPDD 0x44445050
		uint32 Crc32;
		SVerT  Ver;
		uint32 Flags;
		int64  CtxOffs;
		uint32 SpecialObject; // @v10.0.04
		uint8  Reserve[36];
	};
	struct TableEntry {
		long   Id;
		int64  Offs;
		int64  NumRecs;
		int64  NumChunks;
	};
	int    SLAPI OpenStream(const char * pFileName);
	int    SLAPI CloseStream();
	int    SLAPI Helper_Dump(long tblID);
	int    SLAPI Helper_Undump(long tblID);

	enum {
		stValid   = 0x0001,
		stOwnDbes = 0x0002
	};
	long   State;
	PPDbEntrySet2 * P_Dbes;
	StrAssocArray TblNameList;
	SArray TblEntryList;
	SSerializeContext Ctx;
	SFile  FDump; // ���� �����
	uint32 MaxBufLen;
	int    Valid;
	Param  P;
};

SLAPI PrcssrDbDump::PrcssrDbDump(PPDbEntrySet2 * pDbes) : TblEntryList(sizeof(PrcssrDbDump::TableEntry)), MaxBufLen(1024 * 1024), State(stValid), Valid(1)
{
	if(pDbes)
		P_Dbes = pDbes;
	else {
		P_Dbes = new PPDbEntrySet2;
		PPIniFile ini_file;
		P_Dbes->ReadFromProfile(&ini_file);
		State |= stOwnDbes;
	}
	MEMSZERO(P);
}

SLAPI PrcssrDbDump::~PrcssrDbDump()
{
	if(State & stOwnDbes)
		delete P_Dbes;
}

int SLAPI PrcssrDbDump::EditParam(Param * pData)
{
	class DbDumpDialog : public TDialog {
	public:
		DbDumpDialog(PPDbEntrySet2 * pDbes) : TDialog(DLG_DBDUMP), P_Dbes(pDbes)
		{
			assert(pDbes);
			FileBrowseCtrlGroup::Setup(this, CTLBRW_DBDUMP_FILE, CTL_DBDUMP_FILE, 1, 0, PPTXT_FILPAT_DBDUMP,
				FileBrowseCtrlGroup::fbcgfFile | FileBrowseCtrlGroup::fbcgfAllowNExists);
		}
		int    setDTS(const PrcssrDbDump::Param * pData)
		{
			int    ok = 1;
			Data = *pData;
			AddClusterAssoc(CTL_DBDUMP_ACTION, 0, 1);
			AddClusterAssoc(CTL_DBDUMP_ACTION, 1, 0);
			SetClusterData(CTL_DBDUMP_ACTION, Data.Mode);
			// @v10.0.05 {
			AddClusterAssoc(CTL_DBDUMP_SPCOB, 0, spcobNone);
			AddClusterAssoc(CTL_DBDUMP_SPCOB, 1, spcobQuot);
			SetClusterData(CTL_DBDUMP_SPCOB, Data.SpcOb);
			disableCtrl(CTLSEL_DBDUMP_TBL, Data.SpcOb != spcobNone);
			DisableClusterItem(CTL_DBDUMP_ACTION, 1, (Data.SpcOb != spcobNone && !DS.GetConstTLA().IsAuth()));
			DisableClusterItem(CTL_DBDUMP_SPCOB, 0, DS.GetConstTLA().IsAuth());
			// } @v10.0.05
			/*
			DbLoginBlock blk;
			if(Dbes.GetBySymb(Data.DbSymb, &blk)) {
				SString temp_buf;
				blk.GetAttr(DbLoginBlock::attrID, temp_buf);
				Dbes.SetSelection(temp_buf.ToLong()-1);
			}
			*/
			SetupTblCombo(Data.Mode);
			SetupDBEntryComboBox(this, CTLSEL_DBDUMP_DB, P_Dbes);
			setCtrlString(CTL_DBDUMP_FILE, Data.FileName);
			return ok;
		}
		int    getDTS(PrcssrDbDump::Param * pData)
		{
			int    ok = 1;
			GetClusterData(CTL_DBDUMP_ACTION, &Data.Mode);
			long   db_id = getCtrlLong(CTLSEL_DBDUMP_DB);
			DbLoginBlock blk;
			P_Dbes->SetSelection(db_id);
			THROW_SL(P_Dbes->GetByID(db_id, &blk));
			blk.GetAttr(DbLoginBlock::attrDbSymb, Data.DbSymb);
			Data.TblID = static_cast<BTBLID>(getCtrlLong(CTLSEL_RECOVER_TBL));
			getCtrlString(CTL_DBDUMP_FILE, Data.FileName);
			Data.FileName.Strip();
			ASSIGN_PTR(pData, Data);
			CATCHZOKPPERR
			return ok;
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(event.isCbSelected(CTLSEL_DBDUMP_DB)) {
				long   db_id = getCtrlLong(CTLSEL_DBDUMP_DB);
				P_Dbes->SetSelection(db_id);
				SetupTblCombo(Data.Mode);
			}
			else if(event.isClusterClk(CTL_DBDUMP_ACTION)) {
				long   new_mode = 0;
				GetClusterData(CTL_DBDUMP_ACTION, &new_mode);
				if(new_mode != Data.Mode) {
					Data.TblID = 0;
					SetupTblCombo(new_mode);
					Data.Mode = new_mode;
				}
			}
			else if(event.isClusterClk(CTL_DBDUMP_SPCOB)) {
				GetClusterData(CTL_DBDUMP_SPCOB, &Data.SpcOb);
				disableCtrl(CTLSEL_DBDUMP_TBL, Data.SpcOb != spcobNone);
				DisableClusterItem(CTL_DBDUMP_ACTION, 1, (Data.SpcOb != spcobNone && !DS.GetConstTLA().IsAuth()));
			}
			else if(TVBROADCAST && TVCMD == cmCommitInput) {
				GetClusterData(CTL_DBDUMP_ACTION, &Data.Mode);
				if(Data.Mode == 0) {
					Data.TblID = 0;
					SetupTblCombo(Data.Mode);
				}
				else {
					getCtrlString(CTL_DBDUMP_FILE, Data.FileName);
					SPathStruc::ReplaceExt(Data.FileName, "ppdump", 0);
					setCtrlString(CTL_DBDUMP_FILE, Data.FileName);
				}
			}
			else
				return;
			clearEvent(event);
		}
		int    SetupTblCombo(int mode)
		{
			int    ok = -1;
			if(mode == 1) {
				SetupDBTableComboBox(this, CTLSEL_DBDUMP_TBL, P_Dbes, P_Dbes->GetSelection(), static_cast<BTBLID>(Data.TblID));
				ok = 1;
			}
			else if(mode == 0) {
				getCtrlString(CTL_DBDUMP_FILE, Data.FileName);
				ComboBox * cb = static_cast<ComboBox *>(getCtrlView(CTLSEL_DBDUMP_TBL));
				if(cb && Data.FileName.NotEmptyS() && fileExists(Data.FileName)) {
					PrcssrDbDump dump(P_Dbes);
					StrAssocArray * p_tbl_list = new StrAssocArray;
					if(dump.GetTableListInFile(Data.FileName, p_tbl_list)) {
						p_tbl_list->SortByText();
						cb->setListWindow(CreateListWindow(p_tbl_list, lbtDisposeData), 0);
						ok = 1;
					}
					else {
						ZDELETE(p_tbl_list);
						ok = PPErrorZ();
					}
				}
			}
			return ok;
		}

		PPDbEntrySet2 * P_Dbes; // DbLoginBlockArray
		PrcssrDbDump::Param Data;
	};

	DIALOG_PROC_BODY_P1(DbDumpDialog, P_Dbes, pData);
}

int SLAPI PrcssrDbDump::InitParam(Param * pData)
{
	if(pData) {
		pData->TblID = 0;
		pData->Mode = 1;
		if(DS.GetConstTLA().IsAuth()) {
			pData->SpcOb = spcobQuot;
			CurDict->GetDbSymb(pData->DbSymb);
			if(pData->DbSymb.NotEmpty()) {
				int   db_id = P_Dbes->GetBySymb(pData->DbSymb, 0);
				if(db_id > 0)
					P_Dbes->SetSelection(db_id);
			}
		}
		else {
			pData->SpcOb = spcobNone;
			pData->DbSymb.Z();
		}
	}
	return 1;
}

int SLAPI PrcssrDbDump::Init(const Param * pData)
{
	RVALUEPTR(P, pData);
	return 1;
}

int SLAPI PrcssrDbDump::Run()
{
	int    ok = 1;
	int    db_locked = 0;
	THROW(OpenStream(P.FileName));
	if(P.SpcOb == spcobNone) {
		DbLoginBlock dlb;
		PPIniFile ini_file;
		PPDbEntrySet2 dbes;
		THROW(DS.GetConstTLA().IsAuth() == 0);
		dbes.ReadFromProfile(&ini_file);
		THROW_SL(dbes.GetBySymb(P.DbSymb, &dlb));
		THROW(DS.OpenDictionary2(&dlb, 0));
		THROW(DS.GetSync().LockDB());
		db_locked = 1;
	}
	PPWait(1);
	if(P.Mode == 1) {
		if(P.SpcOb == spcobNone) {
			if(P.TblID) {
				THROW(Helper_Dump(P.TblID));
			}
			else {
				StrAssocArray tbl_list;
				CurDict->GetListOfTables(0, &tbl_list);
				SString path;
				for(uint j = 0; j < tbl_list.getCount(); j++) {
					THROW(Helper_Dump(tbl_list.Get(j).Id));
				}
			}
		}
		else if(P.SpcOb == spcobQuot) {
			THROW(Helper_Dump(0));
		}
	}
	else if(P.Mode == 0) {
		if(P.SpcOb == spcobNone) {
			if(P.TblID) {
				THROW(Helper_Undump(P.TblID));
			}
			else {
				for(uint j = 0; j < TblNameList.getCount(); j++) {
					THROW(Helper_Undump(TblNameList.Get(j).Id));
				}
			}
		}
		else if(P.SpcOb == spcobQuot) {
			THROW(Helper_Undump(0));
		}
	}
	THROW(CloseStream());
	PPWait(0);
	CATCH
		CloseStream();
		ok = 0;
	ENDCATCH
	if(db_locked)
		THROW(DS.GetSync().UnlockDB());
	return ok;
}

int SLAPI PrcssrDbDump::IsValid() const
{
	return BIN(Valid);
}

int SLAPI PrcssrDbDump::OpenStream(const char * pFileName)
{
	int    ok = -1;
	DumpHeader hdr;
	Ctx.Init(SSerializeContext::fSeparateDataStruct, getcurdate_());
	if(P.Mode) {
		THROW_SL(FDump.Open(pFileName, SFile::mReadWriteTrunc | SFile::mBinary | SFile::mNoStd));
		MEMSZERO(hdr);
		hdr.Signature = 0x44445050;
		hdr.Ver = DS.GetVersion();
		THROW_SL(FDump.Write(&hdr, sizeof(hdr)));
		ok = 1;
	}
	else {
		uint32 crc = 0;
		SBuffer buffer;
		THROW_SL(FDump.Open(pFileName, SFile::mRead | SFile::mBinary | SFile::mNoStd));
		THROW_SL(FDump.Read(&hdr, sizeof(hdr)));
		THROW(hdr.Signature == 0x44445050);
		THROW_SL(FDump.CalcCRC(sizeof(hdr), &crc));
		THROW(hdr.Crc32 == crc);

		FDump.Seek64(hdr.CtxOffs);
		FDump.Read(buffer);
		THROW_SL(Ctx.SerializeStateOfContext(-1, buffer));
		TblEntryList.clear();
		THROW_SL(Ctx.Serialize(-1, &TblEntryList, buffer));
		THROW_SL(TblNameList.Read(buffer, 0));
	}
	CATCHZOK
	return ok;
}

int SLAPI PrcssrDbDump::CloseStream()
{
	int    ok = 1;
	if(P.Mode == 1) {
		DumpHeader hdr;
		SBuffer buffer;
		int64  state_offs = FDump.Tell64();
		THROW_SL(Ctx.SerializeStateOfContext(+1, buffer));
		THROW_SL(Ctx.Serialize(+1, &TblEntryList, buffer));
		THROW_SL(TblNameList.Write(buffer, 0));
		THROW_SL(FDump.Write(buffer));

		MEMSZERO(hdr);
		hdr.Signature = 0x44445050;
		hdr.Ver = DS.GetVersion();
		hdr.CtxOffs = state_offs;
		THROW_SL(FDump.CalcCRC(sizeof(hdr), &hdr.Crc32));
		FDump.Seek64(0);
		THROW_SL(FDump.Write(&hdr, sizeof(hdr)));
	}
	CATCHZOK
	FDump.Close();
	return ok;
}

int SLAPI PrcssrDbDump::GetTableListInFile(const char * pFileName, StrAssocArray * pList)
{
	int    ok = 1;
	P.Mode = 0;
	THROW(OpenStream(pFileName));
	ASSIGN_PTR(pList, TblNameList);
	CATCHZOK
	return ok;
}

int SLAPI PrcssrDbDump::Helper_Undump(long tblID)
{
	int    ok = 1;
	// @v10.3.0 (never used) int    ref_allocated = 0;
	uint   pos = 0;
	SBuffer buffer;
	THROW(P.Mode == 0);
	if(P.SpcOb == spcobNone) {
		if(TblEntryList.lsearch(&tblID, &pos, PTR_CMPFUNC(long))) {
			const TableEntry & r_entry = *static_cast<const TableEntry *>(TblEntryList.at(pos));
			SString tbl_name;
			TblNameList.GetText(tblID, tbl_name);
			THROW_SL(FDump.Seek64(r_entry.Offs));
			{
				DbProvider * p_dict = CurDict;
				IterCounter cntr;
				DBTable tbl(tbl_name);
				int    has_lob = 0;
				DBField lob_fld;
				RECORDSIZE fix_rec_size = tbl.getRecSize();
				THROW(tbl.allocOwnBuffer(16*1024));
				if(tbl.HasLob(&lob_fld) > 0) {
					has_lob = 1;
				}
				THROW_DB(p_dict->RenewFile(tbl, 0, 0));
				{
					PPTransaction tra(1);
					THROW(tra);
					cntr.Init((long)r_entry.NumRecs); // @32-64
					for(int64 i = 0; i < r_entry.NumChunks; i++) {
						int64  local_count = 0;
						buffer.Z();
						THROW_SL(FDump.Read(&local_count, sizeof(local_count)));
						THROW_SL(FDump.Read(buffer));
						for(int64 j = 0; j < local_count; j++) {
							tbl.clearDataBuf();
							THROW_SL(Ctx.Unserialize(tbl.GetTableName(), &tbl.GetFields(), tbl.getDataBuf(), buffer));
							if(has_lob) {
								const SLob * p_lob = static_cast<const SLob *>(lob_fld.getValuePtr());
								tbl.setLobSize(lob_fld, p_lob ? p_lob->GetPtrSize() : 0);
							}
							THROW_DB(tbl.insertRec());
							if(has_lob) {
								//
								// ��� ��� ������������ LOB-���� ��������������� ��� � ����������������
								// ����, �� ������� ���������������� ����� �� ��������� ������ ������.
								//
								tbl.writeLobData(lob_fld, 0, 0, 0);
							}
							PPWaitPercent(cntr.Increment(), tbl_name);
						}
					}
					THROW(tra.Commit());
				}
				THROW(p_dict->PostProcessAfterUndump(&tbl));
			}
		}
	}
	else if(P.SpcOb == spcobQuot) {
		THROW(TblEntryList.getCount() == 1);
		THROW(DS.GetConstTLA().IsAuth());
		const TableEntry & r_entry = *(TableEntry *)TblEntryList.at(pos);
		THROW_SL(FDump.Seek64(r_entry.Offs));
		{
			Quotation2Core qc2;
			for(int64 i = 0; i < r_entry.NumChunks; i++) {
				int64  local_count = 0;
				buffer.Z();
				THROW_SL(FDump.Read(&local_count, sizeof(local_count)));
				THROW_SL(FDump.Read(buffer));
				THROW(qc2.UndumpCurrent(buffer, 1));
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PrcssrDbDump::Helper_Dump(long tblID)
{
	int    ok = -1;
	int    ref_allocated = 0;
	DbTableStat ts;
	SBuffer buffer;
	int64  recs_count = 0;  // ����� ���������� �������
	const  int64 start_offs = FDump.Tell();
	THROW(P.Mode == 1);
	if(P.SpcOb == spcobNone) {
		if(CurDict->GetTableInfo(tblID, &ts) && !(ts.Flags & XTF_DICT)) {
			SBuffer lob_buf;
			char   key[MAXKEYLEN];
			int64  local_count = 0; // ���������� ������� � �������
			int64  chunk_count = 0; // ���������� ��������
			int    has_lob = 0;
			IterCounter cntr;
			DBField lob_fld;
			DBTable tbl(ts.TblName);
			THROW(tbl.allocOwnBuffer(16*1024));
			if(tbl.HasLob(&lob_fld) > 0) {
				has_lob = 1;
			}
			PPInitIterCounter(cntr, &tbl);
			MEMSZERO(key);
			tbl.clearDataBuf();
			if(tbl.search(0, &key, spFirst)) do {
				if(has_lob) {
					//
					// ������������ ����� LOB-���� ��� ����, ����� ������� Serialize �����
					// ��������� ��������� ��� � ������.
					//
					lob_buf.Z();
					tbl.readLobData(lob_fld, lob_buf);
					tbl.writeLobData(lob_fld, lob_buf.constptr(), lob_buf.GetAvailableSize(), 1);
				}
				THROW_SL(Ctx.Serialize(tbl.GetTableName(), &tbl.GetFieldsNonConst(), tbl.getDataBuf(), buffer));
				if(has_lob) {
					//
					// ������� ���������������� ����� LOB-���� �� ��������� ������ ������.
					//
					tbl.writeLobData(lob_fld, 0, 0, 0);
				}
				local_count++;
				recs_count++;
				if(buffer.GetAvailableSize() >= MaxBufLen) {
					THROW_SL(FDump.Write(&local_count, sizeof(local_count)));
					THROW_SL(FDump.Write(buffer));
					buffer.Z();
					local_count = 0;
					chunk_count++;
				}
				tbl.clearDataBuf();
				PPWaitPercent(cntr.Increment(), tbl.GetTableName());
			} while(tbl.search(0, &key, spNext));
			THROW_DB(BTROKORNFOUND);
			if(local_count) {
				THROW_SL(FDump.Write(&local_count, sizeof(local_count)));
				THROW_SL(FDump.Write(buffer));
				buffer.Z();
				local_count = 0;
				chunk_count++;
			}
			if(recs_count) {
				TableEntry entry;
				MEMSZERO(entry);
				entry.Id = tblID;
				entry.Offs = start_offs;
				entry.NumRecs = recs_count;
				entry.NumChunks = chunk_count;
				THROW_SL(TblEntryList.insert(&entry));
				THROW_SL(TblNameList.Add(tblID, ts.TblName));
				ok = 1;
			}
		}
	}
	else if(P.SpcOb == spcobQuot) {
		Quotation2Core qc2;
		if(!PPRef) {
			THROW_MEM(PPRef = new Reference);
			ref_allocated = 1;
		}
		THROW(qc2.DumpCurrent(buffer, &recs_count));
		THROW_SL(FDump.Write(&recs_count, sizeof(recs_count)));
		THROW_SL(FDump.Write(buffer));
		{
			TableEntry entry;
			MEMSZERO(entry);
			entry.Id = 0;
			entry.Offs = start_offs;
			entry.NumRecs = recs_count;
			entry.NumChunks = 1;
			THROW_SL(TblEntryList.insert(&entry));
			THROW_SL(TblNameList.Add(tblID, ts.TblName));
		}
	}
	CATCHZOK
	if(ref_allocated) {
		ZDELETE(PPRef);
	}
	return ok;
}

int SLAPI DoDbDump(PPDbEntrySet2 * pDbes)
{
	int    ok = -1;
	PrcssrDbDump::Param p;
	PrcssrDbDump proc(pDbes);
	proc.InitParam(&p);
	while(proc.EditParam(&p) > 0) {
		if(!proc.Init(&p))
			PPError();
		if(proc.Run()) {
			ok = 1;
			break;
		}
		else
			PPError();
	}
	return ok;
}
//
//
//
class DBMaintenanceDialog : public TDialog {
public:
	DBMaintenanceDialog(PPDbEntrySet2 * _dbes) : TDialog(DLG_BU_PRELUDE), dbes(_dbes)
	{
		if(dbes) {
			if(dbes->GetCount() == 1)
				dbes->SetSelection(1);
			SetupDBEntryComboBox(this, CTLSEL_BU_PRELUDE_DB, dbes);
			int    enable_cmds = BIN(dbes->GetSelection());
			enableCommand(cmBuBackup, enable_cmds);
			enableCommand(cmBuRestore, enable_cmds);
			enableCommand(cmRecover, enable_cmds);
			enableCommand(cmProtect, enable_cmds);
			enableCommand(cmDump,    enable_cmds);
		}
		else
			disableCtrl(CTLSEL_BU_PRELUDE_DB, 1);
	}
	int    getDTS(long * pDbID)
	{
		getCtrlData(CTLSEL_BU_PRELUDE_DB, pDbID);
		if(*pDbID == 0)
			*pDbID = dbes->GetSelection();
		return 1;
	}
private:
	DECL_HANDLE_EVENT;
	PPDbEntrySet2 * dbes;
};

IMPL_HANDLE_EVENT(DBMaintenanceDialog)
{
	long   dbid;
	TDialog::handleEvent(event);
	if(TVCOMMAND) {
		if(event.isCbSelected(CTLSEL_BU_PRELUDE_DB)) {
			dbid = getCtrlLong(CTLSEL_BU_PRELUDE_DB);
			int    enable_cmds = 0;
			if(!dbid && !dbes->GetSelection()) {
				enable_cmds = 0;
			}
			else {
				if(dbid)
					dbes->SetSelection(dbid);
				enable_cmds = 1;
			}
			enableCommand(cmBuBackup, enable_cmds);
			enableCommand(cmBuRestore, enable_cmds);
			enableCommand(cmRecover, enable_cmds);
			enableCommand(cmProtect, enable_cmds);
			enableCommand(cmDump, enable_cmds);
			clearEvent(event);
		}
		else if(oneof4(TVCMD, cmBuBackup, cmBuRestore, cmBuRemove, cmBuRemoveCr)) {
			dbid = getCtrlLong(CTLSEL_BU_PRELUDE_DB);
			if(dbid == 0 && !dbes->GetSelection()) {
				PPError(PPERR_RSTRDBIDNEEDED, 0);
				clearEvent(event);
				return;
			}
			else if(dbid)
				dbes->SetSelection(dbid);
			if(IsInState(sfModal)) {
				const int cmd_ = TVCMD;
				clearEvent(event);
				endModal(cmd_);
				return; // ����� endModal �� ������� ���������� � this
			}
		}
		else if(oneof7(TVCMD, cmBuAutoBackup, cmRecover, cmProtect, cmDump, cmCreateDB, cmSetupBackupCfg, cmDbMonitor))
			if(IsInState(sfModal)) {
				const int cmd_ = TVCMD;
				clearEvent(event);
				endModal(cmd_);
				return; // ����� endModal �� ������� ���������� � this
			}
	}
}
//
//
//
class ChangeDBListDialog : public PPListDialog {
public:
	ChangeDBListDialog() : PPListDialog(DLG_DBLIST, CTL_DBLIST_DB), F(0, 0, 0, 1), Modified(0)
	{
		Dbes.ReadFromProfile(&F, 0, 1);
		updateList(-1);
	}
private:
	DECL_HANDLE_EVENT
	{
		if(event.isCmd(cmOK)) {
			if(Modified) {
				F.FlashIniBuf();
			}
			endModal(cmOK);
			return; // ����� endModal �� ������� ���������� � this
		}
		else
			PPListDialog::handleEvent(event);
	}
	virtual int setupList();
	virtual int addItem(long * pPos, long * pID);
	virtual int editItem(long pos, long id);
	virtual int delItem(long pos, long id);
	int    DoEditDB(PPID dbid);

	PPDbEntrySet2 Dbes;
	PPIniFile F;
	int    Modified;
};

int ChangeDBListDialog::DoEditDB(PPID dbid)
{
	class EditDbEntryDialog : public TDialog {
	public:
		EditDbEntryDialog() : TDialog(DLG_CREATEDB)
		{
		}
		void SetupServerType(int serverType)
		{
			if(serverType >= 0) {
				setCtrlUInt16(CTL_CREATEDB_SRVTYPE, (serverType == sqlstORA) ? 1 : 0);
			}
			else {
				serverType = (getCtrlUInt16(CTL_CREATEDB_SRVTYPE) == 1) ? sqlstORA : sqlstNone;
			}
			disableCtrls(serverType != sqlstORA, CTL_CREATEDB_DBNAME, CTL_CREATEDB_DBUSER, CTL_CREATEDB_DBUSERPW, 0L);
			disableCtrls(serverType == sqlstORA, CTL_CREATEDB_DICT, 0L);
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(event.isClusterClk(CTL_CREATEDB_SRVTYPE)) {
				SetupServerType(-1);
				clearEvent(event);
			}
		}
	};
	int   ok = -1;
	EditDbEntryDialog * dlg = new EditDbEntryDialog;
	SString temp_buf;
	int    server_type = sqlstNone;
	THROW(CheckDialogPtrErr(&dlg));
	Dbes.GetAttr(dbid, DbLoginBlock::attrServerType, temp_buf);
	if(temp_buf.CmpNC("ORACLE") == 0)
		server_type = sqlstORA;
	dlg->SetupServerType(server_type);
	if(dbid) {
		Dbes.GetAttr(dbid, DbLoginBlock::attrDbSymb, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_ENTRYNAME, temp_buf);
		Dbes.GetAttr(dbid, DbLoginBlock::attrDbFriendlyName, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_NAME, temp_buf);
		Dbes.GetAttr(dbid, DbLoginBlock::attrDictPath, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_DICT, temp_buf);
		Dbes.GetAttr(dbid, DbLoginBlock::attrDbPath, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_DATA, temp_buf);
		//
		Dbes.GetAttr(dbid, DbLoginBlock::attrDbName, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_DBNAME, temp_buf);
		Dbes.GetAttr(dbid, DbLoginBlock::attrUserName, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_DBUSER, temp_buf);
		Dbes.GetAttr(dbid, DbLoginBlock::attrPassword, temp_buf);
		dlg->setCtrlString(CTL_CREATEDB_DBUSERPW, temp_buf);
	}
	FileBrowseCtrlGroup::Setup(dlg, CTLBRW_CREATEDB_DICT, CTL_CREATEDB_DICT, 1, PPTXT_TITLE_SELSYSPATH, 0, FileBrowseCtrlGroup::fbcgfPath);
	FileBrowseCtrlGroup::Setup(dlg, CTLBRW_CREATEDB_DATA, CTL_CREATEDB_DATA, 2, PPTXT_TITLE_SELDATPATH, 0, FileBrowseCtrlGroup::fbcgfPath);
	for(int valid_data = 0; !valid_data && ExecView(dlg) == cmOK;) {
		//char   admin_name[64], admin_pwd[64];
		//ushort use_dti = 0;
		DbLoginBlock dlb;
		server_type = (dlg->getCtrlUInt16(CTL_CREATEDB_SRVTYPE) == 1) ? sqlstORA : sqlstNone;
		if(server_type == sqlstORA)
			dlb.SetAttr(DbLoginBlock::attrServerType, "ORACLE");
		dlg->getCtrlString(CTL_CREATEDB_ENTRYNAME, temp_buf.Z());
		if(!temp_buf.NotEmptyS())
			PPErrorByDialog(dlg, CTL_CREATEDB_ENTRYNAME, PPERR_NAMENEEDED);
		else {
			dlb.SetAttr(DbLoginBlock::attrDbSymb, temp_buf);
			dlg->getCtrlString(CTL_CREATEDB_NAME, temp_buf.Z());
			dlb.SetAttr(DbLoginBlock::attrDbFriendlyName, temp_buf);
			dlg->getCtrlString(CTL_CREATEDB_DICT, temp_buf.Z());
			dlb.SetAttr(DbLoginBlock::attrDictPath, temp_buf);
			dlg->getCtrlString(CTL_CREATEDB_DATA, temp_buf.Z());
			dlb.SetAttr(DbLoginBlock::attrDbPath, temp_buf);
			if(server_type == sqlstORA) {
				dlg->getCtrlString(CTL_CREATEDB_DBNAME, temp_buf.Z());
				dlb.SetAttr(DbLoginBlock::attrDbName, temp_buf);
				dlg->getCtrlString(CTL_CREATEDB_DBUSER, temp_buf.Z());
				dlb.SetAttr(DbLoginBlock::attrUserName, temp_buf);
				dlg->getCtrlString(CTL_CREATEDB_DBUSERPW, temp_buf.Z());
				dlb.SetAttr(DbLoginBlock::attrPassword, temp_buf);
				temp_buf.Z();
			}
			if(Dbes.RegisterEntry(&F, &dlb) && Dbes.Add(0, &dlb, 1)) {
				ok = valid_data = 1;
				Modified = 1;
			}
			else
				PPError(PPERR_SLIB);
		}
	}
	CATCHZOK
	delete dlg;
	return ok;
}

int ChangeDBListDialog::setupList()
{
	SString entry_name, name, dict_path, data_path;
	DbLoginBlock dlb;
	for(long i = 1; i <= (long)Dbes.GetCount(); i++) {
		if(Dbes.GetByID(i, &dlb)) {
			dlb.GetAttr(DbLoginBlock::attrDbSymb, entry_name);
			dlb.GetAttr(DbLoginBlock::attrDbFriendlyName, name);
			dlb.GetAttr(DbLoginBlock::attrDictPath, dict_path);
			dlb.GetAttr(DbLoginBlock::attrDbPath, data_path);
			StringSet ss(SLBColumnDelim);
			ss.add(name);
			ss.add(entry_name);
			ss.add(data_path);
			ss.add(dict_path);
			if(!addStringToList(i, ss.getBuf()))
				return 0;
		}
	}
	return 1;
}

int ChangeDBListDialog::addItem(long *, long *pID)
{
	int    ok = DoEditDB(0);
	if(ok > 0) {
		Dbes.Clear();
		Dbes.ReadFromProfile(&F, 0, 1);
		*pID = Dbes.GetCount() - 1;
	}
	return ok;
}

int ChangeDBListDialog::editItem(long, long id)
{
	int    ok = DoEditDB(id);
	if(ok > 0) {
		Dbes.Clear();
		Dbes.ReadFromProfile(&F, 0, 1);
	}
	return ok;
}

int ChangeDBListDialog::delItem(long, long id)
{
	int    ok = -1;
	if(id && CONFIRMCRIT(PPCFM_DELETE)) {
		SString entry_name;
		if(Dbes.GetAttr(id, DbLoginBlock::attrDbSymb, entry_name)) {
			const  char * p_sect = "DBNAME";
			if(F.RemoveParam(p_sect, entry_name)) {
				Dbes.Clear();
				Dbes.ReadFromProfile(&F, 0, 1);
				Modified = 1;
				ok = 1;
			}
			else
				ok = (PPError(PPERR_SLIB), 0);
		}
	}
	return ok;
}

int ChangeDBList()
{
	ChangeDBListDialog * dlg = new ChangeDBListDialog();
	if(CheckDialogPtr(&dlg))
		ExecViewAndDestroy(dlg);
	return 1;
}
//
//
//
struct BackupDlgData {
	PPID   DBID;
	PPBackupScen Scen;
	long   CopyID;
	uint   Cmd;
};

int SLAPI CheckBuCopy(PPBackup * pPB, BackupDlgData * pBDD, int showDialog = 1);

class BackupDialog : public TDialog {
public:
	BackupDialog(uint rezID, BackupDlgData * data, PPBackup * ppb) : TDialog(rezID), CtrlX(0), Data(*data), AsBackup(BIN(data->Cmd == cmBuBackup)), PPB(ppb)
	{
		enableCommand(cmBuBackup,  1);
		enableCommand(cmBuRestore, 1);
		enableCommand(cmBuRemove,  1);
		enableCommand(cmBuCheck,   1);
		Data.Cmd = 0;
		P_List   = static_cast<SmartListBox *>(getCtrlView(CTL_BU_BACKUP_LIST));
		SetupStrListBox(this, CTL_BU_BACKUP_LIST);
		setupScenCombo();
		setupCopyList();
	}
	int    updateCopyList()
	{
		setupCopyList();
		return 1;
	}
	int    getDTS(BackupDlgData *);
private:
	DECL_HANDLE_EVENT;
	void   setupCopyList();
	void   setupScenCombo();
	int    getScenData();
	int    AsBackup; // if 0 then restore dialog
	SmartListBox * P_List;
	BackupDlgData  Data;
	PPBackup * PPB;
	int    CtrlX;
};

int BackupDialog::getDTS(BackupDlgData * data)
{
	getScenData();
	Data.CopyID = 0;
	CALLPTRMEMB(P_List, getCurID(&Data.CopyID));
	data->DBID   = Data.DBID;
	data->Scen   = Data.Scen;
	data->CopyID = Data.CopyID;
	data->Cmd    = Data.Cmd;
	return 1;
}

IMPL_HANDLE_EVENT(BackupDialog)
{
	if(TVKEYDOWN && TVKEY != kbCtrlX)
		CtrlX = 0;
	TDialog::handleEvent(event);
	if(TVCOMMAND) {
		if(event.isCbSelected(CTLSEL_BU_BACKUP_SCEN)) {
			setupCopyList();
			clearEvent(event);
			CtrlX = 0;
		}
		else if(TVCMD == cmLBItemFocused) {
			if(event.isCtlEvent(CTL_BU_BACKUP_LIST)) {
				if(P_List) {
					long   copy_id = 0;
					P_List->getCurID(&copy_id);
					enableCommand(cmBuRestore, BIN(copy_id));
					enableCommand(cmBuRemove,  BIN(copy_id));
					enableCommand(cmBuCheck,   BIN(copy_id));
				}
			}
		}
		else if(oneof4(TVCMD, cmBuBackup, cmBuRestore, cmBuRemove, cmBuRemoveCr)) {
			if(IsInState(sfModal)) {
				CtrlX = 0;
				Data.Cmd = TVCMD;
				clearEvent(event);
				endModal(Data.Cmd);
				return; // ����� endModal �� ������� ���������� � this
			}
		}
		else if(TVCMD == cmBuCheck) {
			CALLPTRMEMB(P_List, getCurID(&Data.CopyID));
			if(CheckBuCopy(PPB, &Data) <= 0)
				PPError();
			Data.CopyID = 0;
			clearEvent(event);
		}
	}
	else if(TVKEYDOWN) {
		if(TVKEY == kbCtrlX) {
			if(CtrlX) {
				CtrlX = 0;
				if(IsInState(sfModal)) {
					Data.Cmd = cmBuReleaseContinuous;
					endModal(Data.Cmd);
				}
			}
			else
				CtrlX = 1;
			clearEvent(event);
		}
	}
}

void BackupDialog::setupScenCombo()
{
	ComboBox * p_cb = static_cast<ComboBox *>(getCtrlView(CTLSEL_BU_BACKUP_SCEN));
	if(p_cb) {
		PPBackupScen scen;
		ListWindow * p_lw = CreateListWindow(40, lbtDisposeData);
		long   i = 0;
		while(PPB->EnumScen(&i, &scen) > 0)
			p_lw->listBox()->addItem(i, scen.Name);
		p_cb->setListWindow(p_lw, (i != 1) ? 0 : i);
	}
}

int BackupDialog::getScenData()
{
	long   scen_id = 0;
	getCtrlData(CTLSEL_BU_BACKUP_SCEN, &scen_id);
	if(scen_id)
		PPB->GetScen(scen_id, &Data.Scen);
	else
		MEMSZERO(Data.Scen);
	return 1;
}

void BackupDialog::setupCopyList()
{
	SString text;
	BCopyData * bcdata = 0;
	getScenData();
	SmartListBox * p_list = static_cast<SmartListBox *>(getCtrlView(CTL_BU_BACKUP_LIST));
	if(p_list) {
		p_list->freeAll();
		BCopySet bcset(Data.Scen.Name);
		PPB->GetCopySet(&bcset);
		bcset.Sort(BCopySet::ordByDateDesc);
		if(AsBackup) {
			text = "<New copy>";
			p_list->addItem(0, text);
		}
		for(uint i = 0; bcset.enumItems(&i, (void **)&bcdata);) {
			text = 0;
			text.Cat(bcdata->Dtm, MKSFMT(12, ALIGN_LEFT | DATF_DMY), MKSFMT(12, ALIGN_LEFT | TIMF_HMS));
			text.Cat(bcdata->Set);
			if(!p_list->addItem(bcdata->ID, text)) {
				PPError(PPERR_SLIB, 0);
				break;
			}
		}
		p_list->focusItem(0);
	}
}
//
//
//
static int SLAPI DBMaintenanceFunctions(PPDbEntrySet2 * dbes)
{
	DBMaintenanceDialog * dlg = new DBMaintenanceDialog(dbes);
	PPID   db_id = 0;
	int    reply = cmCancel;
	if(CheckDialogPtr(&dlg)) {
		reply = ExecView(dlg);
		dlg->getCtrlData(CTLSEL_BU_PRELUDE_DB, &db_id);
		if(db_id != 0)
			dbes->SetSelection(db_id);
		else
			db_id = dbes->GetSelection();
	}
	else
		reply = 0;
	delete dlg;
	return reply;
}

int SLAPI PPBackup::LockDatabase()
{
	int    ok = 1, waiting = 0;
	if(!(State & stDbIsLocked)) {
		while(ok > 0 && !P_Sync->LockDB()) {
			if(DS.CheckExtFlag(ECF_SYSSERVICE))
				ok = 0;
			else if(PPErrCode != PPERR_SYNCDBLOCKED && PPErrCode != PPERR_SYNCDBINUSE)
				ok = 0;
			else if(waiting || PPMessage(mfConf|mfYes|mfNo, PPCFM_WAITONDBLOCK) == cmYes) {
				if(!waiting) {
					PPWait(1);
					PPWaitMsg(PPSTR_TEXT, PPTXT_WAITONDBLOCK, 0);
				}
				waiting = 1;
		   		for(int i = 0; ok > 0 && i < 20; i++)
					if(PPCheckUserBreak() == 0)
						ok = -1;
					else
						SDelay(500);
			}
			else
				ok = -1;
		}
		PPWait(0);
		if(ok > 0)
			State |= stDbIsLocked;
	}
	return ok;
}

int SLAPI PPBackup::UnlockDatabase()
{
	if(State & stDbIsLocked) {
		if(P_Sync->UnlockDB())
			State &= ~stDbIsLocked;
	}
	return 1;
}

int SLAPI UseCopyContinouos(PPDbEntrySet2 * pDbes)
{
	int    r = 0;
	if(pDbes) {
		PPIniFile ini_file;
		int    ini_param = 0;
		if(ini_file.GetInt(PPINISECT_BACKUP, PPINIPARAM_USECOPYCONTINUOUS, &ini_param) > 0 && ini_param) {
			PPID   dbentry_id = NZOR(pDbes->GetSelection(), pDbes->SetDefaultSelection());
			SString data_path, disk;
			pDbes->GetAttr(dbentry_id, DbLoginBlock::attrDbPath, data_path);
			SPathStruc ps(data_path);
			if(ps.Flags & SPathStruc::fUNC) {
				(disk = ps.Drv).RmvLastSlash();
				SString lcn; // local machine name
				if(SGetComputerName(lcn) && disk.CmpNC(lcn) == 0)
					r = 1;
			}
			else {
				(disk = ps.Drv).CatChar(':').CatChar('\\');
				r = BIN(dbentry_id && GetDriveType(SUcSwitch(disk)) == DRIVE_FIXED); // @unicodeproblem
			}
		}
	}
	return r;
}

#define CTLGRP_FBRW 1

class BackupParamDialog : public TDialog {
public:
	BackupParamDialog() : TDialog(DLG_BUPARAM)
	{
		FileBrowseCtrlGroup::Setup(this, CTLBRW_BUPARAM_PATH, CTL_BUPARAM_PATH, CTLGRP_FBRW, PPTXT_TITLE_SELBACKUPPATH, 0, FileBrowseCtrlGroup::fbcgfPath);
	}
	int setDTS(const PPBackupScen * pScen);
	int getDTS(SString & rDBSymb, PPBackupScen * pScen);
private:
	DECL_HANDLE_EVENT;

	PPBackupScen Scen;
};

IMPL_HANDLE_EVENT(BackupParamDialog)
{
	TDialog::handleEvent(event);
	if(event.isKeyDown(kbF2)) {
		SString path;
		DBS.GetDbPath(DBS.GetDbPathID(), path);
		setCtrlString(CTL_BUPARAM_PATH, path.SetLastSlash().Cat("backup"));
		clearEvent(event);
	}
}

int BackupParamDialog::setDTS(const PPBackupScen * pScen)
{
	if(!RVALUEPTR(Scen, pScen))
		MEMSZERO(Scen);
	Scen.numCopies = (Scen.numCopies <= 0) ? 1 : Scen.numCopies;
	setCtrlData(CTL_BUPARAM_PATH, Scen.BackupPath);
	setCtrlData(CTL_BUPARAM_MAXCOPIES, &Scen.numCopies);
	AddClusterAssoc(CTL_BUPARAM_FLAGS, 0x01, BCOPYDF_USECOMPRESS);
	SetClusterData(CTL_BUPARAM_FLAGS, Scen.Flags);
	return 1;
}

int BackupParamDialog::getDTS(SString & rDBSymb, PPBackupScen * pScen)
{
	int    ok = 1;
	DbProvider * p_dict = CurDict;
	SString dbname;
	PPIniFile ini_file;
	PPDbEntrySet2 dbes;
	dbes.ReadFromProfile(&ini_file);
	getCtrlData(CTL_BUPARAM_PATH, Scen.BackupPath);
	getCtrlData(CTL_BUPARAM_MAXCOPIES, &Scen.numCopies);
	GetClusterData(CTL_BUPARAM_FLAGS, &Scen.Flags);
	PPSetAddedMsgString(setLastSlash(Scen.BackupPath));
	THROW_PP(pathValid(Scen.BackupPath, 0), PPERR_NEXISTPATH);
	Scen.numCopies = (Scen.numCopies <= 0) ? 1 : Scen.numCopies;
	p_dict->GetDbSymb(rDBSymb);
	Scen.ID = dbes.GetBySymb(rDBSymb, 0);
	rDBSymb.CopyTo(Scen.Name, sizeof(Scen.Name));
	p_dict->GetDbName(dbname);
	dbname.CopyTo(Scen.DBName, sizeof(Scen.DBName));
	ASSIGN_PTR(pScen, Scen);
	CATCHZOKPPERR
	return ok;
}

int SLAPI EditBackupParam(SString & rDBSymb, PPBackupScen * pScen)
{
	int    ok = -1;
	BackupParamDialog * p_dlg = new BackupParamDialog();
	THROW_MEM(p_dlg);
	THROW(CheckDialogPtr(&p_dlg));
	p_dlg->setDTS(pScen);
	while(ok < 0 && ExecView(p_dlg) == cmOK)
		if(p_dlg->getDTS(rDBSymb, pScen))
			ok = 1;
	CATCHZOK
	delete p_dlg;
	return ok;
}

int SLAPI DoServerBackup(SString & rDBSymb, PPBackupScen * pScen)
{
	int    ok = -1, is_locked = 0;
	int    use_copy_continouos = 0;
	uint   count = 0, i = 0;
	int    bss_factor = 0;
	PPID   db_id = 0;
	PPDriveMapping drv_map;
	PPIniFile ini_file;
	PPDbEntrySet2 dbes;
	DbLoginBlock dlb;
	SString data_path;
	PPBackup * p_bu = 0;
	BCopyData copy_data;
	THROW(ini_file.IsValid());
	THROW(dbes.ReadFromProfile(&ini_file, 0));
	drv_map.Load(&ini_file);
	THROW_SL(db_id = dbes.GetBySymb(rDBSymb, &dlb));
	dlb.GetAttr(DbLoginBlock::attrDbPath, data_path);
	THROW_PP_S(::access(data_path, 0) == 0, PPERR_DBDIRNFOUND, data_path);
	dbes.SetSelection(db_id);
	use_copy_continouos = UseCopyContinouos(&dbes);
	THROW(p_bu = PPBackup::CreateInstance(&dbes));
	if(use_copy_continouos) {
		SString fmt_buf, msg_buf;
		msg_buf.Printf(PPLoadTextS(PPTXT_BACKUPLOG_CONINOUOS_MODE, fmt_buf), rDBSymb.cptr());
		PPLogMessage(PPFILNAM_BACKUP_LOG, msg_buf, LOGMSGF_TIME);
	}
	else
		THROW(p_bu->LockDatabase());
	{
		BCopySet bcset(pScen->Name);
		is_locked = use_copy_continouos ? 0 : 1;
		ini_file.GetInt(PPINISECT_SYSTEM, PPINIPARAM_BSSFACTOR, &bss_factor);
		copy_data.BssFactor = bss_factor;
		ini_file.Get(PPINISECT_SYSTEM, PPINIPARAM_BACKUPTEMP, copy_data.TempPath);
		if(!copy_data.TempPath.NotEmptyS()) {
			char * p_path = getenv("TMP");
			if(SETIFZ(p_path, getenv("TEMP")))
				copy_data.TempPath = p_path;
		}
		else
			drv_map.ConvertPathToUnc(copy_data.TempPath);
		copy_data.Set = pScen->Name;
		copy_data.CopyPath = pScen->BackupPath;
		drv_map.ConvertPathToUnc(copy_data.CopyPath);
		copy_data.Flags = pScen->Flags;
		SETFLAG(copy_data.Flags, BCOPYDF_USECOPYCONT, use_copy_continouos);
		THROW_PP(p_bu->Backup(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
		p_bu->GetCopySet(&bcset);
		bcset.Sort(BCopySet::ordByDate);
		count = bcset.getCount();
		{
			BCopyData * p_bcd = 0;
			for(i = 0; count > (uint)pScen->numCopies; count--)
				if(bcset.enumItems(&i, (void **)&p_bcd) > 0)
					THROW_PP(p_bu->RemoveCopy(p_bcd, CallbackBuLog, 0), PPERR_DBLIB);
			ok = 1;
		}
	}
	CATCH
		CALLPTRMEMB(p_bu, RemoveCopy(&copy_data, CallbackBuLog, 0));
		CallbackBuLog(BACKUPLOG_ERROR, 0, 0);
		ok = 0;
	ENDCATCH
	if(is_locked)
		p_bu->UnlockDatabase();
	delete p_bu;
	return ok;
}

static int SLAPI _DoAutoBackup(PPBackup * pBu, PPBackupScen * pScen, int useCopyContinouos)
{
	int    ok = 1;
	uint   i, j;
	BCopySet bcset(pScen->Name);
	int    do_backup = 0;
	BCopyData copy_data;
	if(pBu->GetLastScenCopy(pScen, &copy_data) > 0) {
		if(pScen->Period > 0 && diffdate(getcurdate_(), copy_data.Dtm.d) >= pScen->Period)
			do_backup = 1;
	}
	else
		do_backup = 1;
	if(do_backup) {
		PPDriveMapping drv_map;
		PPIniFile ini_file;
		int    bss_factor = 0;
		drv_map.Load(&ini_file);
		ini_file.GetInt(PPINISECT_SYSTEM, PPINIPARAM_BSSFACTOR, &bss_factor);
		copy_data.BssFactor = (long)bss_factor;
		ini_file.Get(PPINISECT_SYSTEM, PPINIPARAM_BACKUPTEMP, copy_data.TempPath);
		if(!copy_data.TempPath.NotEmptyS()) {
			char * p_path = getenv("TMP");
			if(SETIFZ(p_path, getenv("TEMP")))
				copy_data.TempPath = p_path;
		}
		else
			drv_map.ConvertPathToUnc(copy_data.TempPath);
		copy_data.Set = pScen->Name;
		copy_data.CopyPath = pScen->BackupPath;
		drv_map.ConvertPathToUnc(copy_data.CopyPath);
		copy_data.Flags = pScen->Flags;
		PPWait(1);
		SETFLAG(copy_data.Flags, BCOPYDF_USECOPYCONT, useCopyContinouos);
		THROW_PP(pBu->Backup(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
	}
	pBu->GetCopySet(&bcset);
	bcset.Sort(BCopySet::ordByDate);
	i = bcset.getCount();
	{
		BCopyData * p_bcd = 0;
		for(j = 0; i > (uint)pScen->numCopies; i--)
			if(bcset.enumItems(&j, (void **)&p_bcd) > 0)
				THROW_PP(pBu->RemoveCopy(p_bcd, CallbackBuLog, 0), PPERR_DBLIB);
	}
	CATCH
		pBu->RemoveCopy(&copy_data, CallbackBuLog, 0);
		CallbackBuLog(BACKUPLOG_ERROR, 0, 0);
		ok = 0;
	ENDCATCH
	PPWait(0);
	return ok;
}

static int SLAPI _DoAutoBackup(PPDbEntrySet2 * pDbes, PPBackup * pBu, int noDefault, int useCopyContinouos)
{
	int    ok = 1;
	if(pBu == 0) {
		for(long i = 1; i <= (long)pDbes->GetCount(); i++) {
			int use_copy_continouos = 0;
			pDbes->SetSelection(i);
			use_copy_continouos = UseCopyContinouos(pDbes);
			if((pBu = PPBackup::CreateInstance(pDbes)) != 0) {
				_DoAutoBackup(pDbes, pBu, 1, use_copy_continouos); // @recursion
				ZDELETE(pBu);
				ok = 1;
			}
			else
				ok = 0;
		}
	}
	else {
		PPBackupScen scen;
		for(long scen_id = 0; pBu->EnumScen(&scen_id, &scen) > 0;) {
			if(!noDefault || strcmp(scen.Name, DefaultScenName) != 0) {
				ok = useCopyContinouos ? 1 : pBu->LockDatabase();
				if(ok > 0) {
					if(!_DoAutoBackup(pBu, &scen, useCopyContinouos))
						ok = PPErrorZ();
					if(!useCopyContinouos)
						pBu->UnlockDatabase();
				}
			}
		}
	}
	return ok;
}

static int SLAPI _DoBackup(PPBackup * ppb, BackupDlgData & bdd, int useCopyContinouos)
{
	int    ok = -1;
	BCopyData copy_data;
	if(!bdd.CopyID || PPMessage(mfConf|mfYesNo, PPCFM_REWRITEBCOPY) == cmYes) {
		if(!useCopyContinouos) {
			THROW(ok = ppb->LockDatabase());
		}
		else
			ok = 1;
		if(ok > 0) {
			if(bdd.CopyID) {
				THROW_PP(ppb->GetCopyData(bdd.CopyID, &copy_data), PPERR_DBLIB);
				THROW_PP(ppb->RemoveCopy(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
			}
			copy_data.Set = bdd.Scen.Name;
			copy_data.CopyPath = bdd.Scen.BackupPath;
			copy_data.Flags = bdd.Scen.Flags;
			PPWait(1);
			SETFLAG(copy_data.Flags, BCOPYDF_USECOPYCONT, useCopyContinouos);
			THROW_PP(ppb->Backup(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
			ok = 1;
		}
	}
	CATCH
		ppb->RemoveCopy(&copy_data, CallbackBuLog, 0);
		CallbackBuLog(BACKUPLOG_ERROR, 0, 0);
		ok = 0;
	ENDCATCH
	if(!useCopyContinouos)
		ppb->UnlockDatabase();
	PPWait(0);
	return ok;
}

static int SLAPI _DoRemoveCopy(PPBackup * ppb, const BackupDlgData & bdd)
{
	int    ok = -1;
	if(bdd.CopyID && PPMessage(mfConf|mfYesNo, PPCFM_BREMOVE) == cmYes) {
		BCopyData copy_data;
		THROW_PP(ppb->GetCopyData(bdd.CopyID, &copy_data), PPERR_DBLIB);
		THROW_PP(ppb->RemoveCopy(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
		ok = 1;
	}
	CATCHZOK
	return ok;
}

static int SLAPI _DoRestore(PPBackup * ppb, const BackupDlgData & bdd)
{
	int    ok = -1;
	if(bdd.CopyID && PPMessage(mfConf|mfYesNo, PPCFM_BRESTORE) == cmYes) {
		THROW(ok = ppb->LockDatabase());
		if(ok > 0) {
			BCopyData copy_data;
			THROW_PP(ppb->GetCopyData(bdd.CopyID, &copy_data), PPERR_DBLIB);
			PPWait(1);
			THROW_PP(ppb->Restore(&copy_data, CallbackBuLog, 0), PPERR_DBLIB);
			PPWait(0);
			ok = 1;
		}
	}
	CATCHZOK
	ppb->UnlockDatabase();
	PPWait(0);
	return ok;
}

static int SLAPI PPRecoverDialog(PPDbEntrySet2 * pDbes, BTBLID * pTblID, SString & rDestPath, SString & rLogFileName)
{
	class RecoverDialog : public TDialog {
	public:
		RecoverDialog::RecoverDialog() : TDialog(DLG_RECOVER), _dbes(0)
		{
			FileBrowseCtrlGroup::Setup(this, CTLBRW_RECOVER_DEST, CTL_RECOVER_DEST, 1, 0, 0, FileBrowseCtrlGroup::fbcgfPath);
			FileBrowseCtrlGroup::Setup(this, CTLBRW_RECOVER_LOG,  CTL_RECOVER_LOG,  2, 0, 0, FileBrowseCtrlGroup::fbcgfLogFile);
		}
		int    setDTS(PPDbEntrySet2 * dbes, BTBLID tblID, const SString & rDest, const SString & rLog)
		{
			_dbes = dbes;
			SetupDBTableComboBox(this, CTLSEL_RECOVER_TBL, dbes, dbes->GetSelection(), tblID);
			SetupDBEntryComboBox(this, CTLSEL_RECOVER_DB, dbes);
			setCtrlString(CTL_RECOVER_DEST, rDest);
			setCtrlString(CTL_RECOVER_LOG,  rLog);
			return 1;
		}

		int    getDTS(PPDbEntrySet2 * dbes, BTBLID * tblID, SString & rDest, SString & rLog)
		{
			dbes->SetSelection(getCtrlLong(CTLSEL_RECOVER_DB));
			*tblID = static_cast<BTBLID>(getCtrlLong(CTLSEL_RECOVER_TBL));
			getCtrlString(CTL_RECOVER_DEST, rDest);
			rDest.Strip();
			getCtrlString(CTL_RECOVER_LOG,  rLog);
			rLog.Strip();
			return 1;
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(_dbes && event.isCbSelected(CTLSEL_RECOVER_DB)) {
				SetupDBTableComboBox(this, CTLSEL_RECOVER_TBL, _dbes, getCtrlLong(CTLSEL_RECOVER_DB), 0L);
				clearEvent(event);
			}
		}
		PPDbEntrySet2 * _dbes;
	};
	int    ok = -1;
	RecoverDialog * dlg = new RecoverDialog;
	if(CheckDialogPtr(&dlg)) {
		dlg->setDTS(pDbes, *pTblID, rDestPath, rLogFileName);
		if(pTblID)
			dlg->disableCtrl(CTLSEL_RECOVER_DB, true);
		while(ok < 0 && ExecView(dlg) == cmOK)
			if(dlg->getDTS(pDbes, pTblID, rDestPath, rLogFileName))
				ok = 1;
			else
				PPError();
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

static int SLAPI _DoRecover(PPDbEntrySet2 * pDbes, PPBackup * pBP)
{
	int    ok = 1, ret;
	int    all_ok = -1; // ���� >0, �� ��� ������� �������������, ���� 0, �� ���� ������
	SArray r_info_array(sizeof(PPRecoverInfo));
	BTBLID tblID = 0;
	PPRecoverParam param;
	SString data_path, path, bak_path, temp_buf;
	PPIniFile ini_file;
	ini_file.Get(PPINISECT_RECOVER, PPINIPARAM_PATH, path);
	ini_file.Get(PPINISECT_RECOVER, PPINIPARAM_LOG,  param.LogFileName);
	PPGetFileName(PPFILNAM_RECOVER_LOG, param.LogFileName);
	if(PPRecoverDialog(pDbes, &tblID, path, param.LogFileName) > 0) {
		path.Strip().RmvLastSlash();
		DbLoginBlock dlb;
		long   dbentry = NZOR(pDbes->GetSelection(), pDbes->SetDefaultSelection());
		if(pDbes->GetByID(dbentry, &dlb) > 0) {
			ret = 1;
			dlb.GetAttr(DbLoginBlock::attrDbPath, data_path);
			if(path.Empty() || path.CmpNC(data_path) == 0)
				path = 0;
			else {
				PPSetAddedMsgString(path);
				if(IsDirectory(path))
					ret = CONFIRM(PPCFM_EXISTDIR);
				else if((ret = CONFIRM(PPCFM_MAKENEWDIR)) != 0) {
					if(!createDir(path))
						ret = PPErrorZ();
				}
			}
			if(ret) {
				THROW(DS.OpenDictionary2(&dlb, 0));
				THROW(pBP->LockDatabase() > 0);
				PPWait(1);
				param.P_DestPath = path;
				if(param.LogFileName.Empty())
					PPGetFilePath(PPPATH_LOG, PPFILNAM_BACKUP_LOG, param.LogFileName);
				else {
					PPGetPath(PPPATH_LOG, temp_buf);
					SPathStruc::ReplacePath(param.LogFileName, temp_buf, 0);
				}
				//
				// ������� ����������, � ������� ����� ������������ ������ ������ "�� �������"
				//
				for(long k = 1; k < 1000000L; k++)
					if(!::IsDirectory((bak_path = data_path).SetLastSlash().Cat("RB").CatLongZ(k, 6))) {
						THROW_SL(::createDir(bak_path));
						param.P_BakPath = bak_path;
						break;
					}
				if(tblID) {
					THROW_PP(_Recover(tblID, &param, &r_info_array), PPERR_DBLIB);
				}
				else {
					StrAssocArray tbl_list;
					CurDict->GetListOfTables(0, &tbl_list);
					for(uint j = 0; j < tbl_list.getCount(); j++) {
						THROW_PP(_Recover(static_cast<BTBLID>(tbl_list.Get(j).Id), &param, &r_info_array), PPERR_DBLIB);
					}
				}
				PPWait(0);
			}
		}
	}
	CATCHZOKPPERR
	pBP->UnlockDatabase();
	{
		SArray bad_tbls_ary(sizeof(PPRecoverInfo));
		PPRecoverInfo * p_rinfo = 0;
		for(uint i = 0; r_info_array.enumItems(&i, (void **) &p_rinfo);) {
			if(p_rinfo->OrgNumRecs != p_rinfo->ActNumRecs) {
				bad_tbls_ary.insert(p_rinfo);
				all_ok = 0;
			}
			if(all_ok < 0)
				all_ok = 1;
		}
		if(all_ok > 0)
			PPMessage(mfInfo|mfOK, PPINF_RECOVERSUCCESS);
		else if(all_ok == 0) {
			class RcvrInfoDlg : public PPListDialog {
			public:
				RcvrInfoDlg(const SArray * pData) : PPListDialog(DLG_RCVRRES, CTL_RCVRRES_LIST), P_Data(pData)
				{
					updateList(-1);
				}
			private:
				virtual int setupList()
				{
					PPRecoverInfo * p_item = 0;
					SString sub;
					for(uint i = 0; P_Data->enumItems(&i, (void **)&p_item);) {
						StringSet ss(SLBColumnDelim);
						ss.add(p_item->TableName);
						ss.add(sub.Z().Cat(p_item->ActNumRecs));
						ss.add(sub.Z().Cat(p_item->OrgNumRecs));
						ss.add(sub.Z().Cat(p_item->NotRcvrdNumRecs));
						if(!addStringToList(i, ss.getBuf()))
							return 0;
					}
					return 1;
				}
				const SArray * P_Data;
			};
			RcvrInfoDlg * dlg = new RcvrInfoDlg(&bad_tbls_ary);
			if(CheckDialogPtrErr(&dlg))
				ExecViewAndDestroy(dlg);
			else
				ok = 0;
		}
	}
	return ok;
}

static int SLAPI _DoProtect(PPDbEntrySet2 * pDbes, PPBackup * ppb)
{
	int    ok = 1;
	TDialog * dlg = new TDialog(DLG_PROTECT);
	if(CheckDialogPtrErr(&dlg)) {
		char   new_pw[64], old_pw[64];
		dlg->selectCtrl(CTL_LOGIN_PROTECT);
		SetupDBEntryComboBox(dlg, CTLSEL_LOGIN_DB, pDbes);
		dlg->disableCtrl(CTLSEL_LOGIN_DB, true);
		if(ExecView(dlg) == cmOK) {
			int    prot = 0;
			dlg->getCtrlData(CTL_LOGIN_PROTECT, &prot);
			dlg->getCtrlData(CTL_LOGIN_PASSWORD, old_pw);
			prot++;
			if((prot - 1) == 1 || PasswordDialog(0, new_pw, sizeof(new_pw), 0, 1) > 0) {
				long   dbentry = NZOR(pDbes->GetSelection(), pDbes->SetDefaultSelection());
				DbLoginBlock dlb;
				if(pDbes->GetByID(dbentry, &dlb) > 0) {
					if(ppb->LockDatabase() > 0) {
						if(!ProtectDatabase(&dlb, prot - 1, old_pw, new_pw))
							PPError();
						ppb->UnlockDatabase();
					}
				}
			}
		}
	}
	delete dlg;
	return ok;
}

int SLAPI CheckBuCopy(PPBackup * pPB, BackupDlgData * pBDD, int showDialog)
{
	int    ok = -1;
	BCopyData copy_data;
	if(pBDD->CopyID) {
		SString copy_dir, wildcard, temp_buf;
		int64  copy_size = 0;
		LDATE  copy_dt = ZERODATE;
		THROW_PP(pPB->GetCopyData(pBDD->CopyID, &copy_data), PPERR_DBLIB);
		(copy_dir = copy_data.CopyPath).SetLastSlash().Cat(copy_data.SubDir);
		PPSetAddedMsgString(copy_dir);
		THROW_PP_S(IsDirectory(copy_dir) > 0, PPERR_DIRNOTEXISTS, copy_dir);
		(wildcard = copy_dir).SetLastSlash().Cat("*.*");
		SDirec file_enum(wildcard);
		SDirEntry f_data;
		LDATETIME last_modif = ZERODATETIME;
		while(file_enum.Next(&f_data) > 0) {
			copy_size += f_data.Size;
			if(cmp(f_data.AccessTime, last_modif) == 1)
				last_modif = f_data.AccessTime;
		}
		copy_dt = last_modif.d;
		if(copy_size == 0 || copy_size != copy_data.DestSize)
			ok = (PPErrCode = PPERR_BUNOTEXISTSORINV, -1);
		else
			ok = 1;
		if(showDialog) {
			TDialog * p_dlg = 0;
			THROW(p_dlg = new TDialog(DLG_BUCHECK));
			p_dlg->disableCtrls(1, CTL_BUCHECK_LASTMODIF, CTL_BUCHECK_SIZE, CTL_BUCHECK_DIR, 0L);
			p_dlg->setCtrlData(CTL_BUCHECK_LASTMODIF, &copy_dt);
			p_dlg->setCtrlString(CTL_BUCHECK_SIZE, temp_buf.Z().Cat(copy_size));
			p_dlg->setCtrlString(CTL_BUCHECK_DIR, copy_dir);
			ExecViewAndDestroy(p_dlg);
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI DBMaintenance(PPDbEntrySet2 * pDbes, int autoMode)
{
	int    ok = 1;
	int    reply = cmCancel;
	PPBackup * ppb = 0;
	PPDbEntrySet2 local_dbes;
	if(pDbes == 0) {
		PPIniFile ini_file;
		local_dbes.ReadFromProfile(&ini_file);
		pDbes = &local_dbes;
	}
	if(autoMode == 1)
		_DoAutoBackup(pDbes, ppb, 1, 0);
	else if(autoMode == 2)
		reply = cmBuBackup;
	else {
		while(reply != cmCancel || (reply = DBMaintenanceFunctions(pDbes)) == cmBuBackup ||
			oneof7(reply, cmBuRestore, cmBuAutoBackup, cmRecover, cmProtect, cmDump, cmCreateDB, cmSetupBackupCfg) || reply == cmDbMonitor) {
			if(reply == cmBuAutoBackup) {
			   	_DoAutoBackup(pDbes, ppb, 1, 0);
				reply = cmCancel;
			}
			else if(reply == cmDbMonitor) {
				// TestMonitor();
				reply = cmCancel;
			}
			else {
				uint   rez_id = 0;
				if(reply == cmCreateDB) {
					ChangeDBList();
					reply = cmCancel;
				}
				else if(reply == cmSetupBackupCfg) {
					ConfigBackup();
					reply = cmCancel;
				}
				else if(reply == cmDump) {
					DoDbDump(pDbes);
					reply = cmCancel;
				}
				else {
					if(reply == cmBuBackup)
						rez_id = DLG_BU_BACKUP;
					else if(reply == cmBuRestore)
						rez_id = DLG_BU_RSTR;
					if((ppb = PPBackup::CreateInstance(pDbes)) != 0) {
						BackupDlgData bdd;
						if(reply == cmRecover) {
							_DoRecover(pDbes, ppb);
							reply = cmCancel;
						}
						else if(reply == cmProtect) {
							_DoProtect(pDbes, ppb);
							reply = cmCancel;
						}
						else {
							BackupDialog * dlg = 0;
							MEMSZERO(bdd);
							bdd.Cmd = reply;
							THROW(CheckDialogPtr(&(dlg = new BackupDialog(rez_id, &bdd, ppb))));
							while((reply = ExecView(dlg)) != cmCancel) {
								int    r = -1;
								dlg->getDTS(&bdd);
								if(reply == cmBuBackup)
									r = _DoBackup(ppb, bdd, UseCopyContinouos(pDbes));
								else if(reply == cmBuRestore)
						   			r = _DoRestore(ppb, bdd);
								else if(reply == cmBuRemove)
   	        		    			r = _DoRemoveCopy(ppb, bdd);
								else if(reply == cmBuReleaseContinuous && UseCopyContinouos(pDbes)) {
									BCopyData copy_data;
									copy_data.Set = bdd.Scen.Name;
									copy_data.CopyPath = bdd.Scen.BackupPath;
									copy_data.Flags = (bdd.Scen.Flags | BCOPYDF_RELEASECONT);
									PPWait(1);
									if(!ppb->Backup(&copy_data, CallbackBuLog, 0)) {
										PPSetError(PPERR_DBLIB);
										PPError();
										CallbackBuLog(BACKUPLOG_ERROR, 0, 0);
									}
									PPWait(0);
								}
		   	        			if(r > 0)
									dlg->updateCopyList();
						   		else if(r == 0)
						   			PPError();
							}
						}
					}
					else
						PPError();
				}
				ZDELETE(ppb);
				DBS.CloseDictionary();
			}
		}
	}
	CATCHZOK
	delete ppb;
	DBS.CloseDictionary();
	return ok;
}

int SLAPI CreateBackupCopy(const char * pActiveUser, int skipCfm)
{
	int    ok = -1;
	PPIniFile ini_file;
	PPBackup * ppb = 0;
	if(ini_file.IsValid()) {
		int    backup_user_found = 0;
		SString backup_user_list, backup_user;
		if(ini_file.Get((uint)PPINISECT_SYSTEM, (uint)PPINIPARAM_BACKUPUSER, backup_user_list) > 0) {
			PPDbEntrySet2 dbes;
			dbes.ReadFromProfile(&ini_file);
			uint k = 0;
			StringSet ss_backup_users(',', backup_user_list);
			for(uint i = 0; i < ss_backup_users.getCount(); i++) {
				ss_backup_users.get(&k, backup_user);
				if(backup_user.CmpNC(pActiveUser) == 0) {
					backup_user_found = 1;
					break;
				}
			}
			if(backup_user_found) {
				SString dbname;
				{
					SString dbname_path, n, data_path;
  					PPGetPath(PPPATH_DAT, dbname_path);
					dbname_path.RmvLastSlash();
					for(long i = 1; i <= (long)dbes.GetCount(); i++) {
						dbes.GetAttr(i, DbLoginBlock::attrDbSymb, n);
						dbes.GetAttr(i, DbLoginBlock::attrDbPath, data_path);
						if(n.NotEmptyS() && dbname_path.CmpNC(data_path) == 0) {
							dbes.SetSelection(i);
							dbname = n;
							break;
						}
					}
				}
				if((ppb = PPBackup::CreateInstance(&dbes)) != 0) {
					PPBackupScen scen;
					int    is_db_locked = 0, use_copy_continouos = 0;
					for(PPID scen_id = 0; ppb->EnumScen(&scen_id, &scen) > 0;) {
						int    do_backup = 0;
						BCopyData copy_data;
						if(ppb->GetLastScenCopy(&scen, &copy_data) > 0) {
							if(scen.Period > 0 && diffdate(getcurdate_(), copy_data.Dtm.d) > scen.Period)
								do_backup = 1;
						}
						else
							do_backup = 1;
						if(do_backup && dbname.CmpNC(scen.DBName) == 0 && stricmp(scen.Name, DefaultScenName)) {
							int cc = 1;
							if(!skipCfm) {
								cc = CONFIRM(PPCFM_DOBACKUP);
								skipCfm = 1;
							}
							if(cc && (use_copy_continouos || is_db_locked || ppb->LockDatabase() > 0)) {
								is_db_locked = BIN(!use_copy_continouos);
								if(!_DoAutoBackup(ppb, &scen, use_copy_continouos))
									ok = PPErrorZ(); // ����� ��������� �� ������
								else if(ok)
									ok = 1;
							}
							else
								break;
						}
					}
					if(is_db_locked)
						ppb->UnlockDatabase();
				}
				else
					ok = 0;
			}
		}
	}
	ZDELETE(ppb);
	return ok;
}

int SLAPI SetDatabaseChain()
{
	int    ok = -1, r;
	char   password[32];
	TDialog * dlg = 0;
	THROW(r = PPCheckDatabaseChain());
	if(r > 0) {
		THROW(CheckDialogPtr(&(dlg = new TDialog(DLG_DBUNCHAIN))));
		if(ExecView(dlg) == cmOK) {
			dlg->getCtrlData(CTL_DBUNCHAIN_PASSWORD, password);
			THROW(PPUnchainDatabase(password));
			ok = 1;
		}
	}
	else if(PasswordDialog(DLG_DBCHAIN, password, sizeof(password), 3, 1) > 0) {
		strip(password);
		THROW(PPChainDatabase(password[0] ? password : 0));
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}
//
//
//
//
// Data Protection
//
static int SLAPI ProtectDatabase(DbLoginBlock * pDlb, int protect, char * pPw, char * pNewPw)
{
	int    ok = 1;
	char   buf[128];
	if(PPMessage(mfConf|mfYes|mfCancel, PPCFM_PROTECT) == cmYes) {
		DbTableStat ts;
		StrAssocArray tbl_list;
		PPWait(1);
		THROW(DS.OpenDictionary2(pDlb, 0));
		{
			DbProvider * p_db = CurDict;
			p_db->GetListOfTables(0, &tbl_list);
			for(uint j = 0; j < tbl_list.getCount(); j++) {
				if(p_db->GetTableInfo(tbl_list.Get(j).Id, &ts) > 0) {
					PPWaitMsg(ts.TblName);
					DBS.GetProtectData(buf, 1);
					THROW_PP(stricmp(buf, pPw) == 0, PPERR_INVUSERORPASSW);
					THROW_DB(p_db->ProtectTable(ts.ID, pPw, pNewPw, protect));
					memzero(buf, sizeof(buf));
				}
			}
			if(protect == 0)
				THROW(p_db->SetupProtectData(pPw, pNewPw));
		}
		PPWait(0);
	}
	else
		ok = -1;
	CATCHZOK
	memzero(buf, sizeof(buf));
	return ok;
}
//
//
//
int SLAPI PPLicUpdate()
{
	int    ok = -1;
	char   path[MAXPATH];
	TDialog * dlg = 0;
	PPLicData lic;
	if(PPGetLicData(&lic)) {
		if(CheckDialogPtrErr(&(dlg = new TDialog(DLG_REGISTRATION)))) {
			FileBrowseCtrlGroup::Setup(dlg, CTLBRW_REGISTRAT_LICFILE, CTL_REGISTRATION_LICFILE, 1, 0, 0, FileBrowseCtrlGroup::fbcgfFile);
			dlg->setCtrlData(CTL_REGISTRATION_NAME,    lic.RegName);
			dlg->setCtrlData(CTL_REGISTRATION_SERIAL,  lic.RegNumber);
			dlg->setCtrlData(CTL_REGISTRATION_COUNT,   &lic.LicCount);
			dlg->disableCtrls(1, CTL_REGISTRATION_NAME, CTL_REGISTRATION_SERIAL, CTL_REGISTRATION_COUNT, 0);
			memset(&lic, ' ', sizeof(lic));
			for(int valid_data = 0; !valid_data && ExecView(dlg) == cmOK;) {
				dlg->getCtrlData(CTL_REGISTRATION_LICFILE, path);
				if(fileExists(strip(path)))
					if(PPUpdateLic(path[0] ? path : 0) > 0)
						valid_data = 1;
					else
						PPError();
				else
					PPError(PPERR_SLIB, path);
			}
		}
		else
			ok = 0;
	}
	else
		ok = PPErrorZ();
	delete dlg;
	return ok;
}

int SLAPI PPLicRegister()
{
	int    ok = -1;
	char   name[64], regkey[32], path[MAXPATH];
	TDialog * dlg = 0;
	if(CheckDialogPtrErr(&(dlg = new TDialog(DLG_REGISTRATION)))) {
		FileBrowseCtrlGroup::Setup(dlg, CTLBRW_REGISTRAT_LICFILE, CTL_REGISTRATION_LICFILE, 1, 0, 0, FileBrowseCtrlGroup::fbcgfFile);
		for(int valid_data = 0; !valid_data && ExecView(dlg) == cmOK;) {
			dlg->getCtrlData(CTL_REGISTRATION_NAME,    name);
			dlg->getCtrlData(CTL_REGISTRATION_SERIAL,  regkey);
			dlg->getCtrlData(CTL_REGISTRATION_LICFILE, path);
			strip(name);
			strip(regkey);
			strip(path);
			if(PPUpdateLic(path[0] ? path : 0, name, regkey))
				ok = valid_data = 1;
			else
				PPError();
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}
//
//
//
int DBMaintainParam::Read(SBuffer & rBuf, long)
{
	return rBuf.GetAvailableSize() ? (rBuf.Read(this, sizeof(DBMaintainParam)) ? 1 : PPSetErrorSLib()) : -1;
}

int DBMaintainParam::Write(SBuffer & rBuf, long) const
{
	return rBuf.Write(this, sizeof(DBMaintainParam)) ? 1 : PPSetErrorSLib();
}

class DBMaintainDlg : public TDialog {
public:
	DBMaintainDlg() : TDialog(DLG_DBMAINTAIN)
	{
	}
	int    setDTS(const DBMaintainParam *);
	int    getDTS(DBMaintainParam *);
private:
	DECL_HANDLE_EVENT;
	void   SetCtrls(long tables);
	DBMaintainParam Data;
};

IMPL_HANDLE_EVENT(DBMaintainDlg)
{
	TDialog::handleEvent(event);
	if(TVCOMMAND && TVCMD == cmClusterClk) {
		long   tables = 0;
		GetClusterData(CTL_DBMAINTAIN_TABLES, &tables);
		SetCtrls(tables);
		clearEvent(event);
	}
}

void DBMaintainDlg::SetCtrls(long tables)
{
	uint   ctl = CTL_DBMAINTAIN_DLSDAYS;
	for(long tbl = DBMaintainParam::tblDLS; tbl <= DBMaintainParam::tblXBill; tbl <<= 1, ctl++) {
		if(!(tables & tbl)) {
			int    days = 0;
			setCtrlData(ctl, &days);
		}
		disableCtrl(ctl, !(tables & tbl));
	}
}

int DBMaintainDlg::setDTS(const DBMaintainParam * pParam)
{
	if(!RVALUEPTR(Data, pParam))
		MEMSZERO(Data);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 0, DBMaintainParam::tblDLS);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 1, DBMaintainParam::tblMRP);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 2, DBMaintainParam::tblSJ);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 3, DBMaintainParam::tblRsrvSj);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 4, DBMaintainParam::tblXBill);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 5, DBMaintainParam::tblXBillRecover);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 6, DBMaintainParam::tblTempAltGGrp);
	AddClusterAssoc(CTL_DBMAINTAIN_TABLES, 7, DBMaintainParam::tblMoveObsolete); // @v9.0.3
	SetClusterData(CTL_DBMAINTAIN_TABLES,  Data.Tables);
	setCtrlData(CTL_DBMAINTAIN_DLSDAYS,   &Data.DLSDays);
	setCtrlData(CTL_DBMAINTAIN_MRPDAYS,   &Data.MRPDays);
	setCtrlData(CTL_DBMAINTAIN_SJDAYS,    &Data.SJDays);
	setCtrlData(CTL_DBMAINTAIN_XBILDAYS,  &Data.XBillDays);
	SetCtrls(Data.Tables);
	DisableClusterItem(CTL_DBMAINTAIN_TABLES, 3, !(CConfig.Flags & CCFLG_RSRVSJ));
	return 1;
}

int DBMaintainDlg::getDTS(DBMaintainParam * pParam)
{
	GetClusterData(CTL_DBMAINTAIN_TABLES, &Data.Tables);
	getCtrlData(CTL_DBMAINTAIN_DLSDAYS,   &Data.DLSDays);
	getCtrlData(CTL_DBMAINTAIN_MRPDAYS,   &Data.MRPDays);
	getCtrlData(CTL_DBMAINTAIN_SJDAYS,    &Data.SJDays);
	getCtrlData(CTL_DBMAINTAIN_XBILDAYS,  &Data.XBillDays);
	ASSIGN_PTR(pParam, Data);
	return 1;
}

int SLAPI DBMaintainDialog(DBMaintainParam * pParam) { DIALOG_PROC_BODY(DBMaintainDlg, pParam); }

int SLAPI DoDBMaintain(const DBMaintainParam * pParam)
{
	int    ok = -1, do_maintain = 1;
	PPLogger logger;
	DBMaintainParam param;
	if(!RVALUEPTR(param, pParam)) {
		ReadDBMaintainCfg(&param);
		THROW(do_maintain = DBMaintainDialog(&param));
	}
	if(do_maintain > 0) {
		LDATE to_dt = ZERODATE;
		PPWait(1);
		logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_START);
		if(param.Tables & (DBMaintainParam::tblSJ|DBMaintainParam::tblRsrvSj)) {
			SysJournal sj;
			to_dt = (param.Tables & DBMaintainParam::tblSJ) ? plusdate(getcurdate_(), -param.SJDays) : ZERODATE;
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_STARTSYSJ);
			THROW(sj.DoMaintain(to_dt, BIN(param.Tables & DBMaintainParam::tblRsrvSj), &logger));
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_ENDSYSJ);
		}
		/* @v9.8.11 ����� �� ��������� ��-�� ��������� ���������� �������� ������ ����������
		if(param.Tables & (DBMaintainParam::tblXBill|DBMaintainParam::tblXBillRecover)) {
			HistBillCore hb;
			to_dt = (param.Tables & DBMaintainParam::tblXBill) ? plusdate(getcurdate_(), -param.XBillDays) : ZERODATE;
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_STARTXBILL);
			THROW(hb.DoMaintain(to_dt, BIN(param.Tables & DBMaintainParam::tblXBillRecover), &logger));
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_ENDXBILL);
		}
		*/
		if(param.Tables & DBMaintainParam::tblDLS) {
			DeviceLoadingStat dls;
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_STARTDLS);
			THROW(dls.DoMaintain(plusdate(getcurdate_(), -param.DLSDays)));
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_ENDDLS);
		}
		if(param.Tables & DBMaintainParam::tblMRP) {
			PPObjMrpTab mrp_obj;
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_STARTMRP);
			THROW(mrp_obj.DoMaintain(plusdate(getcurdate_(), -param.MRPDays)));
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_ENDMRP);
		}
		if(param.Tables & DBMaintainParam::tblTempAltGGrp) {
			PPObjGoodsGroup gg_obj;
			GoodsGroupRecoverParam ggr_param;
			ggr_param.Flags |= GoodsGroupRecoverParam::fDelTempAltGrp;
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_STARTRMVTEMPALTGGRP);
			THROW(gg_obj.Recover(&ggr_param, &logger));
			logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_ENDRMVTEMPALTGGRP);
		}
		// @v9.0.3 {
		if(param.Tables & DBMaintainParam::tblMoveObsolete) {
			// @todo
		}
		// } @v9.0.3
	}
	CATCH
		ok = PPErrorZ();
		logger.LogLastError();
	ENDCATCH
	PPWait(0);
	if(do_maintain > 0) {
		logger.LogSubString(PPTXT_DBMAINTAINLOG, DBMAINTAINLOG_END);
		SString file_name;
		PPGetFilePath(PPPATH_LOG, PPFILNAM_INFO_LOG, file_name);
		logger.Save(file_name, 0);
	}
	return ok;
}

struct _DBMantainConfig {  // @persistent @store(PropertyTbl)
	PPID   Tag;            // Const=PPOBJ_CONFIG
	PPID   ID;             // Const=PPCFG_MAIN
	PPID   Prop;           // Const=PPPRP_DBMAINTAINCFG
	int16  DLSDays;
	int16  MRPDays;
	int16  SJDays;
	int16  XBillDays;
	long   Tables;
	char   Reserve[60];
};

int SLAPI ReadDBMaintainCfg(DBMaintainParam * pParam)
{
	int    ok = -1;
	DBMaintainParam  db_param;
	_DBMantainConfig dbm_cfg;
	if(PPRef->GetPropMainConfig(PPPRP_DBMAINTAINCFG, &dbm_cfg, sizeof(dbm_cfg)) > 0) {
		db_param.DLSDays   = dbm_cfg.DLSDays;
		db_param.MRPDays   = dbm_cfg.MRPDays;
		db_param.SJDays    = dbm_cfg.DLSDays;
		db_param.XBillDays = dbm_cfg.XBillDays;
		db_param.Tables    = dbm_cfg.Tables;
		ok = 1;
	}
	else
		MEMSZERO(db_param);
	ASSIGN_PTR(pParam, db_param);
	return ok;
}

int SLAPI EditDBMaintainCfg()
{
	int    ok = -1, is_new = 0;
	DBMaintainParam  param;
	THROW(CheckCfgRights(PPCFGOBJ_DBMAINTAIN, PPR_READ, 0));
	THROW(is_new = ReadDBMaintainCfg(&param));
	if(DBMaintainDialog(&param) > 0) {
		_DBMantainConfig dbm_cfg;
		MEMSZERO(dbm_cfg);
		THROW(CheckCfgRights(PPCFGOBJ_DBMAINTAIN, PPR_MOD, 0));
		dbm_cfg.DLSDays   = param.DLSDays;
		dbm_cfg.MRPDays   = param.MRPDays;
		dbm_cfg.SJDays    = param.DLSDays;
		dbm_cfg.XBillDays = param.XBillDays;
		dbm_cfg.Tables    = param.Tables;
		THROW(PPObject::Helper_PutConfig(PPPRP_DBMAINTAINCFG, PPCFGOBJ_DBMAINTAIN, (is_new < 0), &dbm_cfg, sizeof(dbm_cfg), 1));
		ok = 1;
	}
	CATCHZOKPPERR
	return ok;
}
//
//
//
int SLAPI TestLargeVlrInputOutput()
{
	const  long test_obj_type = 999;
	const  long test_obj_id = 1;
	const  long test_prop_id = 1;

    int    ok = 1;
	Reference * p_ref = PPRef;
	SBuffer src_buf, dest_buf;
	PropertyTbl::Rec rec;
	{
		PPTransaction tra(1);
		THROW(tra);
		{
			//
			// �������� ����� ������
			//
			src_buf.Z();
			MEMSZERO(rec);
			rec.ObjType = test_obj_type;
			rec.ObjID = test_obj_id;
			rec.Prop = test_prop_id;
			src_buf.Write(&rec, offsetof(PropertyTbl::Rec, VT));
			{
				for(uint i = 1; i <= 60000; i++) {
					ulong value = i;
					THROW_SL(src_buf.Write(value));
				}
			}
			THROW(p_ref->PutPropSBuffer(test_obj_type, test_obj_id, test_prop_id, src_buf, 0));
			//
			dest_buf.Z();
			THROW(p_ref->GetPropSBuffer(test_obj_type, test_obj_id, test_prop_id, dest_buf) > 0);
			THROW(dest_buf.IsEqual(src_buf));
		}
		{
			//
			// ��������� ������ �� ����� �������� �������
			//
			src_buf.Z();
			MEMSZERO(rec);
			rec.ObjType = test_obj_type;
			rec.ObjID = test_obj_id;
			rec.Prop = test_prop_id;
			src_buf.Write(&rec, offsetof(PropertyTbl::Rec, VT));
			{
				for(uint i = 1; i <= 65000; i++) {
					ulong value = i;
					THROW_SL(src_buf.Write(value));
				}
			}
			THROW(p_ref->PutPropSBuffer(test_obj_type, test_obj_id, test_prop_id, src_buf, 0));
			//
			dest_buf.Z();
			THROW(p_ref->GetPropSBuffer(test_obj_type, test_obj_id, test_prop_id, dest_buf) > 0);
			THROW(dest_buf.IsEqual(src_buf));
		}
		{
			//
			// ��������� ������ �� ����� �������� �������
			//
			src_buf.Z();
			MEMSZERO(rec);
			rec.ObjType = test_obj_type;
			rec.ObjID = test_obj_id;
			rec.Prop = test_prop_id;
			src_buf.Write(&rec, offsetof(PropertyTbl::Rec, VT));
			{
				for(uint i = 1; i <= 30000; i++) {
					ulong value = i;
					THROW_SL(src_buf.Write(value));
				}
			}
			THROW(p_ref->PutPropSBuffer(test_obj_type, test_obj_id, test_prop_id, src_buf, 0));
			//
			dest_buf.Z();
			THROW(p_ref->GetPropSBuffer(test_obj_type, test_obj_id, test_prop_id, dest_buf) > 0);
			THROW(dest_buf.IsEqual(src_buf));
		}
		{
			//
			// ��������� ������ �� ����� ������ ���������� ������� (��� ������������� ��������� ���������� ���������)
			//
			src_buf.Z();
			MEMSZERO(rec);
			rec.ObjType = test_obj_type;
			rec.ObjID = test_obj_id;
			rec.Prop = test_prop_id;
			src_buf.Write(&rec, offsetof(PropertyTbl::Rec, VT));
			{
				for(uint i = 1; i <= 10; i++) {
					ulong value = i;
					THROW_SL(src_buf.Write(value));
				}
			}
			THROW(p_ref->PutPropSBuffer(test_obj_type, test_obj_id, test_prop_id, src_buf, 0));
			//
			dest_buf.Z();
			THROW(p_ref->GetPropSBuffer(test_obj_type, test_obj_id, test_prop_id, dest_buf) > 0);
			THROW(dest_buf.IsEqual(src_buf));
		}
		{
			//
			// �������� ������ (������� ������ �����)
			//
			src_buf.Z();
			THROW(p_ref->PutPropSBuffer(test_obj_type, test_obj_id, test_prop_id, src_buf, 0));
			//
			dest_buf.Z();
			THROW(p_ref->GetPropSBuffer(test_obj_type, test_obj_id, test_prop_id, dest_buf) < 0);
			THROW(dest_buf.IsEqual(src_buf));
		}
		//
		THROW(tra.Commit());
	}
	CATCHZOK
    return ok;
}
//
//
//
#if SLTEST_RUNNING // {

class PrcssrTestDb {
public:
	struct Param {
		enum {
			fDbProvider = 0x0001 // ����������� SQL-����������. � ��������� ������ - native db
		};
		long   Flags;
		long   NumTaSeries; // ���������� ����� ���������� ������� ������� � TestTa01
		SString WordsFileName;
		SString LogFileName;
		SString OutPath;    // ���� � ������ ������ //
	};
	SLAPI  PrcssrTestDb();
	SLAPI ~PrcssrTestDb();
	int    SLAPI InitParam(Param *);
	int    SLAPI EditParam(Param *);
	int    SLAPI Init(const Param *);
	int    SLAPI Run();
private:
	int    SLAPI GenerateString(char * pBuf, size_t maxLen);
	int    SLAPI CreateTa(int use_ta);
	int    SLAPI GenTa_Rec(TestTa01Tbl::Rec * pRec);
	int    SLAPI CreateRef01(long * pID);
	int    SLAPI CreateRef02(long * pID);
	int    SLAPI GenRef01_Rec(TestRef01Tbl::Rec * pRec);
	int    SLAPI GenRef02_Rec(TestRef02Tbl::Rec * pRec);
	int    SLAPI LogMessage(const char * pMsg);
	int    SLAPI LogGenRec(DBTable * pTbl);
	int    SLAPI OutputTa(const char * pFileName);
	int    SLAPI AnalyzeAndUpdateTa();
	int    SLAPI GetTaHistograms(SHistogram * pHgVal1, SHistogram * pHgVal2);

	struct UpdCbParam {
		enum {
			uIncV1 = 1,
			uUpdV2
		};
		PrcssrTestDb * P_Prcssr;
		double IncV1;
		double NewV2;
		long   Count;
		long   Mode;
		RealRange Filt;
	};
	static int TaRVal1_Update_Callback(DBTable * pTbl, const void * pBefore, const void * pAfter, void * extraPtr);

	Param  P;
	StrAssocArray WordList;
	PPIDArray Ref1List; // ������ ��������� ��������������� ����������� Ref1
	PPIDArray Ref2List; // ������ ��������� ��������������� ����������� Ref2
	SRandGenerator G;
	ulong  TaCount;     // ���������� (�� ������) ������� � ������� TestTa01
	TestRef01Tbl * P_Ref1;
	TestRef02Tbl * P_Ref2;
	TestTa01Tbl * P_Ta;
};

SLAPI PrcssrTestDb::PrcssrTestDb() : TaCount(0), P_Ref1(0), P_Ref2(0), P_Ta(0)
{
}

SLAPI PrcssrTestDb::~PrcssrTestDb()
{
	delete P_Ref1;
	delete P_Ref2;
	delete P_Ta;
}

int SLAPI PrcssrTestDb::InitParam(Param * pParam)
{
	if(pParam) {
		pParam->Flags = 0;
		pParam->NumTaSeries = 100;
		pParam->WordsFileName = 0;
		pParam->LogFileName = 0;
	}
	return 1;
}

int SLAPI PrcssrTestDb::EditParam(Param *)
{
	return 1;
}

int SLAPI PrcssrTestDb::Init(const Param * pParam)
{
	int    ok = 1;
	P = *pParam;
	G.Set(getcurdate_().v ^ getcurtime_().v);
	WordList.Z();
	if(P.WordsFileName) {
		SFile wf(P.WordsFileName, SFile::mRead);
		long   word_id = 0;
		SString temp_buf;
		THROW_SL(wf.IsValid());
		PROFILE_START
		while(wf.ReadLine(temp_buf) > 0) {
			THROW_SL(WordList.Add(++word_id, temp_buf.Chomp().Strip()));
		}
		PROFILE_END
	}
	P_Ref1 = new TestRef01Tbl;
	P_Ref2 = new TestRef02Tbl;
	P_Ta   = new TestTa01Tbl;
	P_Ta->getNumRecs(&TaCount);
	CATCHZOK
	return ok;
}

int SLAPI PrcssrTestDb::GenerateString(char * pBuf, size_t maxLen)
{
	int    ok = 1;
	uint   i;
	SString temp_buf, line_buf;
	uint num_words = labs(G.GetUniformInt(maxLen / 6))+1;
	if(WordList.getCount() > 10) {
		for(i = 0; i < num_words; i++) {
			uint pos = labs(G.GetUniformInt(WordList.getCount()));
			temp_buf = WordList.Get(pos).Txt;
			if(i)
				line_buf.Space();
			line_buf.Cat(temp_buf);
		}
	}
	else {
		const char * p_alphabet = "0123456789.-ABCDEFGHIJKLMNOPQRSTUVWXYZ��������������������������������";
		const size_t ab_len = sstrlen(p_alphabet);
		for(i = 0; i < num_words; i++) {
			temp_buf.Z();
			size_t word_len = labs(G.GetUniformInt(10))+1;
			for(uint j = 0; j < word_len; j++) {
				temp_buf.CatChar(labs(G.GetUniformInt(ab_len)));
			}
			if(i)
				line_buf.Space();
			line_buf.Cat(temp_buf);
		}
	}
	line_buf.ToOem().CopyTo(pBuf, maxLen);
	return ok;
}

int SLAPI PrcssrTestDb::LogMessage(const char * pMsg)
{
	return PPLogMessage(P.LogFileName, pMsg, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_COMP|LOGMSGF_DBINFO);
}

int SLAPI PrcssrTestDb::LogGenRec(DBTable * pTbl)
{
	SString msg_buf, msg_rec_buf;
	pTbl->putRecToString(msg_rec_buf, 1);
	msg_buf.Printf("Generated rec for '%s': %s", pTbl->GetTableName(), msg_rec_buf.cptr());
	return LogMessage(msg_buf);
}

int SLAPI PrcssrTestDb::CreateTa(int use_ta)
{
	int    ok = 1;
	uint   count = 0;
	BExtInsert * p_bei = 0;
	SString msg_buf;
	if(TaCount % 2 == 0)
		count = 1;
	else
		count = G.GetUniformInt(128)+1;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(count > 1) {
			THROW_MEM(p_bei = new BExtInsert(P_Ta));
		}
		for(uint i = 0; i < count; i++) {
			TestTa01Tbl::Rec rec;
			SDelay(20+labs(G.GetUniformInt(100)));
			THROW(GenTa_Rec(&rec));
			{
				P_Ta->copyBufFrom(&rec);
				LogGenRec(P_Ta);
			}
			if(p_bei) {
				THROW_DB(p_bei->insert(&rec));
			}
			else
				THROW_DB(P_Ta->insertRecBuf(&rec));
		}
		if(p_bei)
			THROW_DB(p_bei->flash());
		TaCount += count;
		THROW(tra.Commit());
	}
	LogMessage(msg_buf.Printf("Inserted %u recs to '%s'", count, P_Ta->GetTableName()));
	CATCHZOK
	delete p_bei;
	return ok;
}

int SLAPI PrcssrTestDb::GetTaHistograms(SHistogram * pHgVal1, SHistogram * pHgVal2)
{
	int    ok = 1;
	RECORDNUMBER nr = 0, nr_test = 0;
	P_Ta->getNumRecs(&nr);
	BExtQuery q(P_Ta, 0);
	q.selectAll();
	TestTa01Tbl::Key0 k0;
	MEMSZERO(k0);
	for(q.initIteration(0, &k0, spFirst); q.nextIteration() > 0;) {
		CALLPTRMEMB(pHgVal1, Put(P_Ta->data.RVal1));
		CALLPTRMEMB(pHgVal2, Put(P_Ta->data.RVal2));
		nr_test++;
	}
	if(nr_test != nr) {
		SString msg_buf;
		LogMessage(msg_buf.Printf("Error: scanned (%ul) recs count of table '%s' not equal to getNumRecs (%ul)", nr_test, P_Ta->GetTableName(), nr));
	}
	return ok;
}

int SLAPI PrcssrTestDb::OutputTa(const char * pFileName)
{
	int    ok = 1;
	if(P.OutPath.NotEmpty()) {
		SString temp_buf;
		(temp_buf = P.OutPath).SetLastSlash().Cat(pFileName);
		SFile f(temp_buf, SFile::mAppend);
		if(f.IsValid()) {
			TestTa01Tbl::Key0 k0;
			MEMSZERO(k0);
			if(P_Ta->search(0, &k0, spFirst))
				do {
					P_Ta->putRecToString(temp_buf, 0);
					f.WriteLine(temp_buf.CR());
				} while(P_Ta->search(0, &k0, spNext));
		}
		else {
			LogMessage("Error opening output file for writing");
			ok = 0;
		}
	}
	else
		ok = -1;
	return ok;
}

// static
int PrcssrTestDb::TaRVal1_Update_Callback(DBTable * pTbl, const void * pBefore, const void * pAfter, void * extraPtr)
{
	SString msg_buf;
	TestTa01Tbl::Rec before = *static_cast<const TestTa01Tbl::Rec *>(pBefore);
	TestTa01Tbl::Rec after = *static_cast<const TestTa01Tbl::Rec *>(pAfter);
	UpdCbParam * p_param = static_cast<UpdCbParam *>(extraPtr);
	p_param->Count++;
	if(p_param->Mode == UpdCbParam::uIncV1) {
		if(after.RVal1 != (before.RVal1 + p_param->IncV1)) {
			msg_buf.Printf("Error updating TestTa01::RVal1: %.6lf != %.6lf", after.RVal1, (before.RVal1 + p_param->IncV1));
			p_param->P_Prcssr->LogMessage(msg_buf);
		}
		after.RVal1 = before.RVal1;
		if(memcmp(&after, &before, sizeof(after)) != 0) {
			p_param->P_Prcssr->LogMessage("Error updating TestTa01::RVal1: another fields updated");
		}
		if(!p_param->Filt.Check(before.RVal1)) {
			p_param->P_Prcssr->LogMessage("Error updating TestTa01::RVal1: filter violation");
		}
	}
	else if(p_param->Mode == UpdCbParam::uUpdV2) {
		if(after.RVal2 != p_param->NewV2) {
			msg_buf.Printf("Error updating TestTa01::RVal2: %.6lf != %.6lf", after.RVal2, p_param->NewV2);
			p_param->P_Prcssr->LogMessage(msg_buf);
		}
		after.RVal2 = before.RVal2;
		if(memcmp(&after, &before, sizeof(after)) != 0) {
			p_param->P_Prcssr->LogMessage("Error updating TestTa01::RVal2: another fields updated");
		}
		if(!p_param->Filt.Check(before.RVal2)) {
			p_param->P_Prcssr->LogMessage("Error updating TestTa01::RVal2: filter violation");
		}
	}
	return 1;
}

int SLAPI PrcssrTestDb::AnalyzeAndUpdateTa()
{
	int    ok = 1;
	OutputTa("ta_before_upd.txt");
	SHistogram hg_v1; // ����������� �� ��������� TestTa01::RVal1
	SHistogram hg_v2; // ����������� �� ��������� TestTa01::RVal2
	hg_v1.SetupDynamic(0.0, 0.5);
	hg_v2.SetupDynamic(70.0, 0.65);
	GetTaHistograms(&hg_v1, &hg_v2);
	if(hg_v1.GetResultCount()) {
		uint p = hg_v1.GetResultCount() / 2;
		SHistogram::Result hr;
		if(hg_v1.GetResult(p, &hr) > 0) {
			double middle = (hr.Upp + hr.Low) / 2.0;
			uint n = 0;
			long   test_bin_id = 0;
			hg_v1.GetBinByVal(middle, &test_bin_id);
			assert(test_bin_id == hr.Id);
			do {
				middle += 100.0;
				n++;
			} while(hg_v1.GetBinByVal(middle, 0) == 1);
			UpdCbParam up;
			MEMSZERO(up);
			up.P_Prcssr = this;
			up.IncV1 = n * 100.0;
			up.Mode = UpdCbParam::uIncV1;
			up.Filt.Set(hr.Low, hr.Upp-0.00000001);
			THROW(updateForCb(P_Ta, 1, (P_Ta->RVal1 >= hr.Low && P_Ta->RVal1 < hr.Upp),
				set(P_Ta->RVal1, (P_Ta->RVal1 + up.IncV1)), TaRVal1_Update_Callback, &up));
			if(up.Count != hr.Count) {
				LogMessage("Error updating TestTa01::RVal1: invalid count of updated records");
			}
		}
	}
	if(hg_v2.GetResultCount()) {
		uint p = 0;
		SHistogram::Result hr;
		if(hg_v2.GetResult(p, &hr) > 0) {
			UpdCbParam up;
			MEMSZERO(up);
			up.P_Prcssr = this;
			up.NewV2 = 1000.0;
			up.Mode = UpdCbParam::uUpdV2;
			up.Filt.Set(hr.Low, hr.Upp-0.00000001);
			THROW(updateForCb(P_Ta, 1, (P_Ta->RVal2 >= hr.Low && P_Ta->RVal2 < hr.Upp),
				set(P_Ta->RVal2, dbconst(up.NewV2)), TaRVal1_Update_Callback, &up));
			if(up.Count != hr.Count) {
				LogMessage("Error updating TestTa01::RVal2: invalid count of updated records");
			}
		}
	}
	OutputTa("ta_after_upd.txt");
	CATCHZOK
	return ok;
}

int SLAPI PrcssrTestDb::GenTa_Rec(TestTa01Tbl::Rec * pRec)
{
	int    ok = 1;
	TestTa01Tbl::Rec rec;
	MEMSZERO(rec);
	getcurdatetime(&rec.Dt, &rec.Tm);
	//
	// ����������� �������� �������� Ref1ID =0.05
	//
	if(labs(G.GetUniformInt(100)) < 5) {
		rec.Ref1ID = 0;
	}
	else {
		//
		// ����������� ������� ������ Ref1ID =0.000001
		//
		if(G.GetUniformInt(1000000) == 0) {

		}
		else {
			if(Ref1List.getCount() == 0) {
				//
				// ���� � ����������� ��� �� ����� ������, �� ���������� ������� ����� ������� �����������
				//
				THROW(CreateRef01(&rec.Ref1ID));
			}
			else {
				//
				// ����������� �������� ������ �������� ����������� ��������� � ��������������� �����������
				// �� ������������ ���������� ����������.
				//
				double p = 1.0 / log((double)TaCount);
				if(labs(G.GetUniformInt(1000000)) < p * 1000000.0) {
					//
					// ������ � ����������� �������� ������ �������� �����������
					//
					THROW(CreateRef01(&rec.Ref1ID));
				}
				else {
					//
					// ���������� ������������ ������� �����������
					//
					uint pos = labs(G.GetUniformInt(Ref1List.getCount()));
					assert(pos < Ref1List.getCount());
					rec.Ref1ID = Ref1List.get(pos);
				}
			}
		}
	}
	//
	// ����������� �������� �������� Ref2ID =0.80
	//
	if(labs(G.GetUniformInt(100)) < 80) {
		rec.Ref2ID = 0;
	}
	else {
		//
		// ����������� ������� ������ Ref2ID =0.000001
		//
		if(G.GetUniformInt(1000000) == 0) {

		}
		else {
			if(Ref2List.getCount() == 0) {
				//
				// ���� � ����������� ��� �� ����� ������, �� ���������� ������� ����� ������� �����������
				//
				THROW(CreateRef02(&rec.Ref2ID));
			}
			else {
				//
				// ����������� �������� ������ �������� ����������� ��������� � ��������������� �����������
				// �� ������������ ���������� ����������.
				//
				double p = 1.0 / log((double)TaCount);
				if(labs(G.GetUniformInt(1000000)) < p * 1000000.0) {
					//
					// ������ � ����������� �������� ������ �������� �����������
					//
					THROW(CreateRef02(&rec.Ref2ID));
				}
				else {
					//
					// ���������� ������������ ������� �����������
					//
					uint pos = labs(G.GetUniformInt(Ref2List.getCount()));
					assert(pos < Ref2List.getCount());
					rec.Ref2ID = Ref2List.get(pos);
				}
			}
		}
	}
	rec.LVal = G.GetPoisson(10.0);
	rec.IVal = (int16)G.GetUniformInt(1000);
	rec.UIVal = (uint16)labs(G.GetUniformInt(1000));
	rec.RVal1 = round(5.0+G.GetGaussian(5.0/3.0), 5);
	rec.RVal2 = round(70.0 + G.GetGamma(10.0, 3.0), 5);
	GenerateString((char *)rec.S, sizeof(rec.S));
	ASSIGN_PTR(pRec, rec);
	CATCHZOK
	return ok;
}

int SLAPI PrcssrTestDb::CreateRef01(long * pID)
{
	int    ok = 1;
	long   id = 0;
	SString msg_buf;
	TestRef01Tbl::Rec rec;
	GenRef01_Rec(&rec);
	{
		P_Ref1->copyBufFrom(&rec);
		LogGenRec(P_Ref1);
	}
	TestRef01Tbl::Key1 k1;
	MEMSZERO(k1);
	k1.L = rec.L;
	k1.I16 = rec.I16;
	if(P_Ref1->search(1, &k1, spEq)) {
		id = P_Ref1->data.ID;
		rec.ID = id;
		//
		// ��������� ����� ������������ ��� ������������ ������� #05 (�������� S48)
		//

		// @log Ref1 by Key1 found
		THROW_DB(updateFor(P_Ref1, 0, (P_Ref1->ID == id),
			// L � I16 �� ������
			set(P_Ref1->UI16, dbconst(rec.UI16)).
			set(P_Ref1->F64, dbconst(rec.F64)).
			set(P_Ref1->F32, dbconst(rec.F32)).
			set(P_Ref1->D, dbconst(rec.D)).
			set(P_Ref1->T, dbconst(rec.T)).
			set(P_Ref1->S48, dbconst(rec.S48)).
			set(P_Ref1->S12, dbconst(rec.S12))));
		LogMessage(msg_buf.Printf("Updated rec of '%s' (reason: Key1 found)", P_Ref1->GetTableName()));
	}
	else {
		TestRef01Tbl::Key5 k5;
		MEMSZERO(k5);
		STRNSCPY(k5.S48, rec.S48);
		if(P_Ref1->search(5, &k5, spEq)) {
			id = P_Ref1->data.ID;
			rec.ID = id;
			//
			// ��������� ����� ������������ ��� ������������ ������� #01 (�������� L � I16)
			//

			// @log Ref1 by Key5 found
			THROW_DB(updateFor(P_Ref1, 0, (P_Ref1->ID == id),
				set(P_Ref1->L, dbconst(rec.L)).
				set(P_Ref1->I16, dbconst(rec.I16)).
				set(P_Ref1->UI16, dbconst(rec.UI16)).
				set(P_Ref1->F64, dbconst(rec.F64)).
				set(P_Ref1->F32, dbconst(rec.F32)).
				set(P_Ref1->D, dbconst(rec.D)).
				set(P_Ref1->T, dbconst(rec.T)).
				// S48 �� ������
				set(P_Ref1->S12, dbconst(rec.S12))));
			LogMessage(msg_buf.Printf("Updated rec of '%s' (reason: Key5 found)", P_Ref1->GetTableName()));
		}
		else {
			P_Ref1->copyBufFrom(&rec);
			THROW_DB(P_Ref1->insertRec(0, &id));
			Ref1List.add(id);
			LogMessage(msg_buf.Printf("Inserted rec to '%s'", P_Ref1->GetTableName()));
		}
	}
	ASSIGN_PTR(pID, id);
	CATCHZOK
	return ok;
}

int SLAPI PrcssrTestDb::GenRef01_Rec(TestRef01Tbl::Rec * pRec)
{
	int    ok = 1;
	TestRef01Tbl::Rec rec;
	MEMSZERO(rec);
	rec.L = G.GetUniformInt(100000);
	rec.I16 = (int16)G.GetUniformInt(32000);
	rec.UI16 = (uint16)labs(G.GetUniformInt(64000));
	rec.F64 = round(G.GetGaussian(1.0), 5);
	rec.F32 = (float)fabs(round(50.0+G.GetGaussian(2.0), 3));
	rec.D = plusdate(getcurdate_(), G.GetUniformInt(31));
	rec.T.settotalsec(getcurtime_().totalsec() + G.GetUniformInt(600));
	GenerateString(rec.S48, sizeof(rec.S48));
	GenerateString(rec.S12, sizeof(rec.S12));
	ASSIGN_PTR(pRec, rec);
	return ok;
}

int SLAPI PrcssrTestDb::CreateRef02(long * pID)
{
	int    ok = 1;
	long   id = 0;
	SString msg_buf;
	TestRef02Tbl::Rec rec;
	GenRef02_Rec(&rec);
	{
		P_Ref2->copyBufFrom(&rec);
		LogGenRec(P_Ref2);
	}
	TestRef02Tbl::Key1 k1;
	MEMSZERO(k1);
	k1.L = rec.L;
	k1.I16 = rec.I16;
	if(P_Ref2->search(1, &k1, spEq)) {
		id = P_Ref2->data.ID;
		rec.ID = id;
		//
		// ��������� ����� ����������� ��� ������������ ������� #05 (�������� S48)
		//

		// @log Ref2 by Key1 found
		THROW_DB(updateFor(P_Ref2, 0, (P_Ref2->ID == id),
			// L � I16 �� ������
			set(P_Ref2->UI16, dbconst(rec.UI16)).
			set(P_Ref2->F64, dbconst(rec.F64)).
			set(P_Ref2->F32, dbconst(rec.F32)).
			set(P_Ref2->D, dbconst(rec.D)).
			set(P_Ref2->T, dbconst(rec.T)).
			set(P_Ref2->S48, dbconst(rec.S48)).
			set(P_Ref2->S12, dbconst(rec.S12)).
			set(P_Ref2->N, dbconst(rec.N))));
		LogMessage(msg_buf.Printf("Updated rec of '%s' (reason: Key1 found)", P_Ref2->GetTableName()));
	}
	else {
		TestRef02Tbl::Key5 k5;
		MEMSZERO(k5);
		STRNSCPY(k5.S48, rec.S48);
		if(P_Ref2->search(5, &k5, spEq)) {
			id = P_Ref2->data.ID;
			rec.ID = id;
			//
			// ��������� ����� ������������ ��� ������������ ������� #01 (�������� L � I16)
			//

			// @log Ref2 by Key5 found
			THROW_DB(updateFor(P_Ref2, 0, (P_Ref2->ID == id),
				set(P_Ref2->L, dbconst(rec.L)).
				set(P_Ref2->I16, dbconst(rec.I16)).
				set(P_Ref2->UI16, dbconst(rec.UI16)).
				set(P_Ref2->F64, dbconst(rec.F64)).
				set(P_Ref2->F32, dbconst(rec.F32)).
				set(P_Ref2->D, dbconst(rec.D)).
				set(P_Ref2->T, dbconst(rec.T)).
				// S48 �� ������
				set(P_Ref2->S12, dbconst(rec.S12)).
				set(P_Ref2->N, dbconst(rec.N))));
			LogMessage(msg_buf.Printf("Updated rec of '%s' (reason: Key5 found)", P_Ref2->GetTableName()));
		}
		else {
			P_Ref2->copyBufFrom(&rec);
			THROW_DB(P_Ref2->insertRec(0, &id));
			Ref2List.add(id);
			LogMessage(msg_buf.Printf("Inserted rec to '%s'", P_Ref2->GetTableName()));
		}
	}
	ASSIGN_PTR(pID, id);
	CATCHZOK
	return ok;
}

int SLAPI PrcssrTestDb::GenRef02_Rec(TestRef02Tbl::Rec * pRec)
{
	int    ok = 1;
	TestRef02Tbl::Rec rec;
	MEMSZERO(rec);
	rec.L = G.GetPoisson(100.0);
	rec.I16 = (int16)G.GetPoisson(10.0);
	rec.UI16 = (uint16)G.GetPoisson(10.0);
	rec.F64 = round(1000.0 + G.GetGaussian(100.0), 2);
	rec.F32 = (float)fabs(round(50.0+G.GetGaussian(2.0), 3));
	rec.D = plusdate(getcurdate_(), G.GetUniformInt(60));
	rec.T.settotalsec(getcurtime_().totalsec() + G.GetUniformInt(300));
	GenerateString(rec.S48, sizeof(rec.S48));
	GenerateString(rec.S12, sizeof(rec.S12));
	GenerateString(rec.N, sizeof(rec.N));
	ASSIGN_PTR(pRec, rec);
	return ok;
}

int SLAPI PrcssrTestDb::Run()
{
	int    ok = 1;
	SString msg_buf, err_msg_buf;
	for(long i = 0; i < P.NumTaSeries; i++) {
		if(!CreateTa(1)) {
			PPGetLastErrorMessage(1, err_msg_buf);
			LogMessage(msg_buf.Printf("Error execution of PrcssrTestDb::CreateTa(): %s", err_msg_buf.cptr()));
			ok = 0;
		}
	}
	if(ok) {
		if(!AnalyzeAndUpdateTa()) {
			PPGetLastErrorMessage(1, err_msg_buf);
			LogMessage(msg_buf.Printf("Error execution of PrcssrTestDb::CreateTa(): %s", err_msg_buf.cptr()));
			ok = 0;
		}
	}
	return ok;
}

SLTEST_R(PrcssrTestDb)
{
	int    ok = 1, r = 0;
	uint   arg_no = 0;
	SString arg;
	PrcssrTestDb::Param param;
	PrcssrTestDb prcssr;
	prcssr.InitParam(&param);
	/*
	;
	; ���������:
	;   0 - ���������� ����� �������� ������� ����������
	;
	*/
	if(EnumArg(&arg_no, arg)) {
		if(arg.ToLong() > 0)
			param.NumTaSeries = arg.ToLong();
	}
	(param.WordsFileName = GetSuiteEntry()->InPath).SetLastSlash().Cat("dictwords.txt");
	(param.LogFileName = GetSuiteEntry()->OutPath).SetLastSlash().Cat("dbtest.log");
	param.OutPath = GetSuiteEntry()->OutPath;
	THROW(SLTEST_CHECK_NZ(prcssr.Init(&param)));
	THROW(SLTEST_CHECK_NZ(prcssr.Run()));
	CATCH
		CurrentStatus = ok = 0;
	ENDCATCH
	return CurrentStatus;
}

SLTEST_R(TestDbSerialization)
{
	int    ok = 1;
	uint   i;
	SString raw_file_name, srlz_file_name;

	(raw_file_name = GetSuiteEntry()->OutPath).SetLastSlash().Cat("raw.bin");
	(srlz_file_name = GetSuiteEntry()->OutPath).SetLastSlash().Cat("srlz.bin");

	PPIDArray serial_list;
	PPIDArray id_list;

	int bill_first = 1;
	int lot_first = 1;
	int cc_first = 1;
	int todo_first = 1;

	BillTbl bill_tbl;
	ReceiptTbl lot_tbl;
	CCheckCore cc_tbl;
	PrjTaskTbl todo_tbl;

	BillTbl::Key0 bill_k0;
	ReceiptTbl::Key0 lot_k0;
	CCheckTbl::Key0 cc_k0;
	PrjTaskTbl::Key0 todo_k0;
	{
		SFile raw_file(raw_file_name, SFile::mWrite|SFile::mBinary);
		SFile srlz_file(srlz_file_name, SFile::mWrite|SFile::mBinary);

		SSerializeContext ctx;
		SBuffer srlz_buf, state_buf;
		ctx.Init(SSerializeContext::fSeparateDataStruct, getcurdate_());
		THROW(SLTEST_CHECK_NZ(raw_file.IsValid()));
		THROW(SLTEST_CHECK_NZ(srlz_file.IsValid()));
		for(i = 0; i < 100000; i++) {
			ulong rn = SLS.GetTLA().Rg.Get();
			int sp;
			//
			// ��������� ������������������ ����� ������� ��� ������������ �������������� //
			//
			const long s = (rn % 4);
			switch(s) {
				case 0: // bill
					{
						sp = bill_first ? spFirst : spNext;
						if(bill_tbl.search(0, &bill_k0, sp)) {
							BillTbl::Rec bill_rec;
							bill_tbl.copyBufTo(&bill_rec);
							//
							// �������� ����������������� ������� BNFieldList::IsEqualRecords
							//
							THROW(SLTEST_CHECK_NZ(bill_tbl.GetFields().IsEqualRecords(&bill_rec, &bill_tbl.data)));
							THROW(SLTEST_CHECK_NZ(ctx.Serialize(bill_tbl.GetTableName(), &bill_tbl.GetFieldsNonConst(), &bill_tbl.data, srlz_buf)));
							THROW(SLTEST_CHECK_NZ(raw_file.Write(&bill_tbl.data, sizeof(bill_tbl.data))));
							serial_list.add(s);
							id_list.add(bill_rec.ID);
							bill_first = 0;
						}
					}
					break;
				case 1: // receipt
					{
						sp = lot_first ? spFirst : spNext;
						if(lot_tbl.search(0, &lot_k0, sp)) {
							ReceiptTbl::Rec lot_rec;
							lot_tbl.copyBufTo(&lot_rec);
							//
							// �������� ����������������� ������� BNFieldList::IsEqualRecords
							//
							THROW(SLTEST_CHECK_NZ(lot_tbl.GetFields().IsEqualRecords(&lot_rec, &lot_tbl.data)));
							THROW(SLTEST_CHECK_NZ(ctx.Serialize(lot_tbl.GetTableName(), &lot_tbl.GetFieldsNonConst(), &lot_tbl.data, srlz_buf)));
							THROW(SLTEST_CHECK_NZ(raw_file.Write(&lot_tbl.data, sizeof(lot_tbl.data))));
							serial_list.add(s);
							id_list.add(lot_rec.ID);
							lot_first = 0;
						}
					}
					break;
				case 2: // ccheck
					{
						sp = cc_first ? spFirst : spNext;
						if(cc_tbl.search(0, &cc_k0, sp)) {
							CCheckTbl::Rec cc_rec;
							cc_tbl.copyBufTo(&cc_rec);
							//
							// �������� ����������������� ������� BNFieldList::IsEqualRecords
							//
							THROW(SLTEST_CHECK_NZ(cc_tbl.GetFields().IsEqualRecords(&cc_rec, &cc_tbl.data)));
							THROW(SLTEST_CHECK_NZ(ctx.Serialize(cc_tbl.GetTableName(), &cc_tbl.GetFieldsNonConst(), &cc_tbl.data, srlz_buf)));
							THROW(SLTEST_CHECK_NZ(raw_file.Write(&cc_tbl.data, sizeof(cc_tbl.data))));
							{
								CCheckLineTbl::Rec ccl_rec;
								for(int rbc = 0; cc_tbl.EnumLines(cc_rec.ID, &rbc, &ccl_rec) > 0;) {
									THROW(SLTEST_CHECK_NZ(ctx.Serialize(cc_tbl.Lines.GetTableName(), &cc_tbl.Lines.GetFieldsNonConst(), &ccl_rec, srlz_buf)));
									THROW(SLTEST_CHECK_NZ(raw_file.Write(&ccl_rec, sizeof(ccl_rec))));
								}
							}
							serial_list.add(s);
							id_list.add(cc_rec.ID);
							cc_first = 0;
						}
					}
					break;
				case 3: // prjtask
					{
						sp = todo_first ? spFirst : spNext;
						if(todo_tbl.search(0, &todo_k0, sp)) {
							PrjTaskTbl::Rec todo_rec;
							todo_tbl.copyBufTo(&todo_rec);
							//
							// �������� ����������������� ������� BNFieldList::IsEqualRecords
							//
							THROW(SLTEST_CHECK_NZ(todo_tbl.GetFields().IsEqualRecords(&todo_rec, &todo_tbl.data)));
							THROW(SLTEST_CHECK_NZ(ctx.Serialize(todo_tbl.GetTableName(), &todo_tbl.GetFieldsNonConst(), &todo_tbl.data, srlz_buf)));
							THROW(SLTEST_CHECK_NZ(raw_file.Write(&todo_tbl.data, sizeof(todo_tbl.data))));
							serial_list.add(s);
							id_list.add(todo_rec.ID);
							todo_first = 0;
						}
					}
					break;
			}
		}
		THROW(SLTEST_CHECK_NZ(ctx.SerializeStateOfContext(+1, state_buf)));
		THROW(SLTEST_CHECK_NZ(srlz_file.Write(state_buf)));
		THROW(SLTEST_CHECK_NZ(srlz_file.Write(srlz_buf)));
	}
	//
	// ������������ ��������
	//
	THROW(SLTEST_CHECK_EQ(serial_list.getCount(), id_list.getCount()));
	//
	// �������������� ������
	//
	{
		SFile srlz_file(srlz_file_name, SFile::mRead|SFile::mBinary);
		SSerializeContext ctx;
		SBuffer srlz_buf, state_buf;
		THROW(SLTEST_CHECK_NZ(srlz_file.Read(state_buf)));
		THROW(SLTEST_CHECK_NZ(srlz_file.Read(srlz_buf)));
		THROW(SLTEST_CHECK_NZ(ctx.SerializeStateOfContext(-1, state_buf)));
		for(i = 0; i < serial_list.getCount(); i++) {
			const int s = serial_list.get(i);
			PPID id = id_list.get(i);
			switch(s) {
				case 0: // bill
					{
						BillTbl::Rec bill_rec;
						SLTEST_CHECK_NZ(bill_tbl.search(0, &id, spEq));
						THROW(SLTEST_CHECK_NZ(ctx.Unserialize(bill_tbl.GetTableName(), &bill_tbl.GetFields(), &bill_rec, srlz_buf)));
						THROW(SLTEST_CHECK_NZ(bill_tbl.GetFields().IsEqualRecords(&bill_rec, &bill_tbl.data)));
					}
					break;
				case 1: // receipt
					{
						ReceiptTbl::Rec lot_rec;
						SLTEST_CHECK_NZ(lot_tbl.search(0, &id, spEq));
						THROW(SLTEST_CHECK_NZ(ctx.Unserialize(lot_tbl.GetTableName(), &lot_tbl.GetFields(), &lot_rec, srlz_buf)));
						THROW(SLTEST_CHECK_NZ(lot_tbl.GetFields().IsEqualRecords(&lot_rec, &lot_tbl.data)));
					}
					break;
				case 2: // ccheck
					{
						CCheckTbl::Rec cc_rec;
						SLTEST_CHECK_NZ(cc_tbl.search(0, &id, spEq));
						THROW(SLTEST_CHECK_NZ(ctx.Unserialize(cc_tbl.GetTableName(), &cc_tbl.GetFields(), &cc_rec, srlz_buf)));
						THROW(SLTEST_CHECK_NZ(cc_tbl.GetFields().IsEqualRecords(&cc_rec, &cc_tbl.data)));
						{
							CCheckLineTbl::Rec ccl_rec, ccl_rec2;
							for(int rbc = 0; cc_tbl.EnumLines(cc_rec.ID, &rbc, &ccl_rec) > 0;) {
								THROW(SLTEST_CHECK_NZ(ctx.Unserialize(cc_tbl.Lines.GetTableName(), &cc_tbl.Lines.GetFields(), &ccl_rec2, srlz_buf)));
								THROW(SLTEST_CHECK_NZ(cc_tbl.Lines.GetFields().IsEqualRecords(&ccl_rec, &ccl_rec2)));
							}
						}
					}
					break;
				case 3: // prjtask
					{
						PrjTaskTbl::Rec todo_rec;
						SLTEST_CHECK_NZ(todo_tbl.search(0, &id, spEq));
						THROW(SLTEST_CHECK_NZ(ctx.Unserialize(todo_tbl.GetTableName(), &todo_tbl.GetFields(), &todo_rec, srlz_buf)));
						THROW(SLTEST_CHECK_NZ(todo_tbl.GetFields().IsEqualRecords(&todo_rec, &todo_tbl.data)));
					}
					break;
			}
		}
	}
	CATCH
		CurrentStatus = ok = 0;
	ENDCATCH
	return CurrentStatus;
}

#endif // } SLTEST_RUNNING
