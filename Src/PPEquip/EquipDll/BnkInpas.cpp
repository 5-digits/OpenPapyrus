// BNKINPAS.CPP
// ���������� ��� ������ � ����������� INPAS, �������������� �����

#include <ppdrvapi.h>
#include <slib.h>
#include <comdisp.h>

extern PPDrvSession DRVS;

#define THROWERR(expr,val)     { if(!(expr)) { DRVS.SetErrCode(val); goto __scatch; } }

#define TIMEOUT                  180000  // ����� �� �������� � �������� ����� �������
#define RUBLE                    643     // ��� �����

// ������ ��������
#define INPAS_FUNC_INIT          26      // ������������� ����������
#define INPAS_FUNC_PAY           1       // ������
#define INPAS_FUNC_REFUND        4       // �������/������ ������
#define INPAS_FUNC_CLOSEDAY      59      // �������� ���

class PPDrvINPASTrmnl : public PPBaseDriver {
public:
	PPDrvINPASTrmnl()
	{
		SString file_name;
		getExecPath(file_name);
		(SlipLogFileName = file_name).SetLastSlash().Cat("INPAStrmnl_Slip.log"); // @v10.2.0
		DRVS.SetLogFileName(file_name.SetLastSlash().Cat("INPAStrmnl.log"));
	}
	int    ProcessCommand(const SString & rCmd, const char * pInputData, SString & rOutput);
	int	   Init(SString & rCheck);
	int    Pay(double amount, SString & rSlip);
	int	   Refund(double amount, SString & rSlip);
	int	   GetSessReport(SString & rCheck);
private:
	enum {
		InitResources = 1,          // ����� ������ DualConnectorInterface
		Exchange,                   // ����� ������ DualConnectorInterface
		FreeResources,              // ����� ������ DualConnectorInterface
		SetChannelTerminalParam,	// ����� ������ DualConnectorInterface
		Release,                    // ����� ������ ISAPacket
		ErrorCode,                  // ��-�� ������� ������ DualConnectorInterface
		ErrorDescription,           // ��-�� ������� ������ DualConnectorInterface
		Amount,                     // ��-�� ������� ������ ISAPacket
		AdditionalAmount,           // ��-�� ������� ������ ISAPacket
		CurrencyCode,               // ��-�� ������� ������ ISAPacket
		DateTimeHost,               // ��-�� ������� ������ ISAPacket
		CardEntryMode,              // ��-�� ������� ������ ISAPacket
		PINCodingMode,              // ��-�� ������� ������ ISAPacket
		PAN,                        // ��-�� ������� ������ ISAPacket
		CardExpiryDate,             // ��-�� ������� ������ ISAPacket
		TRACK2,                     // ��-�� ������� ������ ISAPacket
		AuthorizationCode,          // ��-�� ������� ������ ISAPacket
		ReferenceNumber,            // ��-�� ������� ������ ISAPacket
		ResponseCodeHost,           // ��-�� ������� ������ ISAPacket
		PinBlock,                   // ��-�� ������� ������ ISAPacket
		PinKey,                     // ��-�� ������� ������ ISAPacket
		WorkKey,                    // ��-�� ������� ������ ISAPacket
		TextResponse,               // ��-�� ������� ������ ISAPacket
		TerminalDateTime,           // ��-�� ������� ������ ISAPacket
		TrxID,                      // ��-�� ������� ������ ISAPacket
		OperationCode,              // ��-�� ������� ������ ISAPacket
		TerminalTrxID,              // ��-�� ������� ������ ISAPacket
		TerminalID,                 // ��-�� ������� ������ ISAPacket
		MerchantID,                 // ��-�� ������� ������ ISAPacket
		DebitAmount,                // ��-�� ������� ������ ISAPacket
		DebitCount,                 // ��-�� ������� ������ ISAPacket
		CreditAmount,               // ��-�� ������� ������ ISAPacket
		CreditCount,                // ��-�� ������� ������ ISAPacket
		OrigOperation,              // ��-�� ������� ������ ISAPacket
		MAC,                        // ��-�� ������� ������ ISAPacket
		Status,                     // ��-�� ������� ������ ISAPacket
		AdminTrack2,                // ��-�� ������� ������ ISAPacket
		AdminPinBlock,              // ��-�� ������� ������ ISAPacket
		AdminPAN,                   // ��-�� ������� ������ ISAPacket
		AdminCardExpiryDate,        // ��-�� ������� ������ ISAPacket
		AdminCardEntryMode,         // ��-�� ������� ������ ISAPacket
		VoidDebitAmount,            // ��-�� ������� ������ ISAPacket
		VoidDebitCount,             // ��-�� ������� ������ ISAPacket
		VoidCreditAmount,           // ��-�� ������� ������ ISAPacket
		VoidCreditCount,            // ��-�� ������� ������ ISAPacket
		ProcessingFlag,             // ��-�� ������� ������ ISAPacket
		HostTrxID,                  // ��-�� ������� ������ ISAPacket
		RecipientAddress,           // ��-�� ������� ������ ISAPacket
		CardWaitTimeout,            // ��-�� ������� ������ ISAPacket
		DeviceSerNumber,            // ��-�� ������� ������ ISAPacket
		CommandMode,                // ��-�� ������� ������ ISAPacket
		CommandMode2,               // ��-�� ������� ������ ISAPacket
		CommandResult,              // ��-�� ������� ������ ISAPacket
		FileData,                   // ��-�� ������� ������ ISAPacket
		MessageED,                  // ��-�� ������� ������ ISAPacket
		CashierRequest,             // ��-�� ������� ������ ISAPacket
		CashierResponse,            // ��-�� ������� ������ ISAPacket
		AccountType,                // ��-�� ������� ������ ISAPacket
		CommodityCode,              // ��-�� ������� ������ ISAPacket
		PaymentDetails,             // ��-�� ������� ������ ISAPacket
		ProviderCode,               // ��-�� ������� ������ ISAPacket
		Acquirer,                   // ��-�� ������� ������ ISAPacket
		AdditionalData,             // ��-�� ������� ������ ISAPacket
		ModelNo,                    // ��-�� ������� ������ ISAPacket
		ReceiptData,                // ��-�� ������� ������ ISAPacket
		TermResponseCode,           // ��-�� ������� ������ ISAPacket
		SlipNumber,                 // ��-�� ������� ������ ISAPacket
	};
	void AsseptDC(ComDispInterface * pNameDCObj) 
	{
		if(pNameDCObj) {
			ASSIGN_ID_BY_NAME(pNameDCObj, InitResources);
			ASSIGN_ID_BY_NAME(pNameDCObj, Exchange);
			ASSIGN_ID_BY_NAME(pNameDCObj, FreeResources);
			ASSIGN_ID_BY_NAME(pNameDCObj, SetChannelTerminalParam);
			ASSIGN_ID_BY_NAME(pNameDCObj, ErrorCode);
			ASSIGN_ID_BY_NAME(pNameDCObj, ErrorDescription);
		}
	}
	void AsseptSAP(ComDispInterface * pNameSAPObj) 
	{
		if(pNameSAPObj) {
			ASSIGN_ID_BY_NAME(pNameSAPObj, Amount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdditionalAmount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CurrencyCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, DateTimeHost);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CardEntryMode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, PINCodingMode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, PAN);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CardExpiryDate);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TRACK2);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AuthorizationCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ReferenceNumber);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ResponseCodeHost);
			ASSIGN_ID_BY_NAME(pNameSAPObj, PinBlock);
			ASSIGN_ID_BY_NAME(pNameSAPObj, PinKey);
			ASSIGN_ID_BY_NAME(pNameSAPObj, WorkKey);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TextResponse);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TerminalDateTime);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TrxID);
			ASSIGN_ID_BY_NAME(pNameSAPObj, OperationCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TerminalTrxID);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TerminalID);
			ASSIGN_ID_BY_NAME(pNameSAPObj, MerchantID);
			ASSIGN_ID_BY_NAME(pNameSAPObj, DebitAmount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, DebitCount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CreditAmount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CreditCount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, OrigOperation);
			ASSIGN_ID_BY_NAME(pNameSAPObj, MAC);
			ASSIGN_ID_BY_NAME(pNameSAPObj, Status);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdminTrack2);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdminPinBlock);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdminPAN);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdminCardExpiryDate);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdminCardEntryMode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, VoidDebitAmount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, VoidDebitCount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, VoidCreditAmount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, VoidCreditCount);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ProcessingFlag);
			ASSIGN_ID_BY_NAME(pNameSAPObj, HostTrxID);
			ASSIGN_ID_BY_NAME(pNameSAPObj, RecipientAddress);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CardWaitTimeout);
			ASSIGN_ID_BY_NAME(pNameSAPObj, DeviceSerNumber);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CommandMode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CommandMode2);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CommandResult);
			ASSIGN_ID_BY_NAME(pNameSAPObj, FileData);
			ASSIGN_ID_BY_NAME(pNameSAPObj, MessageED);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CashierRequest);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CashierResponse);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AccountType);
			ASSIGN_ID_BY_NAME(pNameSAPObj, CommodityCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, PaymentDetails);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ProviderCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, Acquirer);
			ASSIGN_ID_BY_NAME(pNameSAPObj, AdditionalData);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ModelNo);
			ASSIGN_ID_BY_NAME(pNameSAPObj, ReceiptData);
			ASSIGN_ID_BY_NAME(pNameSAPObj, TermResponseCode);
			ASSIGN_ID_BY_NAME(pNameSAPObj, SlipNumber);
			ASSIGN_ID_BY_NAME(pNameSAPObj, Release);
		}
	}
	SString SlipLogFileName;
};

// ������������ ��������
static SIntToSymbTabEntry _StatusMsgTab_SAP[] = {
	{0,		"������������� ������"},
	{1,		"��������"},
	{16,	"��������"},
	{17,	"��������� � OFFLINE"},
	{34,	"��� ����������"},
	{53,	"�������� ��������"}
};

// ������������ ������
static PPDrvSession::TextTableEntry _ErrMsgTab_DC[] = { 
	{0,		"������ ���������� �� �������"},
	{1,		"���� ������� ��������"},
	{2,		"������ �������� LOG �����"},
	{3,		"����� ������"},
	{4,		"������ ������ �������"},
	{6,		"�� ������ ���� ������������"},
	{7,		"������ ������� ����� ������������"},
	{8,		"������ ���������� �����������"},
	{9,		"������ � ���������� ���������"},
	{10,	"������ ��������� ���������� �� COM ����"},
	{11,	"������ � �������� ����������"},
	{12,	"������ ��� �������� ������ ����"},
	{13,	"������ ��������� ����� � �����������"},
	{14,	"������ � ���������� ��������� ���������� �������������� � �������������"}
};

PPDRV_INSTANCE_ERRTAB(InpasTrmnl, 1, 0, PPDrvINPASTrmnl, _ErrMsgTab_DC);

int PPDrvINPASTrmnl::Init(SString & rCheck)
{
	int    ok = 1;
	SString msg_ok, buf_ok;    // ����������, ����������� ��� ������ ���-�� � ���� � ������ ������
	int result_dc = 0;         // ��������� ��� ������ �� p_dclink
	int result_sar = 1;        // ��������� �������� �������� Status �� p_res

	ComDispInterface * p_req = 0;
	ComDispInterface * p_res = 0;
	ComDispInterface * p_dclink = new ComDispInterface;
	THROW(p_dclink);
	p_dclink->Init("DualConnector.DCLink");
	AsseptDC(p_dclink);
	p_dclink->CallMethod(InitResources);

	// ����������� ���������� �� in-������ � out-������ SAPacket
	p_req = new ComDispInterface;
	p_res = new ComDispInterface;
	THROW(p_dclink);
	p_req->Init("DualConnector.SAPacket");
	p_res->Init("DualConnector.SAPacket");
	AsseptSAP(p_req);
	AsseptSAP(p_res);

	// �������� ����������� ������� in-������� SAPacket
	p_req->SetProperty(OperationCode, INPAS_FUNC_INIT);

	// �������� ���������� ������ Exchange
	p_dclink->SetParam(p_req);
	p_dclink->SetParam(p_res);
	p_dclink->SetParam(TIMEOUT);
	// ����� ������ Exchange ������ DCLink
	p_dclink->CallMethod(Exchange);

	p_dclink->GetProperty(ErrorCode, &result_dc);
	p_res->GetProperty(Status, &result_sar);

	THROWERR(result_sar == 1, result_dc);									  // ���� �� 1, ������ ������. ������������ � ��������� ����������
	{																		  //   ���� ���������� ��������� ����������. 
		char slip_ch[1024];                // ������ ��� ����
		p_res->GetProperty(ReceiptData, slip_ch, sizeof(slip_ch));
		rCheck = slip_ch;
	}
	// ���� ��� ������, � ���� ���� ����� �� �������� ���������� ��������
	msg_ok.Cat("operation Init completed");
	DRVS.Log(msg_ok, 0xffff);

	// ������ ���������� � ������� � ����
	CATCH
		ok = 0;
		{
			SString msg, buf, rcode;
			int i = SIntToSymbTab_GetSymb(_StatusMsgTab_SAP, SIZEOFARRAY(_StatusMsgTab_SAP), result_sar, rcode);
			if(!i)
				rcode = _StatusMsgTab_SAP[0].P_Symb;
			DRVS.GetErrText(-1, msg);
			DRVS.GetErrText(result_dc, buf);
			msg.Space().Cat("Error Code").Space().Cat(result_dc).CatDiv(':', 2).Cat(buf).Space().Cat(rcode);
			DRVS.Log(msg, 0xffff);
		}
	ENDCATCH;
	// ������������ ��������
	p_req->CallMethod(Release);
	p_res->CallMethod(Release);
	p_dclink->CallMethod(FreeResources);
	if(p_res) {
		p_res->CallMethod(Release);
		delete p_res;
	}
	if(p_req) {
		p_req->CallMethod(Release);
		delete p_req;
	}
	if(p_dclink) {
		p_dclink->CallMethod(FreeResources);
		delete p_dclink;
	}
	return ok;
}

// ������
int PPDrvINPASTrmnl::Pay(double amount, SString & rSlip)
{
	int    ok = 1;
	SString msg_ok, buf_ok;            // ����������, ����������� ��� ������ ���-�� � ���� � ������ ������
	int result_dc = 0;                 // ��������� ��� ������ �� p_dclink
	int result_sar = 1;	               // ��������� �������� �������� Status �� p_res
	SString temp_buf;
	ComDispInterface * p_req = 0;
	ComDispInterface * p_res = 0;
	ComDispInterface * p_dclink = new ComDispInterface;
	THROW(p_dclink);
	p_dclink->Init("DualConnector.DCLink");
	AsseptDC(p_dclink);
	p_dclink->CallMethod(InitResources);

	// ����������� ���������� �� in-������ � out-������ SAPacket
	p_req = new ComDispInterface;
	p_res = new ComDispInterface;
	THROW(p_dclink);
	p_req->Init("DualConnector.SAPacket");
	p_res->Init("DualConnector.SAPacket");
	AsseptSAP(p_req);
	AsseptSAP(p_res);

	// �������� ����������� ������� in-������� SAPacket
	temp_buf.Z().Cat((long)R0(amount));
	p_req->SetProperty(Amount, temp_buf);
	p_req->SetProperty(CurrencyCode, RUBLE);
	p_req->SetProperty(OperationCode, INPAS_FUNC_PAY);
	
	// �������� ���������� ������ Exchange
	p_dclink->SetParam(p_req);
	p_dclink->SetParam(p_res);
	p_dclink->SetParam(TIMEOUT);
	// ����� ������ Exchange ������ DCLink
	p_dclink->CallMethod(Exchange);
	
	p_dclink->GetProperty(ErrorCode, &result_dc);
	p_res->GetProperty(Status, &result_sar);
																		 // ���� �� 1 ������ ������. ������������ � ��������� ����������
	THROWERR(result_sar == 1, result_dc); 								 //   ���� ���������� ��������� ����������. 
	{
		char slip_ch[1024]; // ������ ��� ����
		p_res->GetProperty(ReceiptData, slip_ch, sizeof(slip_ch));
		rSlip = slip_ch;
	}
	// @v10.2.0 {
	{
		temp_buf.Z().Cat(getcurdatetime_(), DATF_DMY|DATF_CENTURY, TIMF_HMS);
		SLS.LogMessage(SlipLogFileName, temp_buf, 8192);
		SLS.LogMessage(SlipLogFileName, rSlip, 8192);
	}
	// } @v10.2.0 
	// ���� ��� ������, � ���� ���� ����� �� �������� ���������� ��������
	msg_ok.Cat("operation Pay completed");
	DRVS.Log(msg_ok, 0xffff);

	// ������ ���������� � ������� � ����
	CATCH
		{
			SString msg, buf, rcode;
			int i = SIntToSymbTab_GetSymb(_StatusMsgTab_SAP, SIZEOFARRAY(_StatusMsgTab_SAP), result_sar, rcode);
			if(!i)
				rcode = _StatusMsgTab_SAP[0].P_Symb;
			DRVS.GetErrText(-1, msg);
			DRVS.GetErrText(result_dc, buf);
			msg.Space().Cat("Error Code").Space().Cat(result_dc).CatDiv(':', 2).Cat(buf).Space().Cat(rcode);
			DRVS.Log(msg, 0xffff);
		}
		ok = 0;
	ENDCATCH;
	// ������������ ��������
	if(p_res) {
		p_res->CallMethod(Release);
		delete p_res;
	}
	if(p_req) {
		p_req->CallMethod(Release);
		delete p_req;
	}
	if(p_dclink) {
		p_dclink->CallMethod(FreeResources);
		delete p_dclink;
	}
	return ok;
}

// �������
int PPDrvINPASTrmnl::Refund(double amount, SString & rSlip)
{
	int    ok = 1;
	SString msg_ok, buf_ok;     // ����������, ����������� ��� ������ ���-�� � ���� � ������ ������
	int    result_dc = 0;          // ��������� ��� ������ �� p_dclink
	int    result_sar = 1;         // ��������� �������� �������� Status �� p_res
	SString temp_buf;
	ComDispInterface * p_req = 0;
	ComDispInterface * p_res = 0;
	ComDispInterface * p_dclink = new ComDispInterface;
	THROW(p_dclink);
	p_dclink->Init("DualConnector.DCLink");
	AsseptDC(p_dclink);
	p_dclink->CallMethod(InitResources);

	// ����������� ���������� �� in-������ � out-������ SAPacket
	p_req = new ComDispInterface;
	p_res = new ComDispInterface;
	THROW(p_dclink);
	p_req->Init("DualConnector.SAPacket");
	p_res->Init("DualConnector.SAPacket");
	AsseptSAP(p_req);
	AsseptSAP(p_res);

	// �������� ����������� ������� in-������� SAPacket
	temp_buf.Z().Cat((long)R0(amount));
	p_req->SetProperty(Amount, temp_buf);
	p_req->SetProperty(CurrencyCode, RUBLE);
	p_req->SetProperty(OperationCode, INPAS_FUNC_REFUND);

	// �������� ���������� ������ Exchange
	p_dclink->SetParam(p_req);
	p_dclink->SetParam(p_res);
	p_dclink->SetParam(TIMEOUT);
	// ����� ������ Exchange ������ DCLink
	p_dclink->CallMethod(Exchange);

	p_dclink->GetProperty(ErrorCode, &result_dc);
	p_res->GetProperty(Status, &result_sar);
	// ���� �� 1, ������ ������. ������������ � ��������� ����������
	THROWERR(result_sar == 1, result_dc); //   ���� ���������� ��������� ����������. 
	{
		char slip_ch[1024];                // ������ ��� ����
		p_res->GetProperty(ReceiptData, slip_ch, sizeof(slip_ch));
		rSlip = slip_ch;
	}
	// @v10.2.0 {
	{
		temp_buf.Z().Cat(getcurdatetime_(), DATF_DMY|DATF_CENTURY, TIMF_HMS);
		SLS.LogMessage(SlipLogFileName, temp_buf, 8192);
		SLS.LogMessage(SlipLogFileName, rSlip, 8192);
	}
	// } @v10.2.0 
	// ���� ��� ������, � ���� ���� ����� �� �������� ���������� ��������
	msg_ok.Cat("operation Refund completed");
	DRVS.Log(msg_ok, 0xffff);
	// ������ ���������� � ������� � ����
	CATCH
		{
			SString msg, buf, rcode;
			int i = SIntToSymbTab_GetSymb(_StatusMsgTab_SAP, SIZEOFARRAY(_StatusMsgTab_SAP), result_sar, rcode);
			if(!i)
				rcode = _StatusMsgTab_SAP[0].P_Symb;
			DRVS.GetErrText(-1, msg);
			DRVS.GetErrText(result_dc, buf);
			msg.Space().Cat("Error Code").Space().Cat(result_dc).CatDiv(':', 2).Cat(buf).Space().Cat(rcode);
			DRVS.Log(msg, 0xffff);
		}
		ok = 0;
	ENDCATCH;
	// ������������ ��������
	if(p_res) {
		p_res->CallMethod(Release);	
		delete p_res;
	}
	if(p_req) {
		p_req->CallMethod(Release);
		delete p_req;
	}
	if(p_dclink) {
		p_dclink->CallMethod(FreeResources);
		delete p_dclink;
	}
	return ok;
}

int PPDrvINPASTrmnl::GetSessReport(SString & rCheck)
{
	int    ok = 1;
	SString msg_ok, buf_ok;    // ����������, ����������� ��� ������ ���-�� � ���� � ������ ������
	int result_dc = 0;         // ��������� ��� ������ �� p_dclink
	int result_sar = 1001;     // ��������� �������� �������� Status �� p_res

	ComDispInterface * p_req = 0;
	ComDispInterface * p_res = 0;
	ComDispInterface * p_dclink = new ComDispInterface;
	THROW(p_dclink);
	p_dclink->Init("DualConnector.DCLink");
	AsseptDC(p_dclink);
	p_dclink->CallMethod(InitResources);
	// ����������� ���������� �� in-������ � out-������ SAPacket
	p_req = new ComDispInterface;
	p_res = new ComDispInterface;
	THROW(p_dclink);
	p_req->Init("DualConnector.SAPacket");
	p_res->Init("DualConnector.SAPacket");
	AsseptSAP(p_req);
	AsseptSAP(p_res);
	
	// �������� ����������� ������� in-������� SAPacket
	p_req->SetProperty(OperationCode, INPAS_FUNC_CLOSEDAY);

	// �������� ���������� ������ Exchange
	p_dclink->SetParam(p_req);
	p_dclink->SetParam(p_res);
	p_dclink->SetParam(TIMEOUT);
	// ����� ������ Exchange ������ DCLink
	p_dclink->CallMethod(Exchange);
	p_dclink->GetProperty(ErrorCode, &result_dc);
	p_res->GetProperty(Status, &result_sar);
	THROWERR(result_sar == 1, result_dc); // ���� �� 1, ������ ������. ������������ � ��������� ����������  
	{																 //   ���� ���������� ��������� ����������. 
		char slip_ch[1024];				   // ������ ��� ����					   
		p_res->GetProperty(ReceiptData, slip_ch, sizeof(slip_ch));
		rCheck = slip_ch;
	}
	// ���� ��� ������, � ���� ���� ����� �� �������� ���������� ��������
	msg_ok.Cat("operation GetSessReport completed");
	DRVS.Log(msg_ok, 0xffff);

	// ������ ���������� � ������� � ����
	CATCH
		{
			SString msg, buf, rcode;
			int i = SIntToSymbTab_GetSymb(_StatusMsgTab_SAP, SIZEOFARRAY(_StatusMsgTab_SAP), result_sar, rcode);
			if(!i)
				rcode = _StatusMsgTab_SAP[0].P_Symb;
			DRVS.GetErrText(-1, msg);
			DRVS.GetErrText(result_dc, buf);
			msg.Space().Cat("Error Code").Space().Cat(result_dc).CatDiv(':', 2).Cat(buf).Space().Cat(rcode);
			DRVS.Log(msg, 0xffff);
		}
		ok = 0;
	ENDCATCH;
	// ������������ ��������
	if(p_res) {
		p_res->CallMethod(Release);
		delete p_res;
	}
	if(p_req) {
		p_req->CallMethod(Release);
		delete p_req;
	}
	if(p_dclink) {
		p_dclink->CallMethod(FreeResources);
		delete p_dclink;
	}
	return ok;
}

// virtual
int PPDrvINPASTrmnl::ProcessCommand(const SString & rCmd, const char * pInputData, SString & rOutput)
{
	int    err = 0;
	SString value;
	PPDrvInputParamBlock pb(pInputData);
	if(rCmd == "INIT") {
		THROW(Init(rOutput));
	}
	else if(rCmd == "PAY") {
		double amount = (pb.Get("AMOUNT", value) > 0) ? value.ToReal() : 0;
		THROW(Pay(amount, rOutput)); // @v10.3.3 @fix THROW
	}
	else if(rCmd == "REFUND") {
		double amount = (pb.Get("AMOUNT", value) > 0) ? value.ToReal() : 0;
		THROW(Refund(amount, rOutput)); // @v10.3.3 @fix THROW
	}
	else if(rCmd == "GETBANKREPORT") {
		// �������� ����� �� ��������� �� ���� (����� ������, �������� ������)
		GetSessReport(rOutput);
	}
	CATCH
		err = 1;
		{
			SString msg_buf;
			DRVS.Log((msg_buf = "Bank Terminal: error").CatDiv(':', 2).Cat(value), 0xffff);
		}
	ENDCATCH;
	return err;
}
