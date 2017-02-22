// OBJBROWS.CPP
// Copyright (c) A.Sobolev 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2016, 2017
//
#include <pp.h>
#pragma hdrstop
//
// PPObjBrowser
//
PPObjBrowser::PPObjBrowser(uint rezID, DBQuery * q, PPObject * pObj, uint _flags, long _extra) : BrowserWindow(rezID, q)
{
	flags = _flags;
	extra = _extra;
	SetPPObjPtr(pObj);
}

void PPObjBrowser::SetPPObjPtr(PPObject * pObj)
{
	ppobj = pObj;
	if(ppobj) {
		if(!ppobj->CheckRights(PPR_INS))
			flags &= ~OLW_CANINSERT;
		if(!ppobj->CheckRights(PPR_MOD))
			flags &= ~OLW_CANEDIT;
		if(!ppobj->CheckRights(PPR_DEL))
			flags &= ~OLW_CANDELETE;
	}
}

PPID PPObjBrowser::currID()
{
	const void * p_row = view ? view->getCurItem() : 0;
	return p_row ? *(PPID *)p_row : 0L;
}

void PPObjBrowser::updateView()
{
	CALLPTRMEMB(view, refresh());
}

IMPL_HANDLE_EVENT(PPObjBrowser)
{
	PPID   id;
	BrowserWindow::handleEvent(event);
	if(TVCOMMAND) {
		switch(TVCMD) {
			case cmaInsert:
				if((flags & OLW_CANINSERT) && ppobj->Edit(&(id = 0), (void *)extra) == cmOK)
					updateView();
				clearEvent(event);
				break;
			case cmaEdit:
				if(flags & OLW_CANEDIT) {
					if((id = currID()) != 0 && ppobj->Edit(&id, (void *)extra) == cmOK)
						updateView();
					clearEvent(event);
				}
				break;
			case cmaDelete:
				if(flags & OLW_CANDELETE) {
					if((id = currID()) != 0 && ppobj->RemoveObjV(id, 0, PPObject::rmv_default, (void *)extra) > 0)
						updateView();
					clearEvent(event);
				}
				break;
			default:
				break;
		}
	}
}
//
// PPObjListWindow
//
static ListBoxDef * __ListBoxDefFactory(PPID objType, StrAssocArray * pList, void * extraPtr)
{
	PPObject * p_obj = GetPPObject(objType, extraPtr);
	ListBoxDef * p_def = 0;
	if(p_obj && p_obj->GetImplementFlags() & PPObject::implTreeSelector)
		p_def = new StdTreeListBoxDef(pList, lbtDisposeData|lbtDblClkNotify, 0);
	else
		p_def = new StrAssocListBoxDef(pList, lbtDisposeData|lbtDblClkNotify);
	delete p_obj;
	return p_def;
}

PPObjListWindow::PPObjListWindow(PPID objType, StrAssocArray * pList, uint aFlags, void * extraPtr) :
	ListWindow(__ListBoxDefFactory(objType, pList, extraPtr), 0, 0)
{
	PPObject * p_obj = GetPPObject(objType, extraPtr);
	Init(p_obj, aFlags|OWL_OUTERLIST, extraPtr); // @v9.0.1 OWL_OUTERLIST
}

PPObjListWindow::PPObjListWindow(PPObject * aPPObj, uint aFlags, void * extraPtr) :
	ListWindow((aFlags & OLW_LOADDEFONOPEN) ? 0 : aPPObj->Selector(extraPtr), 0, 0)
{
	Init(aPPObj, aFlags, extraPtr);
}

void PPObjListWindow::Init(PPObject * aPPObj, uint aFlags, void * extraPtr)
{
	P_Obj = aPPObj;
	Flags = aFlags | OLW_CANEDIT;
	ExtraPtr = extraPtr;
	DefaultCmd = 0;
	if(P_Obj) {
		if(!P_Obj->CheckRights(PPR_INS))
			Flags &= ~OLW_CANINSERT;
		if(!P_Obj->CheckRights(PPR_DEL))
			Flags &= ~OLW_CANDELETE;
	}
}

PPObjListWindow::~PPObjListWindow()
{
	delete P_Obj;
}

int FASTCALL PPObjListWindow::valid(ushort command)
{
	if(command == cmOK) {
		PPID   id;
		int    r = 1;
		if(!getResult(&id))
			id = 0;
		if(P_Lb->isTreeList()) {
			r = ((StdTreeListBoxDef*)P_Lb->def)->HasChild(id);
			r = BIN(r && (Flags & OLW_CANSELUPLEVEL) || !r);
			if(r)
				r = P_Obj->ValidateSelection(id, Flags, ExtraPtr);
		}
		else if(P_Obj)
			r = P_Obj->ValidateSelection(id, Flags, ExtraPtr);
		if(r <= 0 && P_Obj) {
			if(r < 0) {
				P_Obj->UpdateSelector(P_Lb->def, ExtraPtr);
				P_Lb->setRange(P_Lb->def->getRecsCount());
				P_Lb->drawView();
			}
			return 0;
		}
	}
	return ListWindow::valid(command);
}

IMPL_HANDLE_EVENT(PPObjListWindow)
{
	if(DefaultCmd) {
		if(TVCOMMAND && TVCMD == cmLBDblClk)
			TVCMD = DefaultCmd;
		else if(TVKEYDOWN && TVKEY == kbEnter) {
			event.what = evCommand;
			TVCMD = DefaultCmd;
		}
	}
	ListWindow::handleEvent(event);
	if(P_Obj) {
		int    update = 0; // ���� ���������� ��������� �� �������� � id, �� 2
		PPID   id = 0;
		PPID   preserve_focus_id = 0; // � ��������� �������, ��� ��������������, ������������� id ����� ���� �������.
			// ��� ����� ������� ��������� preserve_focus_id ����� ��������� ����� ���������������� ������� �������.
		if(TVCOMMAND) {
			switch(TVCMD) {
				case cmLBLoadDef:
					if(!def && (Flags & OLW_LOADDEFONOPEN)) {
						ListWindowSmartListBox * p_box = listBox();
						if(p_box) {
							setDef(P_Obj->Selector(ExtraPtr));
							p_box->setDef(def);
							ComboBox * p_combo = p_box->combo;
							if(p_combo) {
								p_combo->setDef(def);
								p_combo->setDataByUndefID();
							}
						}
					}
					break;
				case cmaInsert:
					id = 0;
					if(Flags & OLW_CANINSERT && !(Flags & OWL_OUTERLIST)) { // @v9.0.1 !(Flags & OWL_OUTERLIST)
						if(P_Obj->Edit(&id, ExtraPtr) == cmOK) {
							preserve_focus_id = id;
							update = 2;
						}
						else
							::SetFocus(H());
					}
					break;
				case cmaDelete:
					if(Flags & OLW_CANDELETE && !(Flags & OWL_OUTERLIST) && getResult(&id) && id) { // @9.0.1 !(Flags & OWL_OUTERLIST)
						preserve_focus_id = id;
						if(P_Obj->RemoveObjV(id, 0, PPObject::rmv_default, ExtraPtr) > 0)
							update = 2;
					}
					break;
				case cmaEdit:
					if(Flags & OLW_CANEDIT && getResult(&id) && id) {
						RECT rc;
						if(GetWindowRect(H(), &rc)) {
                    		SLS.GetTLA().SetNextDialogLuPos(rc.right+1, rc.top);
						}
						preserve_focus_id = id;
						if(P_Obj->Edit(&id, ExtraPtr) == cmOK) {
							if(!(Flags & OWL_OUTERLIST)) // @v9.0.1
								update = 2;
							else {
								// @todo ���������� �������� ������, ���� ����� ������� ���������
								::SetFocus(H());
							}
						}
						else
							::SetFocus(H());
					}
					break;
				case cmTransmit:
					if(getResult(&id) && id)
						Transmit(id);
					break;
				case cmSysJournalByObj:
					if(getResult(&id) && id) {
						((PPApp*)APPL)->LastCmd = TVCMD;
						ViewSysJournal(P_Obj->Obj, id, 0);
					}
					break;
				default:
					return;
			}
			clearEvent(event);
		}
		else if(TVKEYDOWN)
			if(TVKEY == KB_CTRLENTER) {
				if(Flags & OLW_CANEDIT && getResult(&id) && id) {
					preserve_focus_id = id;
					if(P_Obj->Edit(&id, ExtraPtr) == cmOK) {
						if(!(Flags & OWL_OUTERLIST)) // @v9.0.1
							update = 2;
						else {
							// @todo ���������� �������� ������, ���� ����� ������� ���������
							::SetFocus(H());
						}
					}
					else
						::SetFocus(H());
					clearEvent(event);
				}
			}
			else if(TVKEY == kbAltF2) {
				if(Flags & OLW_CANINSERT && P_Obj->Obj == PPOBJ_GOODS && getResult(&id) && id) {
					PPID   new_id = 0;
					if(((PPObjGoods*)P_Obj)->AddBySample(&new_id, id) == cmOK) {
						preserve_focus_id = new_id;
						update = 2;
					}
					else
						::SetFocus(H());
					clearEvent(event);
				}
			}
			else
				return;
		PostProcessHandleEvent(update, preserve_focus_id);
	}
}

void PPObjListWindow::PostProcessHandleEvent(int update, PPID focusID)
{
	if(update) {
		P_Obj->UpdateSelector(P_Lb->def, ExtraPtr);
		P_Lb->drawView();
		P_Lb->setRange(P_Lb->def->getRecsCount());
		if(update == 2)
			P_Lb->search(&focusID, 0, srchFirst | lbSrchByID);
		::SetFocus(H());
	}
}

int PPObjListWindow::Transmit(PPID)
{
	int    ok = -1;
	if(P_Obj && IS_REF_OBJTYPE(P_Obj->Obj)) {
		ObjTransmitParam param;
		if(ObjTransmDialog(DLG_OBJTRANSM, &param) > 0) {
			PPID   id = 0;
			const PPIDArray & rary = param.DestDBDivList.Get();
			PPObjIDArray objid_ary;
			PPWait(1);
			for(id = 0; ((PPObjReference *)P_Obj)->EnumItems(&id, 0) > 0;)
				objid_ary.Add(P_Obj->Obj, id);
			THROW(PPObjectTransmit::Transmit(&rary, &objid_ary, &param));
			ok = 1;
		}
	}
	CATCHZOKPPERR
	PPWait(0);
	return ok;
}
//
// PPListDialog
//
SLAPI PPListDialog::PPListDialog(uint rezID, uint aCtlList, long flags) : TDialog(rezID)
{
	ctlList = aCtlList;
	Options = 0;
	if(flags & fOnDblClkOk)
		Options |= oOnDblClkOk;
	if(getCtrlView(STDCTL_OKBUTTON))
		Options |= oHasOkButton;
	if(getCtrlView(STDCTL_EDITBUTTON))
		Options |= oHasEditButton;
	P_Box = (SmartListBox*)getCtrlView(ctlList);
	if(!SetupStrListBox(P_Box))
		PPError();
	if(isCurrCtlID(ctlList) && (Options & oHasOkButton) && (Options & oHasEditButton)) {
		SetDefaultButton(STDCTL_OKBUTTON,   0);
	   	SetDefaultButton(STDCTL_EDITBUTTON, 1);
	}
	updateList(-1);
}

IMPL_HANDLE_EVENT(PPListDialog)
{
	long   p, i;
	TDialog::handleEvent(event);
	if(TVCOMMAND) {
		switch(TVCMD) {
			case cmaInsert:
				if(P_Box) {
					p = i = 0;
					int    r = addItem(&p, &i);
					if(r == 2)
						updateList(i, 0);
					else if(r > 0)
						updateList(p);
				}
				break;
			case cmaDelete:
				if(getCurItem(&p, &i) && delItem(p, i) > 0)
					updateList(-1);
				break;
			case cmaEdit:
				if(getCurItem(&p, &i) && editItem(p, i) > 0) {
					int is_tree_list = BIN(P_Box && P_Box->isTreeList());
					long id = (is_tree_list) ? i : p;
					updateList(id, !BIN(is_tree_list));
				}
				break;
			case cmaSendByMail:
				if(getCurItem(&p, &i))
					 sendItem(p, i);
				break;
			case cmDown:
			case cmUp:
				if(getCurItem(&p, &i)) {
					int    up = (TVCMD == cmUp) ? 1 : 0;
					if(moveItem(p, i, up) > 0)
						updateList(up ? p-1 : p+1);
				}
				break;
			case cmLBDblClk:
				if(P_Box && P_Box->def) {
					int    edit = 1, is_tree_list = 0;
					PPID   cur_id = 0;
					P_Box->def->getCurID(&cur_id);
					if(P_Box->isTreeList()) {
						is_tree_list = 1;
						if(((StdTreeListBoxDef*)P_Box->def)->HasChild(cur_id))
							edit = 0;
					}
					if(event.isCtlEvent(ctlList)) {
						if(cur_id && Options & oOnDblClkOk) {
							TView::messageCommand(this, cmOK);
						}
						else if(edit && getCurItem(&p, &i) && editItem(p, i) > 0) {
							long id = is_tree_list ? i : p;
							updateList(id, !BIN(is_tree_list));
						}
						else
							return;
					}
					else
						return;
				}
				else
					return;
				break;
			case cmRightClick:
				{
					SString temp_buf;
					if(PPLoadTextWin(PPTXT_MENU_LISTBOX, temp_buf)) {

						getCurItem(&p, &i);

						TMenuPopup menu;
						menu.AddSubstr(temp_buf, 0, cmaInsert);     // ��������
						menu.AddSubstr(temp_buf, 1, cmaEdit);       // �������������
						menu.AddSubstr(temp_buf, 2, cmaDelete);     // �������
						if(P_Box && P_Box->def && (P_Box->def->Options & lbtExtMenu))
							menu.AddSubstr(temp_buf, 3, cmaSendByMail); // ������� �� ��. �����
						int    cmd = menu.Execute(H(), TMenuPopup::efRet);
						if(cmd > 0)
							TView::messageCommand(this, cmd);
					}
				}
				break;
			default:
				return;
		}
	}
	else if(TVBROADCAST) {
		if((Options & oHasOkButton) && (Options & oHasEditButton) && ctlList) {
			if(TVCMD == cmReceivedFocus && event.isCtlEvent(ctlList)) {
				SetDefaultButton(STDCTL_OKBUTTON,   0);
				SetDefaultButton(STDCTL_EDITBUTTON, 1);
			}
			else if(TVCMD == cmReleasedFocus && event.isCtlEvent(ctlList)) {
				SetDefaultButton(STDCTL_OKBUTTON,   1);
				SetDefaultButton(STDCTL_EDITBUTTON, 0);
			}
		}
		return;
	}
	else if(TVKEYDOWN)
		if(TVKEY == KB_CTRLENTER) {
			if(Options & oHasOkButton) {
				if(IsInState(sfModal)) {
					endModal(cmOK);
					return; // ����� endModal �� ������� ���������� � this
				}
			}
			else
				TView::messageCommand(this, cmaMore, this);
		}
		else
			return;
	else
		return;
	clearEvent(event);
}

int SLAPI PPListDialog::getSelection(long * pID)
{
	return getCurItem(0, pID);
}

int SLAPI PPListDialog::addStringToList(long itemId, const char * pText)
{
	return (!P_Box || !P_Box->addItem(itemId, pText)) ? PPSetErrorSLib() : 1;
}

void SLAPI PPListDialog::updateList(long pos, int byPos /*= 1*/)
{
	if(P_Box /* @v7.4.1 && P_Box->def*/) {
		lock();
		int    sav_pos = P_Box->def ? (int)P_Box->def->_curItem() : 0;
		P_Box->freeAll();
		if(setupList()) {
			P_Box->drawView();
			if(byPos)
		   		P_Box->focusItem((pos < 0) ? sav_pos : pos);
			else
				P_Box->search(&pos, 0, srchFirst|lbSrchByID);
		}
		else
			PPError();
		unlock();
	}
}

int SLAPI PPListDialog::getCurItem(long * pPos, long * pID)
{
	if(P_Box && P_Box->def) {
		long   i = 0;
		P_Box->getCurID(&i);
		ASSIGN_PTR(pPos, P_Box->def->_curItem());
		ASSIGN_PTR(pID, i);
		return 1;
	}
	else
		return 0;
}

int PPListDialog::setupList() { return -1; }
int PPListDialog::addItem(long *, long *) { return -1; }
int PPListDialog::editItem(long, long) { return -1; }
int PPListDialog::delItem(long, long) { return -1; }
int PPListDialog::moveItem(long pos, long id, int up) { return -1; }
int PPListDialog::sendItem(long, long) { return -1; }
//
// ObjRestrictListDialog
//
ObjRestrictListDialog::ObjRestrictListDialog(uint dlgID, uint listCtlID) : PPListDialog(dlgID, listCtlID)
{
	ObjType = 0;
	P_ORList = 0;
	updateList(-1);
}

int ObjRestrictListDialog::setParams(PPID objType, ObjRestrictArray * pData)
{
	ObjType = objType;
	P_ORList = pData;
	return 1;
}

IMPL_HANDLE_EVENT(ObjRestrictListDialog)
{
	long   p, i;
	PPListDialog::handleEvent(event);
	if(TVCOMMAND && P_ORList) {
		if(TVCMD == cmaLevelUp || TVCMD == cmUp) {
			if(getCurItem(&p, &i) && p > 0) {
				P_ORList->swap(p, p-1);
				updateList(p-1);
			}
		}
		else if(TVCMD == cmaLevelDown || TVCMD == cmDown) {
			if(getCurItem(&p, &i) && p < (long)P_ORList->getCount()-1) {
				P_ORList->swap(p, p+1);
				updateList(p+1);
			}
		}
		else
			return;
	}
	else
		return;
	clearEvent(event);
}

int ObjRestrictListDialog::getObjName(PPID objID, long, SString & rBuf)
{
	if(ObjType) {
		if(!GetObjectName(ObjType, objID, rBuf))
			rBuf.Cat(objID);
		return 1;
	}
	else {
		rBuf = 0;
		return -1;
	}
}

void ObjRestrictListDialog::getExtText(PPID, long /*objFlags*/, SString & rBuf)
{
	rBuf = 0;
}

int ObjRestrictListDialog::setupList()
{
	if(P_ORList) {
		ObjRestrictItem * p_item;
		SString sub;
		for(uint i = 0; P_ORList->enumItems(&i, (void**)&p_item);) {
			StringSet ss(SLBColumnDelim);
			getObjName(p_item->ObjID, p_item->Flags, sub);
			ss.add(sub);
			getExtText(p_item->ObjID, p_item->Flags, sub);
			ss.add(sub);
			if(!P_Box->addItem(p_item->ObjID, ss.getBuf()))
				return PPSetErrorSLib();
		}
		return 1;
	}
	else
		return -1;
}

int ObjRestrictListDialog::addItem(long * pPos, long * pID)
{
	if(P_ORList) {
		uint   pos = 0;
		ObjRestrictItem item;
		MEMSZERO(item);
		if(editItemDialog(&item) > 0)
			if(P_ORList->Add(item.ObjID, item.Flags, &pos)) {
				ASSIGN_PTR(pID, item.ObjID);
				ASSIGN_PTR(pPos, pos);
				return 1;
			}
			else
				return 0;
	}
	return -1;
}

int ObjRestrictListDialog::editItem(long pos, long id)
{
	if(P_ORList) {
		const uint p = (uint)pos;
		if(p < P_ORList->getCount()) {
			ObjRestrictItem item = P_ORList->at(p);
			if(editItemDialog(&item) > 0) {
				P_ORList->at(p) = item;
				return 1;
			}
		}
	}
	return -1;
}

int ObjRestrictListDialog::delItem(long pos, long id)
{
	if(P_ORList && ((uint)pos) < P_ORList->getCount()) {
		P_ORList->atFree((uint)pos);
		return 1;
	}
	else
		return -1;
}
