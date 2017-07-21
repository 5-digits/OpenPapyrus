// REVERSK2.CPP
// ������ � ������������ ������-�2. ����� ���������� ���������� ��� ������ � ���������� ������-��� ����� ������ ����������
//
#include <ppdrvapi.h>
#include <conio.h>

extern PPDrvSession DRVS;

#define BROADCAST_DEV_ADDR	0xFFFF
#define FRAME_HDR_SZ	7
#define CRC16_SZ		2
#define ONLINE_EVENT_GRP_2	2 // ������ ������ ������� 2 (������� ���������� �����������)
//
// ������� �������� �����������
//
#define K2CMD_RESET			0x00	// ����� ��������
#define K2CMD_CONTRL_STATUS	0x06	// ��������� ������� ����������� (���������� � ������)
#define K2CMD_REQ_STATUS	0x07	// ��������� ������� ���������� ����������� �������
#define K2CMD_ONLINE_TASK	0x08	// �������� ������ ������� (��� ������� �������� ����������)
#define	K2CMD_OUTPUT_CTRL	0x0D	// ������ ���������� ��������
#define K2CMD_DEV_ADDR		0x0F	// ��������� ������ ����������� (��������� ������)
#define K2CMD_STATUS_READ	0x88	// �������������� � ���, ��� ������ ���������� ������ ������� �� ����������� �������
//
// ������ ����� ��������� ��� ������� ������� ���������� �������� (0x0D)
//
#define	OUTPUT_CTRL_LINE2      0x01 // ����� 2 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_LINE3      0x02 // ����� 3 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_LINE4      0x04 // ����� 4 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_EXEC_MECH  0x08 // ����� 5 - ���������� ��
#define	OUTPUT_CTRL_LINE1_2    0x10 // ����� 1.2 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_LINE1_3    0x20 // ����� 1.3 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_LINE1_4    0x40 // ����� 1.4 - "STOP" ��� "HOLD", ��� �� ������������
#define	OUTPUT_CTRL_EXEC_MECH1 0x80 // ����� 1.5 - ���������� ��1

// ��������� �� ���������
#define WICKET_ANSWER_TRANSP_OK	0    // ������������ ��. ������� ������� � ����������
#define WICKET_ANSWER_BUSY		1    // ����� ����������� �������
#define WICKET_ANSWER_OK		2    // �������� ���������
#define WICKET_ANSWER_NODATA	3    // ��� ������, ��������������� �������
#define WICKET_ANSWER_NOTASK	0xFF // ������� �� ���������

// �������������� ������ ������� ������ 2
#define ONLINE_EVENT_IN			0x20 // ��������� ��������� ������, �UI0-39� �������� ����������� �������
#define ONLINE_EVENT_OUT		0x21 // ��������� ��������� �������, �UI0-39� �������� ����������� �������
#define ONLINE_EVENT_POWER_ON	0x22 // ��������� �������, �������� �UI0-39� �� ������������
#define ONLINE_EVENT_POWER_OFF	0x23 // ���������� �������, �������� �UI0-39� �� ������������
#define ONLINE_EVENT_TIMER		0x24 // �������������� �������-���������,  �UI0-39� �������� ������������� �����, ������������, ���� �������� ���������� ��� ����������� ������� � ���������. ������� ���������������, ������������ �� ������������.
#define ONLINE_EVENT_CONNECTION_FAILED	0x25 // ������ ����� � ������, �������� �UI0-39� �� ������������
#define ONLINE_EVENT_CONNECTION_RECOVER	0x26 // �������������� ����� � ������, �������� �UI0-39� �� ������������
#define ONLINE_EVENT_BREAKING	0x27 // ������, �������� �UI0-7� ����� ������
#define ONLINE_EVENT_CLOSED		0x29 // �������� (����� �������� �����), �������� �UI0-7� ����� ������

// ��� ������� �� ������-�� h-�����. ����� ��� ������ � DeviceIoControl
#define SERIAL_PURGE_TXABORT	0x00000001
#define SERIAL_PURGE_RXABORT	0x00000002
#define SERIAL_PURGE_TXCLEAR	0x00000004
#define SERIAL_PURGE_RXCLEAR	0x00000008
#define IOCTL_SERIAL_PURGE		CTL_CODE(FILE_DEVICE_SERIAL_PORT,19,METHOD_BUFFERED,FILE_ANY_ACCESS)

// ���� ������
#define WICKET_NOTENOUGHPARAM	300	// �� ���������� ���������� ��� ������ ����������
#define WICKET_UNCNKOWNCOMMAND	301	// �������� ����������� �������
#define WICKET_NOTINITED		302	// ������ �������������
#define WICKET_NOTCONNECTED		303	// ���������� �� �����������

#define WICKET_NOTENOUGHMEM		400 // ������������� ������ ��������� �������
#define K2ERR_OPENPORTFAILED	401 // ������ �������� �����
#define K2ERR_WRITEPORTFAILED	402 // ������ ������ � ����
#define K2ERR_CONTROLPORTFAILED	403 // ������ �������� ������������ ���� � ����
#define K2ERR_MEMERR			404 // ������ ��������� ������
#define K2ERR_GETADDRFAILED		405 // ������ ��������� ������ �����������

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val)     { if(!(expr)) { DRVS.SetErrCode(val); goto __scatch; } }
//
// ������������ CRC16 �� 0xA001
//
static uint16 FASTCALL crc16_update(uint16 crc, uint8 a)
{
    crc ^= a;
    for(int i = 0; i < 8; ++i) {
        if(crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}
//
// ��������� ��������� ����� ��� ������� � ������
//
struct K2FrameHeaderSt {
	K2FrameHeaderSt()
	{
		Clear();
	}
	void CalcCrc()
	{
		Crc = 0xFFFF;
		Crc = crc16_update(Crc, (uchar)Addr);
		Crc = crc16_update(Crc, (uchar)(Addr >> 8));
		Crc = crc16_update(Crc, ReqNum);
		Crc = crc16_update(Crc, Cmd);
		Crc = crc16_update(Crc, DataLen);
	}
	void Clear()
	{
		Addr    = 0;
		ReqNum  = 0;
		Cmd     = 0;
		DataLen = 0;
		Crc     = 0xFFFF;
	}
	ushort Addr;    // ��������� ����� ���������� (����� ���������� �� ������� ��� � ������������)
	uchar  ReqNum;  // ����� ����� (����� ������������ ����� ������ ���������� �� ������ �����������. ������������������ ������� �� �����)
	uchar  Cmd;     // ����� ������� (�������). � ������ ������ ����� �������� ��������� � ������� ���������� �������
	uchar  DataLen; // ����� ������. �������� � ���� ���������� ����� ������ + 2 ����� CRC ������
	ushort Crc;     // CRC ���������
};
//
// ��������� ����� ��� ������� � ������
//
struct K2FrameSt {
	K2FrameSt()
	{
		P_Data = 0;
		Crc    = 0xFFFF;
		BufAllocated = 0;
		P_Data = FxBuf;
		Size = sizeof(FxBuf);
	}
	~K2FrameSt()
	{
		Clear();
	}
	void Clear()
	{
		H.Clear();
		if(BufAllocated) {
			ZDELETE(P_Data);
			BufAllocated = 0;
		}
		Crc = 0;
		Size = sizeof(FxBuf);
		P_Data = FxBuf;
	}
	void CalcCrc()
	{
		Crc = 0xFFFF;
		const uint sz = P_Data ? (H.DataLen - (uint)CRC16_SZ) : 0;
		for(uint i = 0; i < sz; i++)
			Crc = crc16_update(Crc, P_Data[i]);
	}
	//
	// Descr: �������� ������ ��� P_Data. ������� ������� �� Header
	//
	int AllocBuf()
	{
		int    ok = 1;
		if(H.DataLen > CRC16_SZ) {
			size_t sz = H.DataLen - CRC16_SZ;
			if(sz > Size) {
				if(BufAllocated)
					free(P_Data);
				P_Data = (char *)malloc(sz);
				BufAllocated = 1;
			}
			if(P_Data)
				memzero(P_Data, sz);
			else
				ok = 0;
		}
		return ok;
	}
	//
	// Descr: ����������� ������ ��������� � ����� ����� ����. ���������� ��� ����. ������ ��� � pBytes = NULL, �����
	// ������� ������ ��������� ������ ������. �� ������ ��� � ����� ������������ ������
	//
	void GetBytes(char * pBytes, size_t & rSize)
	{
	//#define CMD_PREFIX {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00};
	//#define CMD_PREFIX_SZ	11
		const char cmd_prefix[] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00};
		assert(sizeof(cmd_prefix) == 11);
		rSize = H.DataLen + FRAME_HDR_SZ + sizeof(cmd_prefix);
		if(pBytes != NULL) {
			memzero(pBytes, rSize);
			memcpy(pBytes, cmd_prefix, sizeof(cmd_prefix));
			memcpy(pBytes + sizeof(cmd_prefix), &H, FRAME_HDR_SZ);
			if(P_Data)
				memcpy(pBytes + sizeof(cmd_prefix) + FRAME_HDR_SZ, P_Data, H.DataLen - CRC16_SZ);
			memcpy(pBytes + sizeof(cmd_prefix) + FRAME_HDR_SZ + (H.DataLen - CRC16_SZ), &Crc, CRC16_SZ);
		}
	}
	//
	// ��������� ��������� ����� ��� ������� � ������
	//
	struct Header {
		Header()
		{
			Clear();
		}
		void CalcCrc()
		{
			Crc = 0xFFFF;
			Crc = crc16_update(Crc, (uchar)Addr);
			Crc = crc16_update(Crc, (uchar)(Addr >> 8));
			Crc = crc16_update(Crc, ReqNum);
			Crc = crc16_update(Crc, Cmd);
			Crc = crc16_update(Crc, DataLen);
		}
		void Clear()
		{
			Addr    = 0;
			ReqNum  = 0;
			Cmd     = 0;
			DataLen = 0;
			Crc     = 0xFFFF;
		}
		ushort Addr;    // ��������� ����� ���������� (����� ���������� �� ������� ��� � ������������)
		uchar  ReqNum;  // ����� ����� (����� ������������ ����� ������ ���������� �� ������ �����������. ������������������ ������� �� �����)
		uchar  Cmd;     // ����� ������� (�������). � ������ ������ ����� �������� ��������� � ������� ���������� �������
		uchar  DataLen; // ����� ������. �������� � ���� ���������� ����� ������ + 2 ����� CRC ������
		ushort Crc;     // CRC ���������
	};
	Header H;
	size_t Size;
	char * P_Data;	// ������, ���������� �� ����������
	ushort Crc;		// CRC ������
	int16  BufAllocated; //
	char   FxBuf[512];

	SString & ToString(SString & rBuf) const
	{
		rBuf.CatEq("Addr", (ulong)H.Addr).Space().CatEq("ReqNum", (ulong)H.ReqNum).Space().CatEq("Cmd", (ulong)H.Cmd).Space().CatEq("DataLen", (ulong)H.DataLen);
		return rBuf;
	}
};
//
// ��������� �������� ������-������� (��� ���� ������� ������ ��������� - 10 ����)
//
struct K2OnlineEventSt {
	enum {
		codeCode      = 0x07,
		codeReaderNum = 0x08,
		codeGrpNum    = 0xF0
	};
	K2OnlineEventSt()
	{
		Id = 0;
		Code = 0;
		MEMSZERO(Descr);
	}
	//
	// Descr: �������� ��� ������� (��� ������� Code)
	//
	int    GetEventCode() const
	{
		return (Code & codeCode);
	}
	//
	// Descr: �������� ����� ����������� (��� ������� Code)
	//
	int    GetReaderNum() const
	{
		return ((Code & codeReaderNum) >> 3);
	}
	//
	// Descr: �������� ����� ������ ������� (��� ������� Code)
	//
	int    GetGrpNum() const
	{
		return ((Code & codeGrpNum) >> 4);
	}
	int32	Id;	// ������������� �������
	uchar	Code;	// ��� �������
	// ������ ���� �������:
	//		����		��������
	//		0-2			��� �������
	//		3			����� �����������
	//		4-7			����� ������ �������
	uchar   Descr[5]; // �������� �������
};
//
// �������� ��������� ������/�������
//
struct K2InOutEventSt { // @persistent
	K2InOutEventSt() : InOutMask(0), EventMask(0), Undefined(0)
	{
	}
	ushort	InOutMask;	// ����� ��������������� ��������� ������ ��� �������
	ushort	EventMask;	// ����� ����������� �� ���� ��� ����� �� ������� ��������� ���������
	uchar	Undefined;	// �� ������������
};
//
// ����� ��� ������ � ������������ ������-�2
//
class K2Controller : public PPBaseDriver {
public:
	SFile LogFile;
protected:
	K2Controller();
	~K2Controller();
	int OpenPort(int portNum);
	int ClosePort();
	//
	// Descr: �������� ����� ����������� � ����� ��� � Addr
	//
	int GetAddr(uint16 & rAddr);
	//
	// Descr: ��������������� �����. ���������� ������ ����������� � ���������� ������� � ��������� ����������
	// Returns:
	//   ����� ReqNum � ������� ��� ������ ������
	//
	uchar Reset_(uint16 addr);
	//
	// Descr: �������� ������ �� �������� �������. ���� �� ����������� ������ �� �������� � ������� �������,
	// �������� � timeout, �� ������� ��������� ���� ������
	// Returns:
	//		0 - ������� �� ������
	//		1 - ��������� �������
	//
	int WaitEvent(uint16 addr, int timeout, int evtCode, ...);
	//
	// Descr: �������� ��������� ����� �� �������� �����
	// ARG(line	IN): ����� ����� (OUTPUT_CTRL_xxx)
	// ARG(tm	IN): ����� ���������
	//
	int ActivateLine(uint16 addr, int line, int tm);
private:
	//
	// Descr: ������ ������ � �����������. ������������� ��� ����. ������ ��� �������� ��������� �����,
	//	�� ���� � ����� ������. ������ ��� - ���� ������
	// Returns:
	//		0  - ��� ������� ������
	//		>0 - ���������� ����������� ����
	//
	int K2Read(K2FrameSt & rResp);
	//
	// Descr: ��������� � ��������� ��������� � �������� � ���������� �� �� ����������
	// ARG(addr     IN): �����, �� ������� �������� �������.
	//   0xffff - broadcasting
	//   ���� �����, ���������� ������� GetAddr()
	// ARG(cmd		IN): �������, ������� ���� ��������� (K2CMD_xxx)
	// ARG(pData	IN): ������������ ���������
	// ARG(dataSize	IN): ������ ������������ ����������
	// Returns:
	//   ����� ReqNum, � ������� ��� ������ ������
	//
	uchar SendReq(uint16 addr, int cmd, const void * pData, size_t dataSize);

	HANDLE Handle;
	//ushort SerialN;
	uint16 Reserve; // @alignment
};
//
// ����� ��� ������ � ����������
//
class PPDrvWicket : public K2Controller {
public:
	PPDrvWicket()
	{
		SString file_name;
		getExecPath(file_name);
		DRVS.SetLogFileName(file_name.SetLastSlash().Cat("wicket.log"));
	}
	virtual int ProcessCommand(const SString & rCmd, const char * pInputData, SString & rOutput)
	{
		int    err = 0;
		SString value;
		PPDrvInputParamBlock pb(pInputData);
		if(rCmd == "OPEN") {
			int    port = (pb.Get("PORT", value) > 0) ? value.ToLong() : 0;
			int    timeout = (pb.Get("TIMEOUT", value) > 0) ? value.ToLong() : 0;
			THROW(Open(port, timeout) == 1);
		}
		else if(rCmd == "CLOSE") {
		}
		else if(rCmd == "INIT") {
		}
		else if(rCmd == "RELEASE") {
		}
		else { // ���� ���� �����������  �������, �� �������� �� ����
			DRVS.SetErrCode(serrInvCommand);
			err = 1;
		}
		CATCH
			err = 1;
			{
				SString msg_buf;
				DRVS.GetErrText(-1, value);
				DRVS.Log((msg_buf = "Wicked: error").CatDiv(':', 2).Cat(value), 0xffff);
			}
		ENDCATCH;
		return err;
	}
	//
	// Descr: ������������� ���������� � ������������. ����� ���� ��������� ��������, ���������, ������ �� �������, ��������� ��������
	// ARG(commNum	IN): ����� com-�����, � �������� ��������� ���������� (1..)
	// ARG(tm		IN): �����, �� ������� ��������� �������� (� ������������). �� ��� ��������� �������� ����� ���������
	// Returns:
	//		0 -
	//		1 -
	//
	int    Open(int commNum, int tm /*msec*/);
};

static PPDrvSession::TextTableEntry _ErrMsgTab[] = {
	{ K2ERR_OPENPORTFAILED,		"������ �������� �����" },
	{ K2ERR_WRITEPORTFAILED,    "������ ������ � ����" },
	{ K2ERR_CONTROLPORTFAILED,	"������ �������� ������������ ���� � ����" },
	{ K2ERR_MEMERR,				"������ ��������� ������" },
	{ K2ERR_GETADDRFAILED,		"������ ��������� ������ �����������" }
};

PPDRV_INSTANCE_ERRTAB(K2Controller, 1, 0, PPDrvWicket, _ErrMsgTab);

K2Controller::K2Controller()
{
	Handle	= INVALID_HANDLE_VALUE;
	//SerialN = BROADCAST_DEV_ADDR;
}

K2Controller::~K2Controller()
{
	ClosePort();
}

int K2Controller::OpenPort(int portNum)
{
	int    ok = 1;
	DCB    dcb;
	COMMTIMEOUTS timeouts;
	SString port_num;
	ClosePort();
	(port_num = "\\\\.\\COM").Cat(portNum /*+ 1*/);
	Handle = CreateFile(port_num, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	THROWERR(Handle != INVALID_HANDLE_VALUE, K2ERR_OPENPORTFAILED);
	//
	MEMSZERO(dcb);
	dcb.BaudRate = CBR_115200;
	dcb.StopBits = 1;
	SetCommState(Handle, &dcb);
	//
	// ��� �������� ��� ����, ����� �� �������� ��� ������ �� �����, ���� ������ ������ �� ����
	MEMSZERO(timeouts);
	timeouts.ReadIntervalTimeout = MAXDWORD; // -1;
	SetCommTimeouts(Handle, &timeouts);
	//
	CATCH
		ok = 0;
	ENDCATCH;
	return ok;
}

int K2Controller::ClosePort()
{
	if(Handle != INVALID_HANDLE_VALUE)
		CloseHandle(Handle);
	Handle = INVALID_HANDLE_VALUE;
	return 1;
}

int K2Controller::K2Read(K2FrameSt & rResp)
{
	int    ok = -1;
	SString msg_buf;
	uchar  ch = 0;
	DWORD  ret_size = 0;
	int    count_55 = 0, do_exit = 0;
	//
	// ���� ������������������ �� 10-�� ���� �� ��������� 0x55
	//
	while(!do_exit && ReadFile(Handle, &ch, 1, &ret_size, NULL) && ret_size) {
		if(ch == 0x55) {
			if(++count_55 == 10) {
				ReadFile(Handle, &ch, 1, &ret_size, 0); // ������� ��������������� ���� � ������ �� �����
				do_exit = 1;
			}
		}
		else
			count_55 = 0;
	}
	if(do_exit) {
		ReadFile(Handle, &rResp.H.Addr,    2, &ret_size, 0);
		ReadFile(Handle, &rResp.H.ReqNum,  1, &ret_size, 0);
		ReadFile(Handle, &rResp.H.Cmd,     1, &ret_size, 0);
		ReadFile(Handle, &rResp.H.DataLen, 1, &ret_size, 0);
		ReadFile(Handle, &rResp.H.Crc,     2, &ret_size, 0);
		ushort preserve_crc = rResp.H.Crc;
		rResp.H.CalcCrc();
		if(rResp.H.Crc == preserve_crc) {
			ok = 1;
			if(rResp.AllocBuf()) {
				ReadFile(Handle, rResp.P_Data, rResp.H.DataLen - CRC16_SZ, &ret_size, 0);
				ReadFile(Handle, &rResp.Crc, 2, &ret_size, 0);
				(msg_buf = "Wicked").CatDiv(':', 2).Cat("K2Read succs").CatDiv('-', 1);
				DRVS.Log(rResp.ToString(msg_buf), 0xffff); // @debug
			}
			else
				ok = 0;
		}
		else {
			(msg_buf = "Wicked").CatDiv(':', 2).Cat("K2Read error").CatDiv('-', 1).Cat("invalid CRC");
			DRVS.Log(msg_buf, 0xffff); // @debug
			ok = -2;
		}
	}
	//
	return ok;
}

uchar K2Controller::SendReq(uint16 addr, int cmd, const void * pData, size_t dataSize)
{
	static ACount __ReqNumSeq;

	uchar  req_num = 0;
	SString msg_buf;
	K2FrameSt frame_req;
	DWORD  flag = PURGE_TXCLEAR | PURGE_RXCLEAR;
	DWORD  ret_size = 0;
	char * p_allocated_wr_buf = 0;
	char   out_buffer[256];
	out_buffer[0] = 0;

	THROWERR(DeviceIoControl(Handle, IOCTL_SERIAL_PURGE, &flag, sizeof(flag), &out_buffer, sizeof(out_buffer), &ret_size, NULL), K2ERR_CONTROLPORTFAILED);
	ret_size = 0;
	//
	// ���������� ������
	//
	frame_req.H.Addr = addr; // @v8.2.8 SerialN-->addr
	frame_req.H.Cmd = cmd;
	__ReqNumSeq.Incr();
	if((__ReqNumSeq & 0xff) == 0)
		__ReqNumSeq.Incr();
	req_num = (uchar)(__ReqNumSeq % 256);
	frame_req.H.ReqNum = req_num;
	frame_req.H.DataLen = (uchar)(dataSize + CRC16_SZ);
	frame_req.H.CalcCrc();
	THROW(frame_req.AllocBuf());
	memcpy(frame_req.P_Data, pData, dataSize);
	frame_req.CalcCrc();
	ret_size = 0;
	THROWERR(DeviceIoControl(Handle, IOCTL_SERIAL_PURGE, &flag, sizeof(flag), &out_buffer, 256, &ret_size, NULL), K2ERR_CONTROLPORTFAILED);
	{
		size_t frame_len = 0;
		char * p_buf = 0;
		char   wr_buf[256];
		frame_req.GetBytes(NULL, frame_len);
		if(frame_len > sizeof(wr_buf)) {
			THROWERR(p_allocated_wr_buf = new char[frame_len], K2ERR_MEMERR);
			p_buf = p_allocated_wr_buf;
		}
		else
			p_buf = wr_buf;
		frame_req.GetBytes(p_buf, frame_len);
		WriteFile(Handle, p_buf, frame_len, &(ret_size = 0), 0);
		THROWERR(ret_size == frame_len, K2ERR_WRITEPORTFAILED);
		{
			(msg_buf = "Wicked").CatDiv(':', 2).Cat("SendReq succs").CatDiv('-', 1);
			DRVS.Log(frame_req.ToString(msg_buf), 0xffff); // @debug
		}
	}
	CATCH
		{
			(msg_buf = "Wicked").CatDiv(':', 2).Cat("SendReq error").CatDiv('-', 1);
			DRVS.Log(frame_req.ToString(msg_buf), 0xffff); // @debug
		}
		req_num = 0;
	ENDCATCH;
	// @v8.2.8 (done by destructor) ZDELETE(frame_req.P_Data);
	if(p_allocated_wr_buf)
		delete p_allocated_wr_buf;
	return req_num;
}

int K2Controller::WaitEvent(uint16 addr, int timeout, int evtCode, ...)
{
	int    r = 0;
	const  ulong t1 = GetTickCount();
	// ������ ���� ������ ���-�� �������� � ���� �� ������ evtCode
	for(int do_exit = 0; !r && !do_exit;) {
		K2FrameSt frame_resp;
		const int rr = K2Read(frame_resp);
		if(rr > 0) {
			if(frame_resp.H.Cmd == K2CMD_ONLINE_TASK) {
				//
				// ��������� ���������
				//
				K2OnlineEventSt * p_online_event = (K2OnlineEventSt *)frame_resp.P_Data;
				for(const int * p_evt_code = &evtCode; !r && *p_evt_code; p_evt_code++) {
					if(p_online_event->Code == *p_evt_code) {
						r = 1;
					}
				}
				uchar rn = SendReq(addr, K2CMD_STATUS_READ, &frame_resp.H.ReqNum, sizeof(frame_resp.H.ReqNum));
			}
		}
		else if(rr < 0) {
			Sleep(1);
			if((GetTickCount() - t1) > (uint)timeout)
				do_exit = 1;
		}
	}
	return r;
}

uchar K2Controller::Reset_(uint16 addr)
{
	return SendReq(addr, K2CMD_RESET, 0, 0);
}

int K2Controller::GetAddr(uint16 & rAddr)
{
	rAddr = 0;

	int    ok = 1;
	uchar  rn = 0;
	int    reserved_addr = 0; // ���� ������� ���� �����-�� �����, �� �� ���, ��� �����, �� �� "���� ������-��" ������ ��������� ����� ����������
	THROW(rn = SendReq(0xffff, K2CMD_DEV_ADDR, 0, 0));
	Sleep(20); // @v8.2.8 100-->20
	const  ulong t1 = GetTickCount();
	int    rr = 0;
	do {
		K2FrameSt frame;
		THROW(rr = K2Read(frame));
		if(frame.H.ReqNum == rn && frame.H.Cmd == 2) {
			//
			// ��������� ������ �����������
			//
			struct K2AddrSt {
				ushort	SerialNumber;	// �������� ����� ����������� (�� �� �����)
				uchar	Type;			// ��� �����������
				uchar	Version;		// ������ �����������
			};
			const K2AddrSt * p_addr = (const K2AddrSt *)frame.P_Data;
			rAddr = p_addr->SerialNumber;
			rr = 1;
		}
		else {
			SETIFZ(reserved_addr, frame.H.Addr);
			if((GetTickCount() - t1) > (uint)100) {
				rr = -1;
				rAddr = reserved_addr;
				ok = -1;
				{
					SString msg_buf;
					(msg_buf = "Wicked").CatDiv(':', 2).CatEq("GetAddr returns reserved addr", (long)rAddr);
					DRVS.Log(msg_buf, 0xffff); // @debug
				}
			}
			else {
				Sleep(1);
				rr = 0;
			}
		}
	} while(rr == 0);
	CATCH
		ok = 0;
	ENDCATCH;
	return ok;
}

int K2Controller::ActivateLine(uint16 addr, int line, int tm)
{
	//
	// ��������� ��� ������� ������� ���������� �������� (0x0D)
	//
	struct K2OutputCtrlDataSt {
		K2OutputCtrlDataSt(int line, int tm)
		{
			LineNumber = line;
			Line2 = 0;
			Line3 = 0;
			Line4 = 0;
			ExecMech = 0;
			Line1_2 = 0;
			Line1_3 = 0;
			Line1_4 = 0;
			ExecMech1 = 0;
			switch(LineNumber) {
				case OUTPUT_CTRL_LINE2:      Line2 = tm; break;
				case OUTPUT_CTRL_LINE3:      Line3 = tm; break;
				case OUTPUT_CTRL_LINE4:      Line4 = tm; break;
				case OUTPUT_CTRL_EXEC_MECH:  ExecMech = tm; break;
				case OUTPUT_CTRL_LINE1_2:    Line1_2 = tm; break;
				case OUTPUT_CTRL_LINE1_3:    Line1_3 = tm; break;
				case OUTPUT_CTRL_LINE1_4:    Line1_4 = tm; break;
				case OUTPUT_CTRL_EXEC_MECH1: ExecMech1 = tm; break;
			}
		}
		uchar	LineNumber;			// ������ �����, � �������� �������������� ��������
		int32	Line2;				// "STOP" ��� "HOLD", ��� �� ������������
		int32	Line3;				// "STOP" ��� "HOLD", ��� �� ������������
		int32	Line4;				// "STOP" ��� "HOLD", ��� �� ������������
		int32	ExecMech;			// ���������� �� (�������������� ��������)
		int32	Line1_2;			// "STOP" ��� "HOLD", ��� �� ������������
		int32	Line1_3;			// "STOP" ��� "HOLD", ��� �� ������������
		int32	Line1_4;			// "STOP" ��� "HOLD", ��� �� ������������
		int32	ExecMech1;			// ���������� ��1 (�������������� ��������)
	};
	int    ok = 1;
	K2OutputCtrlDataSt data(line, tm);
	uchar rn = SendReq(addr, K2CMD_OUTPUT_CTRL, &data, sizeof(data));
	THROW(rn);
	{
		K2FrameSt frame;
		Sleep(20);
		int rr = K2Read(frame); // �������� ������������� �� �����������
		if(rr && frame.H.ReqNum == rn && frame.H.Cmd == 2) {
		}
		else {
			SString msg_buf;
			(msg_buf = "Wicked").CatDiv(':', 2).Cat("ActivateLine not acknowledged");
			DRVS.Log(msg_buf, 0xffff); // @debug
		}
	}
	CATCH
		ok = 0;
	ENDCATCH
	return ok;
}

int PPDrvWicket::Open(int commNum, int tm)
{
	int    r = 0;
	const  ulong fn_exec_start = GetTickCount();
	SString msg_buf;
	const int pass_timeout = (tm > 0) ? tm : 10000;
	DRVS.Log((msg_buf = "Wicked: open() executed").CatDiv(':', 2).Cat(commNum), 0xffff);
	THROW(OpenPort(commNum));
	// @v8.2.8 while(WaitEvent(100, ONLINE_EVENT_CONNECTION_FAILED, ONLINE_EVENT_CONNECTION_RECOVER, ONLINE_EVENT_IN, 0));
	// @v8.2.8 {
	/*
	uint   skip_event_count = 0;
	int    skip_event_timeout = 50;
	{
		int    skip_event_result = 0;
		do {
			skip_event_result = WaitEvent(0xffff, skip_event_timeout, ONLINE_EVENT_CONNECTION_FAILED, ONLINE_EVENT_CONNECTION_RECOVER, ONLINE_EVENT_IN, 0);
			if(skip_event_result) {
				skip_event_timeout = 50;
				skip_event_count++;
			}
		} while(skip_event_result);
	}
	*/
	// } @v8.2.8
	{
		uint16 addr = 0;
		THROWERR(GetAddr(addr), K2ERR_GETADDRFAILED);
		THROW(ActivateLine(addr, OUTPUT_CTRL_EXEC_MECH1, /*0xFFFF*/ (pass_timeout / 50)));  // 0xffff - ������� ��������
		Sleep(20); // @v8.2.8
		const ulong open_tm = GetTickCount();
		r = WaitEvent(addr, pass_timeout, ONLINE_EVENT_IN, 0);
		if(r > 0) {
			//
			// ��� ��������� �������� �������� ����� ������� �������:
			// 1. (__rest_pass_timeout  > 0): ������������� ������� ����� ��������� �������
			// 2. (__rest_pass_timeout == 0): ������� ����� ����������� ������� �� �������������� �������� __total_pass_timeout
			//
			const long __total_pass_timeout = 2000;
			const long __rest_pass_timeout = 3000; // @v8.2.8 1000-->3000
			long  pass_timeout_spend = GetTickCount() - open_tm;
			if(__rest_pass_timeout > 0) {
				Sleep(__rest_pass_timeout);
			}
			else {
				if(pass_timeout_spend < __total_pass_timeout) {
					Sleep(__total_pass_timeout - pass_timeout_spend);
				}
			}
			THROW(ActivateLine(addr, OUTPUT_CTRL_EXEC_MECH1, 1));
			DRVS.Log((msg_buf = "Wicked: closed by pass").CatDiv('-', 1).Cat(commNum).Space().CatParStr(pass_timeout_spend), 0xffff);
		}
		else {
			DRVS.Log((msg_buf = "Wicked: closed by timeout").CatDiv('-', 1).Cat(commNum), 0xffff);
		}
	}
	CATCH
		r = -1;
		DRVS.Log((msg_buf = "Wicked: error opening").CatDiv('-', 1).Cat(commNum), 0xffff);
	ENDCATCH;
	ClosePort();
	DRVS.Log((msg_buf = 0).CatEq("Wicked: open exec time", GetTickCount() - fn_exec_start).Space().Cat("ms"), 0xffff);
	return r;
}
//
//EXPORT int Open(int commNum, int tm)
//{
//	Wicket wicket;
//	//wicket.LogFile.Open("reverse_k2.log", SFile::mAppend);
//	return wicket.Open(commNum, tm);
//}
//
//EXPORT int GetLastErr(char * pErrBuf, int bufSize)
//{
//	if(pErrBuf) {
//		for(uint i = 0; i < SIZEOFARRAY(ErrorMsg); i++) {
//			if(ErrorMsg[i].Id == LastErrorCode) {
//				memcpy(pErrBuf, ErrorMsg[i].P_Msg, strlen(ErrorMsg[i].P_Msg) < bufSize ? strlen(ErrorMsg[i].P_Msg) : bufSize);
//			}
//		}
//	}
//	return 1;
//}
