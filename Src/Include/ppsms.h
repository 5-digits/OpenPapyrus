// PPSMS.H
//
#ifndef __PPSMS_H
#define __PPSMS_H

#if 0 // {

class SmsProtocolBuf;

struct StConfig {
	StConfig();
	void   Clear();
	int    SetConfig_(const char * pHost, uint port, const char * pSystemId, const char * pLogin,
		const char * pPassword, const char * pSystemType, uint sourceAddressTon, uint sourceAddressNpi,
		uint destAddressTon, uint destAddressNpi, uint dataCoding, const char * pFrom, uint splitLongMsg);

	SString Host;
	uint   Port;
	SString SystemId; // ������������� ������� � �������
	SString Login;
	SString Password;
	SString SystemType; // ��� �������
	SString AddressRange; // ������ ������� �����������
	uint   SourceAddressTon; // ��� ������ �����������
	uint   SourceAddressNpi; //�������� ��������� ����� �����������
	uint   DestAddressTon; // ��� ������ ����������
	uint   DestAddressNpi; // �������� ��������� ����� ����������
	uint   DataCoding; // ����� ����������� ���������
	uint   EsmClass; // ��� ��������� � ����� �������� ���������
	SString From; // ��� ��� ����� �����������
	uint   SplitLongMsg; // ��������� ��� ��� ������� ���������
	uint16 ResponseTimeout;
	uint16 ResendMsgQueueTimeout;
	uint16 ResendTriesNum;
	uint16 ReconnectTimeout;
	uint16 ReconnectTriesNum;
};

class SmsClient {
public:
	struct StSubmitSMParam { // ��������� ���������� ������� submit_sm
		StSubmitSMParam();
		void   Clear();

		uchar  SourceAddressTon;
		uchar  SourceAddressNpi;
		uchar  DestinationAddressTon;
		uchar  DestinationAddressNpi;
		uchar  EsmClass;
		uchar  ProtocolId;
		uchar  PriorityFlag;
		uchar  ReplaceIfPresentFlag;
		uchar  DataCoding;
		uchar  SmDefaultMsgId;
		uint8  Reserve[2]; // @alignment
		SString SourceAddress;
		SString DestinationAddress;
		SString SheduleDeliveryTime;
		SString ValidityPeriod;
	};
	SmsClient();
	~SmsClient();
	int    CanSend() const;
	int    IsConnected() const;
	int    Connect(const StConfig & config);
    int    Disconnect();
	//
	// Descr: ��������� ����� �������� � �������� ��������� ��������� �� ������� ��������� StatusCodesArr
	//
	int    GetStatusCode(SString & rDestNum, SString & rStatus, size_t pos) const;
	//
	// Descr: ��������� �� ������� ErrorSubmitArr ����� �������� � �������� ������ ������������� ���������
	//
	int    GetErrorSubmit(SString & rDestNum, SString & rErrText, size_t pos) const;
	//
	// Descr: �������� ���������� ������ �� ����, ���� ��� ����
	//
	int    GetRestOfReceives();
	int    SendSms(const char * pTo, const char * pText, SString & rStatus);
	//
	// @v8.5.4 void   SetRecvTimeout(int timeout) { RecvTimeout = timeout; }

	PPLogger Logger;
private:
	struct StSMResults {
		StSMResults();
		int    GetResult(int kindOfResult);

		int    BindResult;
		int    UnbindResult;
		int    SubmitResult;
		int    DataResult;
		int    EnquireLinkResult;
		int    GenericNackResult;
	};
	void   SetConfig(const StConfig & rConfig)
	{
		Config = rConfig;
	}
	//
	// Descr: ��������� ���������� �� ��������� �������� ������� ��� � ������ ErrorSubmitArr
	//
	void   AddErrorSubmit(const char * pDestNum, int errCode);
	void   DecodeDeliverSm(int sequenceNumber, void * pPduBody, size_t bodyLength);
	void   DisconnectSocket();
	//
	// Descr: ��������� ������ ��� � ������ StatusCodesArr
	//
	void   AddStatusCode(const char * pDestNum, int statusCode, const char * pError);
	int    SendSms_(const char * pFrom, const char * pTo, const char * pText);
	int    Send(const void * data, size_t bufLen);
	int    Send(const SmsProtocolBuf & rBuf, int tryReconnect);
	int    Bind();
	int    Unbind();
	int    SubmitSM(const StSubmitSMParam & rParam, const char * pMessage, bool payloadMessage);
	int    ConnectToSMSC();
	int    TryToReconnect(uint & rRecconectionCount);
	int    SendEnquireLink(int sequenceNumber);
	int    SendEnquireLinkResp(int sequenceNnumber);
	int    SendGenericNack(int sequenceNumber, int commandStatus);
	int    SendDeliverSmResp(int sequenceNumber);
	int    Receive(uint timeout);
	//
	// Descr: ���� USE_ENQUIRELINK = true, �� �������� ������� �� �������� ����� ����� ������ ���������� ������� ENQUIRE_LINK_TIMEOUT
	//
	int    ReceiveToTimer();

	TcpSocket ClientSocket;
	LDATETIME StartTime;
	StConfig Config;
	StSMResults SMResults;
	StSubmitSMParam SMParams;
	//
	// ������ � ����������� �� ��������� �������� ������� ���. �������� ������: �����_����������;��������_������
	//
	StrAssocArray ErrorSubmitArr;
	//
	// ������ �������� ��������� ���������, � ��� ����� � ���������. �������� ������: �����_����������;���������_���������
	//
	StrAssocArray StatusCodesArr;
	int    ConnectionState;
	uint   ResendErrLenMsg; // ������� ������� ������������� ��������� ��� �������� ��� �����
	uint   ReSendQueueMsgTryNums; // ������� ������� ������������� ���, ������� �� ���� ���������� ��-�� ������������ �������
	uint   MessageCount; // ������� �������� �������� �������� ��������� (submit_sm)
	uint   SequenceNumber; // ����� ������
	uint   AddStatusCodeNum; // ������� ��������� ������� StatusCodesArr
	uint   AddErrorSubmitNum; // ������� ���������� ������� ErrorSubmitArr
	uint   UndeliverableMessages; // ������� �������������� ���������
	// @v8.5.4 int    RecvTimeout;  // ������� ��������� ������
};

#endif // } 0

#endif // __PPSMS_H
