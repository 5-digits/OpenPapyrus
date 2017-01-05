// Dll ��� ������ � ���������� ���������� INGENICO �� ��������� SPDH (���� ���)
//
#pragma hdrstop
#include <slib.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val)     {if(!(expr)){SetError(val);goto __scatch;}}
#define THROWERR_ADD(expr,val)     {if(!(expr)){SetAddError(val);goto __scatch;}}

int	ErrorCode = 0;
int LastError = 0;
int AddError = 0; // ��� ������, ������������ ���������� ������ �� LastError
				// ������������ ��� INGVTB_NOTINITED, INGVTB_NOTCONNECTED
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// ���� ������
#define OPER_PAY			1	// ������
#define OPER_REFUND			3	// �������
#define OPER_CLOSESESS		11	// �������� �����
#define OPER_CLOSEDAY		12	// �������� ���
#define OPER_TESTCONNECT	96	// ���� �����

// ���� ������
#define INGVTB_NOTENOUGHPARAM	300	// �� ���������� ���������� ��� ������ ����������
#define INGVTB_UNCNKOWNCOMMAND	301	// �������� ����������� �������
#define INGVTB_NOTINITED		302	// ������ �������������
#define INGVTB_NOTCONNECTED		303	// ���������� �� �����������

#define	INGVTB_NOTENOUGHMEM		400 // �� ����������� ������ ��������� ������
#define INGVTB_DLLFILENOTFOUND	401 // ���� arccom.dll �� ������
#define INGVTB_INIFILENOTFOUND	402 // ���� cashreg.ini �� ������
#define INGVTB_CHECKFILENOTFOUND	403 // ���� check.out �� ������
#define INGVTB_NODLLPATH		404 // �� ������ ���� � ����� arccom.dll
#define INGVTB_TRANSERR			405 // ������ ����������

struct ErrMessage {
	uint Id;
	const char * P_Msg;
};

enum {
	lnProcCode = 7,
	lnTrack2 = 60,
	lnPan = 20,
	lnExpiry = 5,
	lnAmount = 13,
	lnCurrency = 4,
	lnTerminalID = 9,
	lnSpdhTerminalID=17,
	lnRetrievalReference = 13,
	lnAuthIdentResponse = 9,
	lnResponseCode = 4,
	lnCardType = 80,
	lnDate = 7,
	lnTime = 7,
	lnBatchNum = 8,
	lnTrack2Credit = 50,
	lnPinblock = 17,
	lnPayData = 50,
	lnPayId = 3,
	lnMtid = 5,
	lnReceivedTextMsg=80,
	lnAID=80,
	lnApplicationLabel=80,
	lnTVR=80
};

struct UserAuthIntSt
{
	UserAuthIntSt()
	{
		Clear();
	}
	void Clear()
	{
		handle = 0;
		abg_id = 0;
		operType = 0;
		memzero(track2, lnTrack2);
		memzero(pan, lnPan);
		memzero(expiry, lnExpiry);
		memzero(pay_acc, lnPan);
		memzero(additional_payment_data, 80);
		memzero(amount, lnAmount);
		memzero(original_amount, lnAmount);
		memzero(currency, lnCurrency);
		memzero(terminalID, lnTerminalID);
		memzero(rrn, lnRetrievalReference);
		memzero(authCode, lnAuthIdentResponse);
		memzero(responseCode, lnResponseCode);
		memzero(cardType, lnCardType);
		memzero(date, lnDate);
		memzero(time, lnTime);
		memzero(payment_data, lnPayData);
		memzero(data_to_print, lnPayData);
		memzero(home_operator, lnPayData);
		memzero(received_text_message, lnReceivedTextMsg);
		memzero(text_message, lnReceivedTextMsg);
		memzero(AID, lnAID);
		memzero(ApplicationLabel, lnApplicationLabel);
		memzero(TVR, lnTVR);
		system_res = 0;
	}
	int	handle;
	int abg_id;
	int operType; //[in] ��� �������� (��������)
	char track2[lnTrack2]; //[in] ����2
	char pan[lnPan]; //[out] PAN
	char expiry[lnExpiry]; //[out] Expiry Date ����
	char pay_acc[lnPan]; //�� ������������
	char additional_payment_data[80]; //�� ������������
	char amount[lnAmount]; //[in] ����� � ��������
	char original_amount[lnAmount]; //[in]������������ ����� � ��������
	char currency[lnCurrency]; //[in] ��� ������
	char terminalID[lnTerminalID]; //[out] ID ���������
	char rrn[lnRetrievalReference]; //[in][out] ������ (��������� ������ ��� ��� ��������, ��� �������� 
		//��� �����, � ��������� ������� ������ ���� �����)
	char authCode[lnAuthIdentResponse]; //[in][out] ��� �����������
	char responseCode[lnResponseCode]; //[out] ��� ������
	char cardType[lnCardType]; //[out] �������� ���� �����
	char date[lnDate]; //[out] ���� ����������
	char time[lnTime]; //[out] ����� ����������
	char payment_data[lnPayData]; //�� ������������
	char data_to_print[lnPayData]; //�� ������������
	char home_operator[lnPayData]; //�� ������������
	char received_text_message[lnReceivedTextMsg];//�� ������������
	char text_message[lnReceivedTextMsg]; //[out] ������������
	char AID[lnAID]; //[out]EMV AID
	char ApplicationLabel[lnApplicationLabel]; //[out]EMV ApplicationLabel
	char TVR[lnTVR]; //[out]EMV TVR
	int system_res; //�� ������������
	// ������ ��� ������ �����2 � ������������ HRS
	// char enc_data[64]; //[in][out] ����������� ������ �����(PAN)
};

typedef int (__cdecl * ProcDll)(UserAuthIntSt *);
SDynLibrary * P_Lib = 0;
ProcDll ProcessOw = 0;

class BnkIngVTB {
public:
	SLAPI BnkIngVTB();
	SLAPI ~BnkIngVTB();
	int RunOneCommand(const char * pCmd, const char * pInputData, char * pOutputData, size_t outSize);
	int Connect();
	int Disconnect();
	int SetCfg();
	int GetLastErrorText(char * pBuf, size_t bufSize);
	int Pay(); // ������
	int Refund(); // �������
	int GetSessReport(char * pZCheck, size_t bufSize); // ����� �� ��������� �� ����

	SString IniPath; // ���� � ����� ini\cashreg.ini
	SString OutPath; // ���� � ����� check.out
private:
	int Port;
	SString Amount; // ����� ������/��������
	SString LogNum;
	SString TransErrStr; // �������� ������ ����������
	SString LastStr; // �������� ������, ������� �� ����������� � �������� �����
	UserAuthIntSt Transaction;
};

static BnkIngVTB * P_BnkIngVtb = 0;

ErrMessage ErrMsg[] = {
	{INGVTB_NOTENOUGHPARAM,		"�� ���������� ���������� ��� ������ ����������"},
	{INGVTB_UNCNKOWNCOMMAND,	"�������� ����������� �������"},
	{INGVTB_NOTINITED,			"������ �������������"},
	{INGVTB_NOTCONNECTED,		"���������� �� �����������"},
	{INGVTB_NOTENOUGHMEM,		"�� ����������� ������ ��������� ������"},
	{INGVTB_DLLFILENOTFOUND,	"���� arccom.dll �� ������"},
	{INGVTB_INIFILENOTFOUND,	"���� cashreg.ini �� ������"},
	{INGVTB_CHECKFILENOTFOUND,	"���� check.out �� ������"},
	{INGVTB_NODLLPATH,			"�� ������ ���� � ����� arccom.dll"}
};

int	FASTCALL SetError(int errCode);
int	FASTCALL SetError(char * pErrCode);
int	FASTCALL SetAddError(int errCode);
int GetLastErrorText(char * pBuf, size_t bufSize);

int FASTCALL SetError(int errCode) { ErrorCode = errCode; return 1; }
int FASTCALL SetError(SString & rErrCode) { ErrorCode = rErrCode.ToLong(); return 1; }
int	FASTCALL SetAddError(int errCode) { AddError = errCode; return 1; };
static int Init(const char * pLibName);
int Release();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return true; 
}

EXPORT int RunCommand(const char * pCmd, const char * pInputData, char * pOutputData, size_t outSize)
{
	int    ok = 0;
	THROW((pCmd != NULL) && (pOutputData != NULL) && (outSize != NULL));
	if(sstreqi_ascii(pCmd, "INIT")) {
		StringSet pairs(';', pInputData);
		SString s_pair, s_param, param_val;
		for(uint i = 0; pairs.get(&i, s_pair) > 0;){
			s_pair.Divide('=', s_param, param_val);
			if(s_param.CmpNC("DLLPATH"))
				break;
		}
		THROWERR(Init(param_val), INGVTB_NOTINITED);
	}
	else if(sstreqi_ascii(pCmd, "RELEASE"))
		THROW(Release())
	else if(P_BnkIngVtb)
		ok = P_BnkIngVtb->RunOneCommand(pCmd, pInputData, pOutputData, outSize);
	else if(sstreqi_ascii(pCmd, "GETLASTERRORTEXT")) { // ��� ������ ��������������
		ok = GetLastErrorText(pOutputData, outSize);
	}
	CATCH
		ok = 1;
		_itoa(ErrorCode, pOutputData, 10);
		LastError = ErrorCode;
		ErrorCode = 0;
	ENDCATCH;
	return ok;
}

int BnkIngVTB::RunOneCommand(const char * pCmd, const char * pInputData, char * pOutputData, size_t outSize)
{
	int ok = 0;
	SString s_pair, s_param, param_val;
	StringSet pairs(';', pInputData);
	if(sstreqi_ascii(pCmd, "CONNECT")) {
		for(uint i = 0; pairs.get(&i, s_pair) > 0;) {
			s_pair.Divide('=', s_param, param_val);
			if(s_param.CmpNC("PORT") == 0)
				Port = param_val.ToLong();
		}
		THROWERR(Connect(), INGVTB_NOTCONNECTED);
	}
	else if(sstreqi_ascii(pCmd, "DISCONNECT")) {
		THROW(Disconnect())
	}
	else if(sstreqi_ascii(pCmd, "SETCONFIG")) {
		for(uint i = 0; pairs.get(&i, s_pair) > 0;) {
			s_pair.Divide('=', s_param, param_val);
			if(s_param.CmpNC("LOGNUM") == 0)
				LogNum = param_val;
		}
		THROW(SetCfg());
	}	
	else if(sstreqi_ascii(pCmd, "GETCONFIG")) {
		ok = 0;
	}
	else if(sstreqi_ascii(pCmd, "PAY")) {
		for(uint i = 0; pairs.get(&i, s_pair) > 0;) {
			s_pair.Divide('=', s_param, param_val);
			if(s_param.CmpNC("AMOUNT") == 0)
				Amount = param_val;
		}
		THROW(Pay());
	}
	else if(sstreqi_ascii(pCmd, "REFUND")) {
		for(uint i = 0; pairs.get(&i, s_pair) > 0;) {
			s_pair.Divide('=', s_param, param_val);
			if(s_param.CmpNC("AMOUNT") == 0)
				Amount = param_val;
		}
		THROW(Refund());
	}
	else if(sstreqi_ascii(pCmd, "GETBANKREPORT")) {
		THROW((ok = GetSessReport(pOutputData, outSize)) != 1);
	}
	else if(sstreqi_ascii(pCmd, "GETLASTERRORTEXT")) {
		ok = BnkIngVTB::GetLastErrorText(pOutputData, outSize);
	}
	else { // ���� ���� �����������  �������, �� �������� �� ����
		THROWERR(0, INGVTB_UNCNKOWNCOMMAND);
	}
	CATCH
		ok = 1;
		_itoa(ErrorCode, pOutputData, 10);
		LastError = ErrorCode;
	ENDCATCH;
	ErrorCode = 0;
	return ok;
}

int Init(const char * pLibName) 
{
	int    ok = 1;
	SPathStruc path_struct;

	THROWERR_ADD(pLibName, INGVTB_NODLLPATH);
	P_Lib = new SDynLibrary(pLibName);
	THROWERR_ADD(P_Lib && P_Lib->IsValid(), INGVTB_DLLFILENOTFOUND);
	ProcessOw = (ProcDll)P_Lib->GetProcAddr("ProcessOw");
	THROWERR_ADD(P_Lib && ProcessOw, INGVTB_DLLFILENOTFOUND);
	if(!P_BnkIngVtb)
		P_BnkIngVtb = new BnkIngVTB;
	THROW(P_BnkIngVtb);
	path_struct.Split(pLibName);
	path_struct.Dir.ReplaceStr("DLL\\", "INI\\", 1);
	(P_BnkIngVtb->IniPath = 0).Cat(path_struct.Drv).CatChar(':').Cat(path_struct.Dir).Cat("cashreg.ini");

	{
		size_t pos = 0;
		int exit = 0;
		SString r_str;
		SFile file(P_BnkIngVtb->IniPath, SFile::mRead);
		THROWERR_ADD(file.IsValid(), INGVTB_INIFILENOTFOUND);
		while(!exit && (file.ReadLine(r_str) > 0)) {
			if(r_str.Search("CHEQ_FILE=", 0, 1, &pos)) {
				r_str.Excise(0, pos + strlen("CHEQ_FILE="));
				r_str.TrimRightChr('\n');
				exit = 1;
			}
		}
		path_struct.Dir.ReplaceStr("INI\\", "\\", 1);
		(P_BnkIngVtb->OutPath = 0).Cat(path_struct.Drv).CatChar(':').Cat(path_struct.Dir).Cat(/*"cheq.out"*/r_str);
	}

	CATCH
		ZDELETE(P_Lib);
		ZDELETE(P_BnkIngVtb);
		ok = 0;
	ENDCATCH;
	return ok;
}

int Release() 
{	
	ProcessOw = 0;
	if(P_BnkIngVtb)
		ZDELETE(P_BnkIngVtb);
	if(P_Lib)
		ZDELETE(P_Lib);
	return 1;
}

BnkIngVTB::BnkIngVTB()
{
	LastError = 0;
	IniPath = 0;
	OutPath = 0;
	LogNum = 0;
	Transaction.Clear();
}

BnkIngVTB::~BnkIngVTB()
{
}

int BnkIngVTB::Connect()
{
	int    ok = 1, exit = 0, result = 0;
	size_t pos = 0;
	int    tell = 0;
	SString r_str, w_str;
	// ������ � ����� �������� ����� �����
	/*SFile file(IniPath, SFile::mRead);
	THROWERR_ADD(file.IsValid(), INGVTB_INIFILENOTFOUND);
	while(!exit && (file.ReadLine(r_str) > 0)) {
		if(r_str.Search("PORT=", 0, 1, &pos)) {
			tell = file.Tell();
			file.Close();
			file.Open(IniPath, SFile::mReadWrite);
			THROWERR_ADD(file.IsValid(), INGVTB_INIFILENOTFOUND);
			GetComDvcSymb(comdvcsCom, Port + 1, 0, port_name, SIZEOFARRAY(port_name));
			w_str.CopyFrom(r_str);
			w_str.Excise(pos + 5, w_str.Len()).Cat(port_name);
			file.Seek(tell - r_str.Len() - 1);
			file.WriteLine(w_str);
			file.Close();
			exit = 1;
		}
	}*/
	Transaction.Clear();
	Transaction.operType = OPER_TESTCONNECT;
	result = ProcessOw(&Transaction);

	(r_str = 0).Cat(P_BnkIngVtb->Transaction.responseCode);
	(r_str = 0).Cat(P_BnkIngVtb->Transaction.text_message);
	(r_str = 0).Cat(P_BnkIngVtb->Transaction.rrn);
	(r_str = 0).Cat(P_BnkIngVtb->Transaction.authCode);
	(TransErrStr = 0).Cat(P_BnkIngVtb->Transaction.text_message);
	
	THROWERR_ADD(result == 0, INGVTB_TRANSERR);
	CATCH
		ok = 0;
	ENDCATCH;
	return ok;
}

int BnkIngVTB::Disconnect()
{
	return 1;
}

int BnkIngVTB::SetCfg()
{
	return 1;
}

int BnkIngVTB::Pay()
{
	int ok = 1, result = 0;
	SString str;
	P_BnkIngVtb->Transaction.Clear();
	memcpy(P_BnkIngVtb->Transaction.terminalID, P_BnkIngVtb->LogNum, lnTerminalID);
	P_BnkIngVtb->Transaction.operType = OPER_PAY;
	memcpy(P_BnkIngVtb->Transaction.currency, "810", lnCurrency); // ��� ������: �����
	memcpy(P_BnkIngVtb->Transaction.amount, P_BnkIngVtb->Amount, lnAmount);
	result = ProcessOw(&P_BnkIngVtb->Transaction);

	(str = 0).Cat(P_BnkIngVtb->Transaction.responseCode);
	(str = 0).Cat(P_BnkIngVtb->Transaction.text_message);
	(str = 0).Cat(P_BnkIngVtb->Transaction.currency);
	(str = 0).Cat(P_BnkIngVtb->Transaction.amount);
	(str = 0).Cat(P_BnkIngVtb->Transaction.rrn);
	(str = 0).Cat(P_BnkIngVtb->Transaction.authCode);
	(TransErrStr = 0).Cat(P_BnkIngVtb->Transaction.text_message);

	THROWERR(result == 0, INGVTB_TRANSERR);
	CATCH
		ok = 0;
	ENDCATCH;
	return ok;
}
	
int BnkIngVTB::Refund()
{
	int ok = 1, result = 0;
	SString str;
	P_BnkIngVtb->Transaction.Clear();
	memcpy(P_BnkIngVtb->Transaction.terminalID, P_BnkIngVtb->LogNum, lnTerminalID);
	P_BnkIngVtb->Transaction.operType = OPER_REFUND;
	memcpy(P_BnkIngVtb->Transaction.currency, "810", lnCurrency); // ��� ������: �����
	memcpy(P_BnkIngVtb->Transaction.amount, P_BnkIngVtb->Amount, lnAmount);
	result = ProcessOw(&P_BnkIngVtb->Transaction);

	(str = 0).Cat(P_BnkIngVtb->Transaction.responseCode);
	(str = 0).Cat(P_BnkIngVtb->Transaction.text_message);
	(str = 0).Cat(P_BnkIngVtb->Transaction.currency);
	(str = 0).Cat(P_BnkIngVtb->Transaction.amount);
	(str = 0).Cat(P_BnkIngVtb->Transaction.rrn);
	(str = 0).Cat(P_BnkIngVtb->Transaction.authCode);
	(TransErrStr = 0).Cat(P_BnkIngVtb->Transaction.text_message);

	THROWERR(result == 0, INGVTB_TRANSERR);
	CATCH
		ok = 0;
	ENDCATCH;
	return ok;
}

int BnkIngVTB::GetSessReport(char * pZCheck, size_t bufSize)
{
	int ok = 0, result = 0;
	int64 file_size = 0;
	size_t pos = 0;
	SString str;
	SFile file(OutPath, SFile::mRead);
	P_BnkIngVtb->Transaction.Clear();
	memcpy(P_BnkIngVtb->Transaction.terminalID, P_BnkIngVtb->LogNum, lnTerminalID);
	P_BnkIngVtb->Transaction.operType = OPER_CLOSEDAY;
	result = ProcessOw(&P_BnkIngVtb->Transaction);

	(str = 0).Cat(P_BnkIngVtb->Transaction.responseCode);
	(str = 0).Cat(P_BnkIngVtb->Transaction.text_message);
	(str = 0).Cat(P_BnkIngVtb->Transaction.rrn);
	(str = 0).Cat(P_BnkIngVtb->Transaction.authCode);
	(TransErrStr = 0).Cat(P_BnkIngVtb->Transaction.text_message);

	THROWERR(result == 0, INGVTB_TRANSERR);
	THROWERR(file.IsValid(), INGVTB_CHECKFILENOTFOUND);
	file.CalcSize(&file_size);
	if(file_size > bufSize)
		ok = 2;
	else {
		while(file.ReadLine(str) > 0) {
			for(size_t i = 0; i < str.Len(); i++) {
				if((str.C(i) > 0) && (str.C(i) < 32)) // �������� ����������� �������, ����� ����� ������, �������, �������� ������
					if(str.C(i) != '\n')
						str.Excise(i, 1);
			}
			memcpy(pZCheck + pos, str.CR(), str.Len());
			pos = strlen(pZCheck);
		}
	}
	CATCH
		ok = 1;
	ENDCATCH;
	return ok;
}

int BnkIngVTB::GetLastErrorText(char * pBuf, size_t bufSize)
{
	int  ok = 0, exit = 0;
	SString msg;
	if(LastError == INGVTB_TRANSERR)
		(msg = 0).Cat(TransErrStr);
	else {
		for(uint i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
			if(LastError == ErrMsg[i].Id) {
				(msg = 0).Cat(ErrMsg[i].P_Msg);
				break;
			}
		}
	}
	if(msg.Empty())
		msg.Cat("Error ").Cat(LastError);
	if(AddError) {
		if(AddError == INGVTB_TRANSERR)
			msg.Cat(": ").Cat(TransErrStr);
		else {
			for(uint i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
				if(AddError == ErrMsg[i].Id) {
					msg.Cat(": ").Cat(ErrMsg[i].P_Msg);
					exit = 1;
					break;
				}
			}
			if(!exit)
				msg.Cat(": ").Cat("Error ").Cat(AddError);
		}
	}
	if(msg.Len() + 1 > bufSize) {
		(LastStr = 0).Cat(msg);
		ok = 2;
	}
	else {
		memcpy(pBuf, msg, msg.Len() + 1);
		AddError = 0;
	}
	return ok;
}

int GetLastErrorText(char * pBuf, size_t bufSize)
{
	int  ok = 0, exit = 0;
	SString msg;
	for(uint i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
		if(LastError == ErrMsg[i].Id) {
			(msg = 0).Cat(ErrMsg[i].P_Msg);
			break;
		}
	}
	if(msg.Empty())
		msg.Cat("Error ").Cat(LastError);
	if(AddError) {
		for(uint i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
			if(AddError == ErrMsg[i].Id) {
				msg.Cat(": ").Cat(ErrMsg[i].P_Msg);
				exit = 1;
				break;
			}
		}
		if(!exit)
			msg.Cat(": ").Cat("Error ").Cat(AddError);
	}
	if(msg.Len() + 1 > bufSize)
		ok = 2;
	else {
		memcpy(pBuf, msg, msg.Len() + 1);
		AddError = 0;
	}
	return ok;
}