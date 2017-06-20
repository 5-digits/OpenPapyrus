// TV.H
// Win32
//
#ifndef __TV_H
#define __TV_H
#define _TURBOVISION
#define USE_CAIRO

class  TView;
class  TGroup;
class  TWindow;
class  TDialog;
class  TButton;
class  TLabel;
class  ComboBox;
class  TBaseBrowserWindow;
class  TCanvas;
class  TCanvas2;
class  WordSelector;
class  SrDatabase; // @v9.2.0
struct TDrawItemData;
class  TWhatman;

#include <slib.h>
#include <tvdefs.h>
#include <commctrl.h>
#include <db.h>

// @v9.2.0 typedef int ccIndex;
// @v9.2.0 typedef bool (*ccTestFunc)(void *, void *);
// @v9.2.0 typedef void (*ccAppFunc)(void *, void *);
// @v9.2.0 const int ccNotFound = -1;
// @v9.2.0 TRect SLAPI makeCenterRect(int w, int h);
//
//
//
class WordSel_ExtraBlock {
public:
	WordSel_ExtraBlock();
	WordSel_ExtraBlock(uint inputCtl, HWND hInputDlg, TDialog * pOutDlg, uint outCtlId, uint minSymbCount, long flags = 0);
	virtual ~WordSel_ExtraBlock();
	void   Init(uint inputCtl, HWND hInputDlg, TDialog * pOutDlg, uint outCtlId, uint minSymbCount, long flags = 0);
	virtual int Search(long id, SString & rBuf);
	//
	// Descr: ��������� ���������������� ���������� ������ �� ���������� ����� pText.
	//   ���� ������� �� ��������������, �� ������ ������� -1.
	//   ���� ������� ��������������, �� ��� ������ ����� ������������ ����� pText �, ���� �������
	//   ����������, ���������� ��������������� �������� *pID � rBuf � ������� >0.
	//   ���� ������� ��������������, � �������� �� ����� pText �� �������, �� ������ ������� 0.
	//
	virtual int SearchText(const char * pText, long * pID, SString & rBuf);
	virtual StrAssocArray * GetList(const char * pText);
	long   GetFlags() const;
	boolean IsTextMode() const;
	void   SetTextMode(boolean v);
	void   SetData(long id, const char * pText);
	//
	// Descr: ���������� ����� ������ �� id, ����� �������� �������� SetData
	//
	void   SetupData(long id);
	int    GetData(long * pId, SString & rBuf);

	enum {
		fAlwaysSearchBySubStr = 0x0001
	};
//protected:
	long   Flags;
	boolean CtrlTextMode; // ���� true, �� ��� ������ ������� TransmitData, ������ � ������� ���������� �����
		// ��������������� � ������������ � ���������� �������� ����� ��������.
		// �����, ������� ����� ������� ��� �� �������� id � ���������� ���������� ����� ������ ��������������� ����� id.
	uint   MinSymbCount;
	uint   InputCtl;
	HWND   H_InputDlg;
	long   SelId;
	uint   OutCtlId;
	TDialog * P_OutDlg;
};
//
// Event codes
//
enum {
	evMouseDown = 0x0001,
	evMouseUp   = 0x0002,
	evMouseMove = 0x0004,
	evMouseAuto = 0x0008,
	evKeyDown   = 0x0010,
	evCommand   = 0x0100,
	evBroadcast = 0x0200,
	evWinCmd    = 0x0400, // @v9.6.5 @construction
	//
	// Event masks
	//
	evNothing   = 0x0000,
	evMouse     = 0x000f,
	evKeyboard  = 0x0010,
	evMessage   = 0xFF00
};
//
// Mouse button state masks
//
enum {
	mbLeftButton  = 0x01,
	mbRightButton = 0x02
};

struct KeyDownCommand { // @size=4
	enum {
		stateAlt   = 0x0001,
		stateCtrl  = 0x0002,
		stateShift = 0x0004
	};
	//SLAPI  KeyDownCommand();
	// @noconstructor
	void   Clear()
	{
		State = 0;
		Code = 0;
	}
	int    FASTCALL operator == (const KeyDownCommand & rS) const
	{
		return (State == rS.State && Code == rS.Code);
	}
	operator long() const
	{
		return *(long *)this;
	}
	int    SLAPI GetKeyName(SString & rBuf, int onlySpecKeys = 0) const;
	int    SLAPI SetKeyName(const char * pStr, uint * pLen);
	int    FASTCALL SetWinMsgCode(uint32 wParam);
	int    FASTCALL SetTvKeyCode(uint16 tvKeyCode);
	//
	// Descr: ����������� ������ chr �� ���������� ��������� ������� ���������� ������ �
	//   ������������ � ������� ���������� ����������.
	// Note: ��������� ���������� ������ ������������� �������� � ����-���.
	// Returns:
	//   !0 - ������� ��������� ������� (���������� ��������� ����������)
	//    0 - ������ (���������� ��������� �� ����������)
	//
	int    FASTCALL SetChar(uint chr);
	//
	// Descr: ����������� ��� ����������� ������� this->Code � ������.
	// Note: ������� "�����" - �������������� ������ ��� ��������� � ����������� ASCII ��������.
	// Returns:
	//   !0 - ��� �������, ��������������� ����������� �������
	//   0  - ������� ������ ������������� � ������
	//
	uint   SLAPI GetChar() const;

	uint16 State;
	uint16 Code;
};
//
// Descr: ���������, ������������ � ���������� cmSetFont
//
struct SetFontEvent {
	long   FontHandle;
	int    DoRedraw;
};

struct HelpEvent {
	enum {
		ctxtMenu = 1,
		ctxtWindow
	};
	int    ContextType; // ctxtXXX
	int    CtlId;
	long   H_Item;
	uint32 ContextId;
	TPoint Mouse;
};
//
// Descr: ���������, ������������ � ���������� cmSize.
//
struct SizeEvent {
	enum {
		tMaxHide = 1,
		tMaximized,
		tMaxShow,
		tMinimized,
		tRestored
	};
	int    ResizeType; // tXXX ��� ������� ��������� �������.
	TPoint PrevSize;   // ������� ���� �� ��������� ����� ��������� //
	TPoint NewSize;    // ����� ������� ����.
};
//
// Descr: ���������, ������������ � ���������� cmPaint.
//
struct PaintEvent {
	enum {
		tPaint = 1,
		tNcPaint,
		tEraseBackground
	};
	enum {
		fErase = 0x0001
	};
	int    PaintType;
	long   H_DeviceContext; // ��� PaintType == tPaint ���� �������� �������.
		// HANDLE_EVENT ������ �������������� �������� �������� ��������� � ����.
	long   Flags;
	TRect  Rect;
};
//
// Descr: ���������, ������������ � ���������� cmMouse
//
struct MouseEvent {
	enum {
		tLDown = 1,  // ������ ����� ������ ����
		tLUp,        // �������� ����� ������ ����
		tLDblClk,    // ������� ������ �� ����� ������ ����
		tRDown,      // ������ ������ ������ ����
		tRUp,        // �������� ������ ������ ����
		tRDblClk,    // ������� ������ �� ������ ������ ����
		tMDown,      // ������ ������� ������ ���� (������)
		tMUp,        // �������� ������� ������ ���� (������)
		tMDblClk,    // ������� ������ �� ������� ������ ���� (������)
		tMove,       // ������ ���� ������������ //
		tWeel,       // �������� ������ ���� //
		tHover,      // ������ ���� ����� �� ����� ����� � ������� ��������� �������
		tLeave       // ������ ���� ����� �� ������� ���������� ������� ����
	};
	enum {
		fLeft      = 0x0001,
		fRight     = 0x0002,
		fMiddle    = 0x0004,
		fShift     = 0x0008,
		fControl   = 0x0010,
		fX1        = 0x0020,
		fX2        = 0x0040,
		fDrag      = 0x0080
	};
	int    Type;
	int    Flags;
	int    WeelDelta;
	TPoint Coord;
};
//
// Descr: ���������, ������������ � ���������� cmScroll
//
struct ScrollEvent {
	enum {
		tBottom = 1,
		tTop,
		tEnd,
		tLineDown,
		tLineUp,
		tPageDown,
		tPageUp,
		tThumbPos,
		tThumbTrack
	};
	int    Dir;        // DIREC_HORZ || DIREC_VERT
	int    Type;
	IntRange Range;
	uint   PageSize;
	int    Pos;
	int    TrackPos;
	long   H_Wnd;
};
//
// Descr: ���������, ������������ � ���������� cmDragndropObj
//
struct DragndropEvent {
	enum {
		acnGet = 1, // ���� �������� ������ � ��������������� ��������
		acnAccept,  // ������������ ������� ������ � ���� (�������� ����� ������ ���� � ����-���������)
		acnLeave    // ������ � ��������������� �������� ���� �� ������� ����-���������.
	};
	int    Action;
};

struct TEvent {
	struct Mouse {
		uchar  buttons;
		int8   doubleClick;
		int16  WhereX;
		int16  WhereY;
		//TPoint where;
	};
	struct KeyDown {
		struct Scan {
			uchar  charCode;
			uchar  scanCode;
		};
		union {
			ushort keyCode;
			Scan   charScan;
		};
		KeyDownCommand K;
	};
	struct Message {
		uint   command;
		union {
			void  * infoPtr;
			TView * infoView;
			LPARAM  LP;
		};
		union {
			long    infoLong;
			ushort  infoWord;
			short   infoInt;
			uchar   infoByte;
			char    infoChar;
			WPARAM  WP;
		};
	};
	uint   what;
	union {
		Mouse mouse;
		KeyDown keyDown;
		Message message;
	};

	TEvent();
	TEvent & setCmd(uint msg, TView * pInfoView);
	uint getCtlID() const;
	int  FASTCALL isCmd(uint cmd) const;
	int  FASTCALL isKeyDown(uint keyCode) const;
	int  FASTCALL isCtlEvent(uint ctlID) const;
	int  FASTCALL isCbSelected(uint ctlID) const;
	int  FASTCALL isClusterClk(uint ctlID) const;
	int  FASTCALL wasFocusChanged(uint ctlID) const;
	int  SLAPI wasFocusChanged2(uint ctl01, uint ctl02) const;
	int  SLAPI wasFocusChanged3(uint ctl01, uint ctl02, uint ctl03) const;
};

#define TVEVENT     event.what
#define TVCOMMAND   (event.what == evCommand)
#define TVBROADCAST (event.what == evBroadcast)
#define TVKEYDOWN   (event.what == evKeyDown)
#define TVCMD       event.message.command
#define TVKEY       event.keyDown.keyCode
#define TVCHR       event.keyDown.charScan.charCode
#define TVSCN       event.keyDown.charScan.scanCode
#define TVINFOVIEW  event.message.infoView
#define TVINFOPTR   event.message.infoPtr
//
//
//
class TCommandSet {
public:
	TCommandSet();
	TCommandSet(const TCommandSet&);
	int    IsEmpty() const;
	int    has(int cmd) const;
	void   enableAll();
	void   enableCmd(int cmd, int is_enable);
	void   enableCmd(const TCommandSet&, int is_enable);
	void   operator += (int cmd);
	void   operator -= (int cmd);
	void   operator += (const TCommandSet&);
	void   operator -= (const TCommandSet&);
	TCommandSet & operator &= (const TCommandSet&);
	TCommandSet & operator |= (const TCommandSet&);
	friend TCommandSet operator & (const TCommandSet&, const TCommandSet&);
	friend TCommandSet operator | (const TCommandSet&, const TCommandSet&);
	friend int operator == (const TCommandSet& tc1, const TCommandSet& tc2);
	friend int operator != (const TCommandSet& tc1, const TCommandSet& tc2);
private:
	int    loc(int);
	int    mask(int);
	uint32 cmds[64];
};

int operator != (const TCommandSet& tc1, const TCommandSet& tc2);

// regex: (virtual)*[ \t]+void[ \t]+handleEvent\([ \t]*TEvent[ \t]*&[^)]*\)
#define DECL_HANDLE_EVENT      virtual void __fastcall handleEvent(TEvent & event)
// regex: void[ \t]+([a-zA-Z0-9_]+)::handleEvent\([ \t]*TEvent[ \t]*&[^)]*\)
#define IMPL_HANDLE_EVENT(cls) void __fastcall cls::handleEvent(TEvent & event)
#define EVENT_BARRIER(f) if(!EventBarrier()) { f; EventBarrier(1); }
//
// Descr: ��������� ��� GDI-��������. ������ ���, ��� "�����������" ���������� ��������������
// ��������� ��������� ����� ��������� � ���� ���������, � ��� ��������� ��������������
// �������������, ������� �������� ���������� � ������� SPaintToolBox::Get() ��� ��������� //
// ���������� GDI-��������.
// �������� � ���������� �������� ������������, ��-�����, ����� ���������� ����� ����.
// ��� ���������� ���������� ��� GDI-�������, ���������� � ���� �����������.
//
#ifdef _WIN32_WCE
#define PS_DOT           3
#define PS_DASHDOT       4
#define PS_DASHDOTDOT    5
#define PS_INSIDEFRAME   7

#define BS_HATCHED       3
#define BS_DIBPATTERN    5
#define BS_DIBPATTERNPT  6

#define HS_HORIZONTAL    1
#define HS_VERTICAL      2
#define HS_FDIAGONAL     3
#define HS_BDIAGONAL     4
#define HS_CROSS         5
#define HS_DIAGCROSS     6
#endif // } _WIN32_WCE
//
// Descr: ������������ ������
//
class SFontDescr { // @persistent
public:
	enum {
		fItalic    = 0x0001,
		fUnderline = 0x0002,
		fStrikeOut = 0x0004,
		fBold      = 0x0008, // ���� Weight == 0.0f � Flags & fBold, �� ������ ����������� ��������
			// Weight �����, ����� ����� �������� ����������. ���� Weight > 0.0, �� ���� ���� ������������.
		fAntialias = 0x0010
	};

	SFontDescr(const char * pFace, int size, int flags);
	void   Init();
	int    FASTCALL IsEqual(const SFontDescr & rS) const;
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	int    ToStr(SString & rBuf, long fmt) const;
	int    FASTCALL FromStr(const char *);
	int    FASTCALL SetLogFont(const LOGFONTA * pLf);
	int    FASTCALL SetLogFont(const LOGFONTW * pLf);
	LOGFONTA * FASTCALL MakeLogFont(LOGFONTA * pLf) const;
	LOGFONTW * FASTCALL MakeLogFont(LOGFONTW * pLf) const;

	int16  Size;        // @anchor ���������� ������ ������ � ��������
	int16  Flags;       // @flags SFontDescr::fXXX
	float  Weight;      // ������� ������. 0.0f - �� �����, 1.0f - ����������, 2.0f - ����������� �������.
	uint8  CharSet;     // ����� �������� XXX_CHARSET (win32)
	uint8  Reserve[15];
	SString Face;       // @anchor
private:
	int    FASTCALL Helper_SetLogFont(const void * pLf);
	int    FASTCALL Helper_MakeLogFont(void * pLf) const;
};
//
// Descr: ������������ ���������� ���������
//
struct SParaDescr { // @persistent
	SParaDescr();
	int    FASTCALL IsEqual(const SParaDescr &) const;
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	int    GetJustif() const;

	enum {
		fJustRight   = 0x0001, // ����������� ������ �� ������� ����
		fJustCenter  = 0x0002  // ����������� ������ �� ������
	};
	TPoint LuIndent;    // ����� � ������� �������
	TPoint RlIndent;    // ������ � ������ �������
	int16  StartIndent; // ������ ������ ������
	int16  Spacing;     // ����������� (���������� ����� ��������)
	int32  Flags;
	uint8  Reserve[16];
};
//
// Descr: ������������ ������� � ������.
//
struct SGlyph {
	uint16 Chr;     // Unicode-������
	int16  Idx;     // ������ ���������� ������� � ������
	FPoint Sz;      // Specifies the size of the smallest rectangle that completely encloses the glyph (its black box).
	FPoint Org;     // Specifies coordinates of the upper left corner of the smallest rectangle that completely encloses the glyph.
	FPoint Advance; // Specifies the distance from the origin of the current character cell to the origin of the next character cell.
	float  LineAdv; // ��������, �� ������� ���������� ������������� ��� �������� �� ����� ������.
		// �������� ����������� �� ������� ������.
};
//
// Descr: ������������� ����������� ���������. ������������ ��������������� ��������
//   �������� � ������������ ����������� ������������, ���� ������� ������������ �����
//   SPaintObj::T (enum SPaintObj::tXXX).
//
class SPaintObj { // @persistent @store(SSerializeContext)
public:
	static const float DefaultMiterLimit; // 4.0f
	//
	// Descr: ���� �������
	//
	enum {
		tUndef = 0, // �� ���������
		tPen,       // ����
		tBrush,     // �����
		tColor,     // ����
		tCursor,    // ������
		tBitmap,    // ������� �����
		tFont,      // �����
		tGradient,  // ��������
		tParagraph, // ��������� ��������
		tCStyle     // ����� ��������� ��������
	};
	//
	// Descr: ����� ������ (PEN)
	//
	enum {
		psSolid       = PS_SOLID,      // The pen is solid. (==0)
		psDash        = PS_DASH,       // The pen is dashed.
			// This style is valid only when the pen width is one or less in device units.
		psDot         = PS_DOT,        // The pen is dotted.
			// This style is valid only when the pen width is one or less in device units.
		psDashDot     = PS_DASHDOT,    // The pen has alternating dashes and dots.
			// This style is valid only when the pen width is one or less in device units.
		psDashDotDot  = PS_DASHDOTDOT, // The pen has alternating dashes and double dots.
			// This style is valid only when the pen width is one or less in device units.
		psNull        = PS_NULL,       // The pen is invisible.
		psInsideFrame = PS_INSIDEFRAME // The pen is solid. When this pen is used in any GDI drawing
			// function that takes a bounding rectangle, the dimensions of the figure are shrunk so that
			// it fits entirely in the bounding rectangle, taking into account the width of the pen.
			// This applies only to geometric pens.
	};
	//
	// Descr: ����� ������ (BRUSH)
	//
	enum {
		bsSolid        = BS_SOLID,       // Solid brush
		bsNull         = BS_NULL,        // Hollow brush
		bsHatched      = BS_HATCHED,     // Hatched brush
		bsPattern      = BS_PATTERN,     // Pattern brush defined by a memory bitmap
		bsDibPattern   = BS_DIBPATTERN,  // A pattern brush defined by a device-independent bitmap (DIB) specification.
			// If lbStyle is BS_DIBPATTERN, the lbHatch member contains a handle to a packed DIB.
		bsDibPatternPt = BS_DIBPATTERNPT // A pattern brush defined by a device-independent bitmap (DIB) specification.
			// If lbStyle is BS_DIBPATTERNPT, the lbHatch member contains a pointer to a packed DIB.
	};
	//
	// Descr: ����� ��������� ����� (line cap)
	//
	enum {
		lcButt = 1,
		lcRound,
		lcSquare
	};
	//
	// Descr: ��� ���������� ����� (line join)
	//
	enum {
		ljMiter = 1,
		ljRound,
		ljBevel
	};
	//
	// Descr: ����� ��������� ������ (BRUSH)
	//
	enum {
		bhsHorz      = HS_HORIZONTAL, // Horizontal hatch                       ----
		bhsVert      = HS_VERTICAL,   // Vertical hatch                         ||||
		bhsFDiagonal = HS_FDIAGONAL,  // 45-degree downward left-to-right hatch (\\\\)
		bhsBDiagonal = HS_BDIAGONAL,  // 45-degree upward left-to-right hatch   ////
		bhsCross     = HS_CROSS,      // Horizontal and vertical crosshatch     ++++
		bhsDiagCross = HS_DIAGCROSS,  // 45-degree crosshatch                   xxxx
	};
	//
	// Descr: ������� �������
	//
	enum {
		frNonZero = 1,
		frEvenOdd
	};

	class Base { // @persistent @store(SSerializeContext)
	public:
		friend int FASTCALL _SetPaintObjInnerHandle(SPaintObj::Base * pBase, SDrawSystem sys, void * h);

		Base();
		void * GetHandle() const
		{
			return Handle;
		}
		SDrawSystem GetSys() const
		{
			return Sys;
		}
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	private:
		void * Handle;
		SDrawSystem Sys;
	};
	//
	// Descr: ����������� ������������� ����
	//
	class Pen : public Base { // @persistent @store(SSerializeContext)
	public:
		Pen();
		~Pen();
		Pen  & FASTCALL operator = (const Pen & rS);
		int    FASTCALL Copy(const Pen & rS);
		int    FASTCALL IsEqual(const Pen & rS) const;
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
		int    IsDashed() const;
		//
		// Descr: ���������� !0, ���� ���� �������� ��������� �������,
		//   �� ���� ����� ���� ���������� ������ ������.
		//   ��������� ��������� ���� ������ ���� ����������:
		//   W = 1.0
		//   S = SPaintObj::psSolid
		//   LineCap = SPaintObj::lcButt || 0
		//   Join = SPaintObj::ljMiter || 0
		//   MiterLimit = 4.0 || 0
		//   P_DashRule = 0
		//
		int    IsSimple() const;
		int    FASTCALL SetSimple(SColor);
		int    AddDashItem(float f);

		SColor C;          // ����
		float  W;          // ������
		int32  S;          // ����� (SPaintObj::psXXX)
		int8   LineCap;    // SPaintObj::lcXXX
		int8   Join;       // SPaintObj::ljXXX
		uint16 Reserve;    // @alignment
		float  MiterLimit; //
		float  DashOffs;   //
		FloatArray * P_DashRule;
	};
	//
	// Descr: ����������� ������������� �����
	//
	class Brush : public Base { // @persistent @store(SSerializeContext)
	public:
		Brush();
		Brush & FASTCALL operator = (const Brush &);
		int    FASTCALL operator == (const Brush &) const;
		int    FASTCALL Copy(const Brush & rS);
		int    FASTCALL IsEqual(const Brush & rS) const;
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
		int    IsSimple() const;
		int    FASTCALL SetSimple(SColor);

		SColor C;          // ����
		int32  S;          // ����� (SPaintObj::bsXXX)
		int8   Hatch;      // ���������� (SPaintObj::bhsXXX)
		int8   Rule;       // SPaintObj::frXXX ������� ������� //
		uint16 Reserve;    // @alignment
		//
		int32  IdPattern;  // ������������� ��������� ��� ������� ����������.
	};
	//
	// Descr: ����������� ������������� ���������
	//
	class Gradient : public Base { // @persistent @store(SSerializeContext)
	public:
		//
		// Descr: ���� ���������
		//
		enum {
			kLinear = 0, // �������� ��������
			kRadial,     // ���������� ��������
			kConical     // ���������� ��������
		};
		//
		// Descr: ������ ���������� �������
		//
		enum {
			sPad,        // ������� ������������� �������� ������� ���������
			sRepeat,     //
			sReflect
		};
		enum {
			uUserSpace,  // ���������� ��������� ������� � ���������������� ��������
			uBB          // ���������� ��������� ������� � �������� ������ ������� (Bounding Box)
		};
		struct Stop {
			float  Offs;
			SColor C;
		};
		//
		// Descr: ���������� ��������� ���������
		//
		enum { lcX1 = 0, lcY1, lcX2, lcY2 };
		//
		// Descr: ���������� ����������� ���������
		//
		enum { rcCX = 0, rcCY, rcR, rcFX, rcFY, rcFR };

		Gradient(int kind = kLinear, int units = uUserSpace);
		Gradient & operator = (const Gradient & rS);
		int    operator == (const Gradient &) const;
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
		int    SetPrototype(const Gradient & rS);
		int    SetLinearCoord(int coord, float val, int pct);
		int    SetRadialCoord(int coord, float val, int pct);
		float  GetLinearCoord(int coord, int * pPct) const;
		float  GetRadialCoord(int coord, int * pPct) const;
		int    GetUnits() const;
		int    AddStop(float offs, SColor c);
		uint   GetStopCount() const;
		const  Stop * GetStop(uint idx) const;

		int8   Kind;    // Gradient::kXXX
		int8   Spread;  // Gradient::sXXX
		LMatrix2D Tf;   // ������� �������������� //
	private:
		int8   Unit;    // Gradient::uXXX
		uint8  PctUf;   // ����� �������, �������� � ���������
			// ��� ��������� ���������:
			// 0x01 - x1, 0x02 - y1, 0x04 - x2, 0x08 - y2
			// ��� ����������� ���������:
			// 0x01 - cx, 0x02 - cy, 0x04 - r, 0x08 - fx, 0x10 - fy, 0x20 - fr (������ ������)
		float  Coord[8];
		TSArray <Stop> StopList;
	};

	class Font : public Base, public SFontDescr { // @persistent @store(SSerializeContext)
	public:
		Font();
		~Font();
		operator HFONT () const;
		operator cairo_font_face_t * () const;
		operator cairo_scaled_font_t * () const;
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
		int    GetGlyph(SDrawContext & rCtx, uint16 chr, SGlyph * pGlyph);

		float  LineAdv; // ��������, �� ������� ���������� ������������� ��� �������� �� ����� ������.
	};
	//
	// Descr: ������������� ����� ��������� ��������
	//
	class CStyle : public Base { // @persistent @store(SSerializeContext)
	public:
		CStyle();
		int    FASTCALL IsEqual(const CStyle & rS) const;
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);

		int    FontId;
		int    PenId;
		int    BrushId;
		uint8  Reserve[16];
	};
	//
	// Descr: ������������� ������� ���������� ���������
	//
	class Para : public Base, public SParaDescr { // @persistent @store(SSerializeContext)
	public:
		Para();
		int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	};

	SPaintObj(int ident = 0);
	~SPaintObj();
	void   Destroy();
	int    GetType() const;
	int    GetId() const;

	Pen *  GetPen() const;
	Brush * GetBrush() const;
	Font * GetFont() const;
	Gradient * GetGradient() const;
	Para * GetParagraph() const;
	CStyle * GetCStyle() const;

	operator COLORREF () const;
	operator SColor () const;
	operator HGDIOBJ () const;
	operator HCURSOR () const;
	operator HBITMAP () const;
	operator cairo_pattern_t * () const;
	//
	// Descr: ����� ������� SPaintObj::Copy
	//
	enum {
		cfLeaveId = 0x0001 // �� ������ ���� Id � ������� this
	};
	//
	// Descr: �������� ���������� ������� rS � ������ this.
	//   ����������� ������.
	//
	int    Copy(const SPaintObj & rS, long flags = 0);
	//
	// Descr: �������� ���������� ���������� � ������ pDest.
	//   � ���������� ������ ������� ��������� this ������
	//   ������ ����� �������� ����������� ��������� (��. SPaintObj::ResetOwnership()).
	//   �������� ����� ��������� ��������� � pDest.
	//   �� ���� ������� ������ ����� �� ����� ������������ const.
	//
	int    CopyTo(SPaintObj * pDest);
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);

	int    CreateHPen(int style, float width, SColor c);
	int    CreateHBrush(int style, SColor c, int32 hatch);
	int    CreatePen(int style, float width, SColor c);
	//
	// Descr: ��������� ������ ���� tPen.
	//   ���������� ���������� ������� �����������.
	//   ���� pPen == 0, �� ������ ��������� ������.
	//
	int    CreatePen(const Pen * pPen);
	int    CreateBrush(int style, SColor c, int32 hatch);
	//
	// Descr: ��������� ������ ���� tBrush.
	//   ���������� ���������� ������� �����������.
	//   ���� pBrush == 0, �� ������ ��������� ������.
	//
	int    CreateBrush(const Brush * pBrush);
	int    CreateFont(const char * pFace, int height, int flags);
	int    CreateGradient(const Gradient * pGradient);

	int    CreateGradientLinear(const FRect & rBound);
	int    CreateGradientRadial(const FShape::Circle & rBound);
	int    AddGradientStop(float offs, SColor c);
	int    CreateParagraph();
	int    CreateCStyle(int fontId, int penId, int brushId);
	int    CreateColor(SColor c);
	int    SetBitmap(uint bmpId);
	int    SetFont(HFONT);
	int    CreateCursor(uint cursorId);
	int    FASTCALL CreateInnerHandle(SDrawContext & rCtx);
	int    FASTCALL DestroyInnerHandle();
private:
	//
	// Descr: ����� ������ ���� ������� ��������� ������ ����� �������� ��������
	//   ���������, ������������ � ��� �������. �� ����, ����� Destroy ��
	//   ����� ��������� �������, �� ������� ��������� �������������� ���
	//   ����������� ������� ������, �� ������� ��������� SPaintObj::H
	//
	void   ResetOwnership();
	int    FASTCALL ProcessInnerHandle(SDrawContext * pCtx, int create);

	enum {
		fCairoPattern = 0x0001,
		fInner        = 0x0002, // ���������� ������������� �������
			// (tPen : SPaintObj::Pen; tBrush : SPaintObj::Brush; tFont : SPaintObj::Font)
		fNotOwner     = 0x0004  // ��������� �� ������� �������� ���������, �� ������� ��������� SPaintObj::H
	};

	int32  Id;
	int16  T;   // ��� ������� (SPaintObj::tXXX)
	uint16 F;
	void * H; // @v9.1.7 uint32-->(void *)
};
//
//   �������������� ������������ ������� �� ��� ���������:
//   1. ������� (1..10000) �������� SPaintToolBox ���������� enum � ������� ����������� ��� ��������������
//   2. ������� ����������������� (80001..90000) ��� SPaintToolBox ���������� ����� ���������������.
//   3. ������������ (100001..) ������������� ��������� �������� SPaintToolBox::CreateDynIdent
//
class SPaintToolBox : private TSArray <SPaintObj> { // @persistent @store(SSerializeContext)
public:
	//
	// ����������������� �������������� ������������
	//
	enum {
		robjFirst = 80001,
		rbrWindow,
		rbrWindowFrame,
		rbr3DDkShadow,
		rbr3DLight,
		rbr3DFace,
		rbr3DShadow,
		rbr3DHilight
	};

	SPaintToolBox();
	~SPaintToolBox();
	SPaintToolBox & Init();
	int    Copy(const SPaintToolBox & rS);
	//
	// Descr: �������� ������ � ��������������� toolIdent �� ���������� rS � this.
	//   ��� ������������ ������� � ���������� this ����������� ����� �������������.
	// Returns:
	//   ������������� ����� ������� � ���������� this.
	//
	int    CopyToolFrom(const SPaintToolBox & rS, int toolIdent);
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	int    CreateReservedObjects();
	//
	// Descr: ������� ������ ������ � ��������������� ident.
	//   ���� ������� ���������� �������, �� ���������� ��������� ��
	//   ��������� ������, ������� ����� ���� ��������������� (��������
	//   �������� SPaintObj::CreatePen(SPaintObj::Pen *) ��� �����������).
	//
	//   ���� �� �������������� ident ��� ���������� ������, �� ������� ���������� 0.
	// Note: ������ �� ������������� ��������� �������� �������������� ����������
	//   SPaintToolBox.
	//
	SPaintObj * FASTCALL CreateObj(int ident);
	int     FASTCALL DeleteObj(int ident); // @v9.1.7
	SPaintObj * FASTCALL GetObj(int ident) const;
	SPaintObj * FASTCALL GetObjBySymb(const char * pSymb, int type) const;
	//
	// Descr: ���� ���-������ �������������� ident. ���� ����� ������ ������,
	//   �� ���������� !0 � �� ������ rSymb ����������� ������ �������.
	//   � ��������� ������ ���������� 0 � �� ������ rSymb ����������� 0 (������ ������).
	//
	int    GetSymb(int ident, SString & rSymb) const;
	//
	// Descr: ������� ������������ ������������� �������.
	//   ���� ��� ��������� pSymb ����������� ������� !isempty(pSymb),
	//   �� ����������� ������������� ������������� � �������� pSymb.
	//   ���� ������ pSymb ��� ���������� � ����������, ��
	//   ������������ �������������, ��������������� � ���.
	// Returns:
	//   >0 - �������������, ������� ����� ���� ����������� ��� �������� ������ �������.
	//   0  - ������.
	//
	int    FASTCALL CreateDynIdent(const char * pSymb = 0);
	//
	// Descr: ���� ������������� �������, ��������������� � �������� pSymb.
	//   ������� ���� ������ �������������, ��������������� � ��������, ��
	//   �� �������� �������� ���������� �� � ���������� ������, ���������������
	//   ����� ��������������.
	// Returns:
	//   >0 - ��������� ������������� �������
	//   0  - ������ �� ������.
	//
	int    FASTCALL SearchSymb(const char * pSymb) const;
	int    FASTCALL SearchColor(SColor c) const;
	//
	// Descr: ������� WinGdi-���� (HPEN) � ��������� ��� � ��������������� ident.
	//
	int    SetPen(int ident, int style, int width, COLORREF c);
	//
	//
	//
	int    SetDefaultPen(int style, int width, SColor c);
	int32  GetDefaultPen();
	//
	// Descr: ������� WinGdi-����� (HBRUSH) � ��������� ��� � ��������������� ident.
	//
	int    SetBrush(int ident, int style, COLORREF c, int32 hatch);
	int    SetColor(int ident, COLORREF);
	int    SetBitmap(int ident, uint bmpId);
	int    SetFont(int ident, HFONT);
	int    CreateCursor(int ident, uint cursorId);

	int    CreateColor(int ident, SColor c);
	int    CreatePen(int ident, int style, float width, SColor c);
	int    CreateBrush(int ident, int style, SColor c, int32 hatch, int patternId = 0);
	int    CreateFont(int ident, const char * pFace, int height, int flags);
	int    CreateGradientLinear(int ident, const FRect &);
	int    CreateGradientRadial(int ident, const FShape::Circle &);
	int    AddGradientStop(int ident, float off, SColor c);
	int    CreateParagraph(int ident, const SParaDescr * pDescr);
	int    CreateCStyle(int ident, int fontId, int penId, int brushId);

	COLORREF FASTCALL GetColor(int ident) const;
	//
	// Descr: ������� ������� ��������� �����, ������������ 0, ����
	//   ������ �� ��������� ����������� �� ���������������.
	//
	int    GetColor(int ident, COLORREF * pC) const;
	HGDIOBJ FASTCALL Get(int ident) const;
	HCURSOR FASTCALL GetCursor(int ident) const;
	HBITMAP FASTCALL GetBitmap(int ident) const;

	SPaintObj::Font * GetFont(SDrawContext & rCtx, int fontIdent);
	SPaintObj::Para * GetParagraph(int ident);
	int    GetGlyphId(SDrawContext & rCtx, int fontIdent, wchar_t chr);
	const  SGlyph * FASTCALL GetGlyph(int glyphId) const;
private:
	enum {
		stReservedObjects = 0x0001 // ������� ����������������� �������
	};
	virtual void FASTCALL freeItem(void *);
	int    FASTCALL GetType(int ident) const;
	int32  DynIdentCount;
	int32  State;
	int32  DefaultPenId; // @v8.9.0
	SymbHashTable Hash; // ������������ ��� �������� ���������� ���������������, ��������������� � ��������������.
	struct GlyphEntry {
		int32  I;
		SGlyph G;
	};
	SArray GlyphList;
};

class STextLayout {
public:
	//
	// Descr: ����� ����� ������������ ������ � ������
	//
	enum {
		fUnlimX        = 0x0001,
		fUnlimY        = 0x0002,
		fWrap          = 0x0004, //
		fNoClip        = 0x0008, // �� ������������ ��������� ���������
		fOneLine       = 0x0010, // �������� ����� � ���� ������ (�� ���������� � ������������ �������� �������)
		fPrecBkg       = 0x0020, // ����������� ��� ������ �� ������ (� ��������� ������ ������������� ��� �������,
			// ���������� ��� ����� ������).
		fVCenter       = 0x0040, // @v9.1.8 ������������ �������������
		fVBottom       = 0x0080  // @v9.1.8 ������������ ������������ �� ������ �������
			// @#(fVCenter^fVBottom)
	};
	struct Item {
		enum {
			fUnderscore = 0x0001
		};
		int16  GlyphIdx; // -1 - �� �������� ������
		uint16 Flags;    // @v9.1.8 ������������ ����� ����������� �������
		FPoint P;
	};
	struct RenderGroup {
		RenderGroup();

		SPaintObj::Font * P_Font;
		int    PenId;
		int    BrushId;
		TSArray <STextLayout::Item> Items;
	};

	STextLayout();
	void   Reset();
	int    Copy(const STextLayout & rS);
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	const  SStringU & GetText() const;
	int    SetText(const char * pText);
	int    SetOptions(long flags, int defParaStyle = -1, int defCStyle = -1);
	int    FASTCALL HasOption(long f) const;
	int    AddParagraph(const char * pText, int paraStyleId);
	int    AddText(const char * pText, int cstyleId);
	int    SetTextStyle(uint startPos, uint len, int cstyleId);
	const FRect & GetBounds() const;
	//
	// Descr: ���������� ������� ������� ����� ������� ������.
	//   ��� ������������� ����� fPrecBkg ��� ������� ���������� �� GetBounds().
	//
	FRect  GetBkgBounds() const;
	int    FASTCALL SetBounds(const FRect & rBounds);
	int    EnumGroups(uint * pI, RenderGroup * pGroup);
	int    Arrange(SDrawContext & rCtx, SPaintToolBox & rTb);
	const  LongArray GetGlyphIdList() const
	{
		return GlyphIdList;
	}
private:
	int    Preprocess(SDrawContext & rCtx, SPaintToolBox & rTb);
	int    FASTCALL CanWrap(uint pos);

	struct CStyle {
		uint32 Start;
		uint32 Len;
		int32  StyleIdent;
	};
	struct StyleGroup {
		uint   Pos;
		SPaintObj::Font * P_Font;
		int    PenId;
		int    BrushId;
	};
	enum {
		stPreprocessed = 0x0001,
		stArranged     = 0x0002
	};
	int32  DefParaStyleIdent; // ������������� ����� ��������� �� ���������
	int32  DefCStyleIdent;    // ������������� ����� �������� �� ���������.
	long   Flags;
	long   State;
	FRect  Bounds;
	FPoint EndPoint;          // @*STextLayout::Arrange �����, �� ������� ������������ ���������� �����.
	SStringU Text;
	LongArray GlyphIdList;                    // @transient
	TSArray <CStyle> CStyleList;              //
	LAssocArray ParaList;                     // ������ ����������: ����������� {������� ������� �������; ����� ���������}
	TSArray <STextLayout::Item> List;         // @transient
	TSArray <STextLayout::StyleGroup> SgList; // @transient

	friend class TloRowState;
};

class TCanvas2 {
public:
	struct Surface {
		Surface()
		{
			HCtx = 0;
			P_Img = 0;
		}
		uint32 HCtx;   // �������� ����������
		SImageBuffer * P_Img;
	};
	struct Capability {
		TPoint SizePt; // ������ ���������� � ������
		FPoint SizeMm; // ������ ���������� � �����������
		FPoint PtPerInch; // ���������� ����� �� ����
	};

	struct FontExt {
		float Ascent;      // The distance that the font extends above the baseline.
			// Note that this is not always exactly equal to the maximum of the extents
			// of all the glyphs in the font, but rather is picked to express the font designer's
			// intent as to how the font should align with elements above it.
		float Descent;     // The distance that the font extends below the baseline. This value is
			// positive for typical fonts that include portions below the baseline. Note that this
			// is not always exactly equal to the maximum of the extents of all the glyphs in the font,
			// but rather is picked to express the font designer's intent as to how the the font should
			// align with elements below it.
		float Height;      // The recommended vertical distance between baselines when setting consecutive
			// lines of text with the font. This is greater than ascent+descent by a quantity known as the
			// line spacing  or external leading. When space is at a premium, most fonts can be set with only
			// a distance of ascent+descent between lines.
		FPoint MaxAdvance; // X - the maximum distance in the X direction that the the origin is advanced
			// for any glyph in the font.
			// Y - the maximum distance in the Y direction that the the origin is advanced for any glyph
			// in the font. this will be zero for normal fonts used for horizontal writing.
			// (The scripts of East Asia are sometimes written vertically.)
	};
	struct TextExt {
		RPoint Bearing; // X - the horizontal distance from the origin to the leftmost part of the glyphs as drawn.
			// Positive if the glyphs lie entirely to the right of the origin.
			// Y - the vertical distance from the origin to the topmost part of the glyphs as drawn.
			// Positive only if the glyphs lie completely below the origin; will usually be negative.
		RPoint Size;    // Size of the glyphs as drawn
		RPoint Advance; // X - distance to advance in the X direction after drawing these glyphs
			// Y - distance to advance in the Y direction after drawing these glyphs.
			// Will typically be zero except for vertical text layout as found in East-Asian languages.
	};
	//
	// Descr: �������� ����������, ������������ ��� ���������
	//
	enum {
		oprCLEAR,
    	oprSOURCE,
    	oprOVER,
    	oprIN,
    	oprOUT,
    	oprATOP,

    	oprDEST,
    	oprDEST_OVER,
    	oprDEST_IN,
    	oprDEST_OUT,
    	oprDEST_ATOP,

    	oprXOR,
    	oprADD,
    	oprSATURATE,

    	oprMULTIPLY,
    	oprSCREEN,
    	oprOVERLAY,
    	oprDARKEN,
    	oprLIGHTEN,
    	oprCOLOR_DODGE,
    	oprCOLOR_BURN,
    	oprHARD_LIGHT,
    	oprSOFT_LIGHT,
    	oprDIFFERENCE,
    	oprEXCLUSION,
    	oprHSL_HUE,
    	oprHSL_SATURATION,
    	oprHSL_COLOR,
    	oprHSL_LUMINOSITY
	};
	SLAPI  TCanvas2(SPaintToolBox & rTb, HDC);
	SLAPI  TCanvas2(SPaintToolBox & rTb, SImageBuffer & rBuf);
	SLAPI ~TCanvas2();
	//
	// Descr: ���������� ����������� ��������� ���������, ��������������� � ������
	//   �����������.
	//   ���� ��������� �� ������������ � ���������� ���������, �� ���������� 0.
	//
	operator HDC() const;
	operator SDrawContext () const;
	//
	// Descr: ���� ��������� ������ �� ���� ������ ����������� SImageBuffer,
	//   �� ���������� ��������� �� ���� �����. � ��������� ������ ���������� 0.
	//
	operator const SImageBuffer * () const;
	int    GetCapability(Capability * pCaps) const;
	SPaintToolBox & GetToolBox()
	{
		return R_Tb;
	}
	// @v9.1.7 {
	int    FASTCALL SelectObjectAndPush(HGDIOBJ);
	int    SLAPI PopObject();
	int    FASTCALL PopObjectN(uint c);
	// } @v9.1.7
	//
	// Primitives
	//
	FPoint GetCurPoint();
	void   FASTCALL MoveTo(FPoint to);
	void   FASTCALL Line(FPoint to);
	void   FASTCALL LineV(float yTo);
	void   FASTCALL LineH(float xTo);
	void   FASTCALL Rect(const FRect & rRect);
	void   FASTCALL Rect(const TRect & rRect);
	void   SLAPI    RoundRect(const FRect &, float radius);
	int    FASTCALL Ellipse(const FRect & rRect);
	void   SLAPI Arc(FPoint center, float radius, float startAngleRad, float endAngleRad);
	void   SLAPI Bezier(FPoint middle1, FPoint middle2, FPoint end);
	int    SLAPI Text(const char * pText, int identFont);
	void   SLAPI ClosePath();
	void   SLAPI SubPath();
	void   FASTCALL GetClipExtents(FRect & rR);
	//
	//
	//
	void   SLAPI SetColorReplacement(SColor original, SColor replacement);
	void   SLAPI ResetColorReplacement();
	//
	int    FASTCALL SetOperator(int opr);
	int    SLAPI GetOperator() const;
	//
	// Descr: ������������� ������� �������������� ������ rMtx.
	//
	void   FASTCALL SetTransform(const LMatrix2D & rMtx);
	//
	// Descr: ��������� � ������������ ������� �������������� ������� rMtx.
	//
	void   FASTCALL AddTransform(const LMatrix2D & rMtx);
	void   FASTCALL GetTransform(LMatrix2D & rMtx) const;
	//
	// Descr: �������������� ������ ������� ���������.
	//
	int    SLAPI BeginScope();
	//
	// Descr: ��������� ������ ������ ������� ��������� � ����������
	//   ������, ���������� ������� ���������, ����������� � �������
	//   ����������� ������ BeginScope().
	//
	int    FASTCALL EndScope(SRegion & rR);

	int    SLAPI Stroke(int paintObjIdent, int preserve);
	int    SLAPI Fill(int paintObjIdent, int preserve);
	int    SLAPI Fill(SColor c, int preseve);
	//
	// Descr: ��������� �� ���������� ����� ������� ������� ��������������.
	//
	void   SLAPI PushTransform();
	//
	// Descr: ��������������� �� ����������� ����� ������� ��������������,
	//   ������� �� ����� ���� ��� ��������� ������� PushTransform().
	//
	int    SLAPI PopTransform();
	//
	//
	//
	void   SLAPI LineVert(int x, int yFrom, int yTo);
	void   SLAPI LineHorz(int xFrom, int xTo, int y);
	void   SLAPI Rect(const TRect & rRect, int penIdent, int brushIdent);
	void   SLAPI Rect(const FRect & rRect, int penIdent, int brushIdent);
	void   SLAPI RoundRect(const FRect &, float radius, int penIdent, int brushIdent);
	// @construction int    SLAPI RoundRect(const TRect & rRect, int penIdent, int brushIdent);
	int    SLAPI PatBlt(const TRect & rR, int brushId, int opr);
	int    FASTCALL SetBkColor(COLORREF);
	int    FASTCALL SetTextColor(COLORREF);
	void   SLAPI SetBkTranparent();

	TPoint SLAPI GetTextSize(const char * pStr);
		// @>>BOOL GetTextExtentPoint32(HDC hdc, LPCTSTR lpString, int cbString, LPSIZE lpSize);
	int    SLAPI TextOut(TPoint p, const char * pText);
	int    SLAPI _DrawText(const TRect & rRect, const char * pText, uint options);
	int    SLAPI DrawTextLayout(STextLayout * pTlo);
	//
	//
	//
	int    FASTCALL Draw(const SImageBuffer * pImg);
	int    FASTCALL Draw(const SDrawFigure * pDraw);
	int    FASTCALL Draw(const SDrawPath * pPath);
	int    FASTCALL Draw(const SDrawShape * pShape);
	int    FASTCALL Draw(const SDrawText * pText);
	int    FASTCALL Draw(const SDrawImage * pImg);
	int    FASTCALL Draw(const SDrawGroup * pDraw);

	enum {
		edgeRaisedInner = 0x0001,
		edgeSunkenInner = 0x0002,
		edgeRaisedOuter = 0x0004,
		edgeSunkenOuter = 0x0008,
		edgeBump        = edgeRaisedOuter|edgeSunkenInner,
		edgeEtched      = edgeSunkenOuter|edgeRaisedInner,
		edgeRaised      = edgeRaisedOuter|edgeRaisedInner,
		edgeSunken      = edgeSunkenOuter|edgeSunkenInner,
		edgeOuter       = edgeRaisedOuter|edgeSunkenOuter,
		edgeInner       = edgeRaisedInner|edgeSunkenInner
	};
	enum {
		borderLeft     = 0x0001,
		borderTop      = 0x0002,
		borderRight    = 0x0004,
		borderBottom   = 0x0008,
		borderDiagonal = 0x0010,

		borderTopLeft  = borderTop | borderLeft,
		borderTopRight = borderTop | borderRight,
		borderBottomLeft  = borderBottom | borderLeft,
		borderBottomRight = borderBottom | borderRight,
		borderRect        = borderLeft | borderTop | borderRight | borderBottom,

		borderMiddle   = 0x0800, // Fill in the middle
		borderSoft     = 0x1000, // For softer buttons
		borderAdjust   = 0x2000, // Calculate the space left over
		borderFlat     = 0x4000, // For flat rather than 3D borders
		borderMono     = 0x8000, // For monochrome borders

		borderDiagEndTopRight    = borderDiagonal|borderTop|borderRight,
		borderDiagEndTopLeft     = borderDiagonal|borderTop|borderLeft,
		borderDiagEndBottomLeft  = borderDiagonal|borderBottom|borderLeft,
		borderDiagEndBottomRight = borderDiagonal|borderBottom|borderRight
	};
	int    SLAPI DrawEdge(TRect & rR, long edge, long flags);
	int    SLAPI DrawFrame(const TRect & rR, int clFrame, int brushId);

	int    FASTCALL SelectFont(SPaintObj::Font * pFont);
private:
	enum {
		fOuterSurface   = 0x0001, // Surface ����� ������ �� ������� ������. �.�. ��������� ��� �� �������.
		fScopeRecording = 0x0002  // ������ � ��������� ������ ������ ��������� � ������ this->Scope.
	};
	class DrawingProcFrame {
	public:
		DrawingProcFrame(TCanvas2 * pCanv, const SDrawFigure * pFig);
		~DrawingProcFrame();
	private:
		TCanvas2 * P_Canv;
		const SDrawFigure * P_Fig;
		int    MtxAppl;
	};
	struct PatternWrapper {
		PatternWrapper();
		~PatternWrapper();
		void * P;
	};

	void   Init(); // @<<TCanvas2::TCanvas2()
	int    FASTCALL SetCairoColor(SColor c);
	int    SLAPI Helper_SelectPen(SPaintToolBox * pTb, int penId);
	int    SLAPI Helper_SelectBrush(SPaintToolBox * pTb, int brushId, PatternWrapper & rPw);
	int    SLAPI Implement_ArcSvg(FPoint radius, float xAxisRotation, int large_arc_flag, int sweep_flag, FPoint toPoint);
	int    SLAPI Implement_Stroke(SPaintToolBox * pTb, int paintObjIdent, int preserve);
	//
	// Descr: ������������ ����� ��� ������ ������������ (��������������, ��� ��� �� ����� ��� ���� �������).
	//
	int    FASTCALL Implement_Stroke(int preserve);
	int    SLAPI Implement_Fill(SPaintToolBox * pTb, int paintObjIdent, int preserve);
	int    SLAPI Implement_StrokeAndFill(SPaintToolBox * pTb, int penIdent, int brushIdent);

	Surface S;
	SRegion Scope; // ������� ������� ��������� ����� �������� BeginScope() � EndScope()
	long   Flags;
	SString TempBuf;
	SPaintToolBox & R_Tb;
	TSStack <LMatrix2D> TmStk;
	SStack GdiObjStack; // @v9.1.7 ��� ������������� � TCanvas �� ������ �������

	struct ColorReplacement {
		ColorReplacement();
		void   Reset();
		void   Set(SColor org, SColor rpl);

		enum {
			fActive = 0x0001 // ��������� ������ �����������
		};
		long   Flags;
		SColor  Original;
		SColor  Replacement;
	};
	ColorReplacement ClrRpl;
	//
	cairo_t * P_Cr;
	cairo_surface_t * P_CrS;
	SPaintObj::Font * P_SelectedFont;
};
//
//
//
class TCanvas {
public:
	SLAPI  TCanvas(HDC);
	SLAPI ~TCanvas();
	operator HDC() const;

	void   FASTCALL SetBounds(const TRect &);
	//const  TRect & GetBounds() const { return Bounds; }

	int    FASTCALL SelectObjectAndPush(HGDIOBJ);
	int    SLAPI PopObject();
	int    FASTCALL PopObjectN(uint c);
	int    FASTCALL MoveTo(TPoint);
	int    FASTCALL Line(TPoint);
	void   SLAPI LineVert(int x, int yFrom, int yTo);
	void   SLAPI LineHorz(int xFrom, int xTo, int y);
	void   SLAPI SetBkTranparent();
	int    FASTCALL SetBkColor(COLORREF);
	int    FASTCALL SetTextColor(COLORREF);
	int    FASTCALL Rectangle(const TRect &);
	int    SLAPI RoundRect(const TRect &, const TPoint & rRoundPt);
	int    SLAPI FillRect(const TRect & rRect, HBRUSH brush);
	TPoint FASTCALL GetTextSize(const char * pStr);
		// @>>BOOL GetTextExtentPoint32(HDC hdc, LPCTSTR lpString, int cbString, LPSIZE lpSize);
	int    SLAPI TextOut(TPoint p, const char * pText);
	int    SLAPI DrawText(const TRect & rRect, const char * pText, uint options);
private:
	enum {
		fOuterDC = 0x0001
	};
	HDC    H_Dc;
	long   Flags;
	TRect  Bounds;
	SStack ObjStack;
};
//
// Descr: ��������������� �������������� ��������� ����������� �� PPView �������
//   ����� ���������� ���������� ��������������� ������ � �������������
//   ����������� �� ���������� PPView::SubSign
//
#define TV_SUBSIGN_DIALOG    10  // TDialog
#define TV_SUBSIGN_BUTTON    11  // TButton
#define TV_SUBSIGN_INPUTLINE 12  // TInputLine
#define TV_SUBSIGN_CLUSTER   13  // Cluster of checkboxes or radiobuttons
#define TV_SUBSIGN_STATIC    14  // TStaticText
#define TV_SUBSIGN_LABEL     15  // TLabel
#define TV_SUBSIGN_LISTBOX   16  // SmartListBox
#define TV_SUBSIGN_COMBOBOX  17  // ComboBox
#define TV_SUBSIGN_IMAGEVIEW 18  // TImageView

class TBitmapHash {
public:
	TBitmapHash();
	~TBitmapHash();
	HBITMAP    FASTCALL Get(uint bmpId);
	HBITMAP    FASTCALL GetSystem(uint bmpId);
private:
	struct Entry {
		long   ID;
		uint32 H;
	};
	TSArray <Entry> List;
};

class TView {
public:
	//static void * message(TView * pReceiver, uint what, uint command);
	//static void * message(TView * pReceiver, uint what, uint command, void * pInfoPtr);
	// @v9.5.5 static void * message(TView * pReceiver, uint what, uint command, long infoVal);
	static void * FASTCALL messageCommand(TView * pReceiver, uint command);
	static void * messageCommand(TView * pReceiver, uint command, void * pInfoPtr);
	static void * FASTCALL messageBroadcast(TView * pReceiver, uint command);
	static void * messageBroadcast(TView * pReceiver, uint command, void * pInfoPtr);

	enum phaseType {
		phFocused,
		phPreProcess,
		phPostProcess
	};
	enum selectMode {
		normalSelect,
		enterSelect,
		leaveSelect,
		forceSelect // same as normalSelect but don't check current selection
	};

	int    commandEnabled(ushort command) const;
	void   FASTCALL enableCommands(const TCommandSet & commands, int is_enable);
	void   FASTCALL enableCommand(ushort command, int is_enable);
	void   getCommands(TCommandSet & commands) const;
	void   setCommands(const TCommandSet & commands);

	static HFONT setFont(HWND hWnd, const char * pFontName, int height);
	//
	// Descr: ������� �������� ������ �� �������� rFd.
	// Note: ��� ������� ������� ������������ ����� ������ wingdi-������� CreateFontIndirect().
	//
	static void * CreateFont(const SFontDescr & rFd);
	static void * SetWindowProp(HWND hWnd, int propIndex, void * ptr);
	static void * SetWindowUserData(HWND hWnd, void * ptr);
	static long SetWindowProp(HWND hWnd, int propIndex, long value);
	static void * FASTCALL GetWindowProp(HWND hWnd, int propIndex);
	static void * FASTCALL GetWindowUserData(HWND hWnd);
	static long FASTCALL GetWindowStyle(HWND hWnd);
	static long FASTCALL GetWindowExStyle(HWND hWnd);
	static int  FASTCALL SGetWindowClassName(HWND hWnd, SString & rBuf);
	static int  FASTCALL SGetWindowText(HWND hWnd, SString & rBuf);
	static int  FASTCALL SSetWindowText(HWND hWnd, const char * pText);
	//
	// Descr: ���������� �������� ���� ������������� ���� hWnd �
	//   ��������� ����������� ����������������� ��������� �����
	//   ���� "@textident" ��� "@{textident}"
	//   ����������� ��� ����, ��������� � ����� ������� �������� �������� TDialog
	//
	static void FASTCALL PreprocessWindowCtrlText(HWND hWnd);

	TView(const TRect & bounds);
	TView();
	virtual ~TView();
	DECL_HANDLE_EVENT;
	// @v9.0.1 TRect  getExtent() const;
	void   setBounds(const TRect & bounds);
	void   changeBounds(const TRect & bounds);
	uint   getHelpCtx();
	TView & SetId(uint id);
	uint   GetId() const;
	int    FASTCALL TestId(uint id) const;
	//
	// Descr: ���������� !0 ���� ���� ��������� ������� �������� ���� �� ����
	//   �� ������, ������������� � ��������� s.
	//
	int    FASTCALL IsInState(uint s) const;
	virtual int    FASTCALL valid(ushort command);
	//
	// Descr: �����, ������������ ��� �������� (����������) ������ � (��)
	//   ��������� �������. ����� ����, ����� ��������� ������ �������
	//   ������ �������.
	//
	virtual int    TransmitData(int dir, void * pData);
	// @v9.0.4 virtual void   endModal(ushort command);
	virtual void setState(uint aState, bool enable);
	virtual int  handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// @v9.6.2 virtual void   draw();
	void   Draw_(); // @v9.6.2
	//
	// Descr: ������������ ������� ������� � �����, �������� �� ������������
	//
	// @v9.1.3 virtual int  Paint_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//
	// Descr: ���������� ��� �������� ���� � ����������� �� ��������� doShow.
	// Note: ������ ���� ������� ������������� ������������ setState(sfVisible, doShow)
	// ARG(doShow IN): !0 - ���������� ����, 0 - �������� ����
	//
	void   Show(int doShow);
	// @v9.6.2 (replaced by Draw_()) void   drawView();
	void   clearEvent(TEvent & event)
	{
		event.what = evNothing;
		event.message.infoPtr = this;
	}
	HWND   getHandle() const;
	void   select();
	//
	// Descr: ���� this �������� ������� ������ owner'�, ��
	//   ���������� �������� owner->curren � 0.
	//
	void   ResetOwnerCurrent();
	int    FASTCALL EventBarrier(int rmv = 0);
	TView * nextView();
	TView * prevView();
	TView * prev();
	TView * next;
	TView * TopView();
	int    IsConsistent() const;
	int    FASTCALL IsSubSign(uint) const;
	uint   GetSubSign() const;
	int    GetEndModalCmd() const;
	//
	// Descr: ������ ���������� �������� (�����������) ��������� ����������� �������
	//   ��� �������������� ������������ ������� �������
	//
	int    OnDestroy(HWND);
	void   SendToParent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void   SetWordSelBlock(WordSel_ExtraBlock *);
	int    hasWordSelector() const { return BIN(P_WordSelBlk); }
	//
	// Descr: ������ ��� ���������� � ������� TWindow::getCtrlView
	//
	uint16 GetId_Unsafe() const
	{
		return Id;
	}
private:
	uint32 Sign;    // ������� ���������� ������. ������������ ��� ������������� ���������� �����������.
protected:
	friend class EvBarrier;

	class EvBarrier {
	public:
		EvBarrier(TView * pV);
		~EvBarrier();
		int    operator !() const;
	private:
		int    Busy;
		TView * P_V;
	};
	uint32 SubSign; // ������� ������� ������. ����������� ������ ���� ����������
		// �������� ����� ���� ��� ������������� "����-�����"
	uint16 Id;
	uint16 Reserve;
	uint32 Sf;      // ���� ������ ��������� ������� (sfXXX)
	WordSel_ExtraBlock * P_WordSelBlk; // owner
public:
	TPoint size;
	TPoint origin;
	uint32 options;
	TGroup * owner;
	HWND   Parent;
	WNDPROC PrevWindowProc;
protected:
	int    HandleKeyboardEvent(WPARAM wParam, int isPpyCodeType = 0);
	//
	// Descr: ��� ������� ������ ���������� ������������ �������� � �����������
	//   ��� �������������� ���������� ������� ���������.
	//
	//   ��� ��� ������ ��������� ������� ����������� �� ����, ��� �����
	//   ��������� ���������� ����, ���������� ���������� ����� ���������� ������� ���������
	//   � �����������.
	//
	//   � ���������, �� �� �������� ����������� ������������ � ����� ����������
	//   ��������� (�� ��������� WM_DESTROY) �������� OnDestroy().
	//   ��� �������� � ��������� ������������ ����.
	//
	int    RestoreOnDestruction();
	//
	// Descr: �������� ����� ������������ �������� (SendDlgItemMessage(Parent, Id, WM_GETTEXT,...)
	//   �, ���� �� �������� �������� (@...), �������� ��� �� �������� �� �������.
	//
	int    SetupText(SString * pText);

	int    EndModalCmd;         // �������, �� ������� ���� ���� ������� (� ��� �����, �������� �� ���������� ���������)
	uint32 HelpCtx;
	TCommandSet * P_CmdSet;
};

class TGroup : public TView {
public:
	SLAPI  TGroup(const TRect & bounds);
	SLAPI ~TGroup();
	DECL_HANDLE_EVENT;
	virtual void   Insert_(TView *p);
	virtual void   setState(uint aState, bool enable);
	virtual int    TransmitData(int dir, void * pData);
	// @v9.6.2 virtual void   draw();
	// @v9.0.4 virtual void   endModal(ushort command);
	virtual int    FASTCALL valid(ushort command);

	ushort execView(TWindow * p);
	void   insertView(TView * p, TView * pTarget);
	void   remove(TView * p);
	void   removeView(TView *p);
	void   setCurrent(TView *p, selectMode mode);
	void   selectNext(/*Boolean forwards*/ /*false*/); // @v9.0.9 ��� ������ ����� ������� forward = False - �������
	// @v9.0.1 TView * firstThat(Boolean (*func)(TView *, void *), void *args);
	void    forEach(void (*func)(TView *, void *), void *args);
	void    insertBefore(TView *p, TView *Target);
	// @v9.4.8 TView * FASTCALL at(short index) const;
	TView * first() const;
	void   redraw();
	void   lock();
	void   unlock();
	uint   GetCurrId() const;
	int    FASTCALL IsCurrCtl(const TView * pV) const;
	int    FASTCALL isCurrCtlID(uint ctlID) const;
	//
	TView * P_Current;
	TView * P_Last;
	// @v9.0.1 phaseType Phase_;
	enum {
		fLockMsgChangedFocus = 0x001
	};
	uint   MsgLockFlags; // fLockMsgXXX
private:
	// @v9.4.8 bool   invalid(TView *p, ushort command);
};
//
//
//
class TToolTip {
public:
	struct ToolItem {
		ToolItem();

		uint   Id;
		HWND   H;     // ����, � ������� ������������ ���������
		long   Param; // �������������� ��������, ��������������� � ����������
		TRect  R;     // �������������, � ������� ������������ ���������
		SString Text; // ����� ���������
	};

	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	TToolTip(HWND hParent, uint maxWidthPix);
	~TToolTip();
	int    AddTool(ToolItem & rItem);
	uint   GetToolsCount();
	int    GetTool(uint idx /*0..GetToolsCount()-1*/, ToolItem & rItem);
	int    RemoveTool(uint ident);
	int    RemoveAllTools();
private:
	int    Create(HWND hParent);
	HWND   H;
	uint   Counter;    // ������� ��� ������������ ���������� ���������� ��������������� ���������
	uint   MaxWidthPix; // ������������ ������ ���� ����������
};
//
//
//
struct ToolbarItem {
	enum {
		fHidden = 0x0001
	};
	long   Cmd;
	uint16 KeyCode;
	uint16 Flags;
	uint   BitmapIndex;
	char   ToolTipText[128];
};

class ToolbarList : private SArray {
public:
	ToolbarList();
	ToolbarList & FASTCALL operator = (const ToolbarList &);
	void   setBitmap(uint b);
	uint   getBitmap() const;
	int    addItem(const ToolbarItem *);
	int    enumItems(uint *, ToolbarItem *);
	uint   getItemsCount() const;
	uint   getVisibleItemsCount() const;
	const  ToolbarItem & FASTCALL getItem(uint idx) const;
	int    searchKeyCode(ushort keyCode, uint * pIdx) const;
	int    moveItem(uint pos, int up);
	void   clearAll();
private:
	uint   Bitmap;
};
//
//
//
class TMenuPopup {
public:
	TMenuPopup();
	~TMenuPopup();
	//
	// Descr: ���������� ���������� ��������� ����, ������� �����������
	//   �������� Add ��� AddSubst.
	//
	uint   GetCount() const;
	//
	// Descr: ��������� ������� cmd � ������� pText.
	//   ���� ����� ���������� � ������� '@' �� �������� ������� ����� ������� ��������
	//   SlSession::LoadString �� ��������� pText+1. ���� ����� ������� ����� ������� ��
	//   �������, �� ������� ������� ����� ���� ���������.
	// Note: ������ pText ������ ���� � ANSI-���������.
	//
	int    Add(const char * pText, int cmd);
	int    AddSubstr(const char * pText, int idx, int cmd);
	int    AddSeparator();
	//
	// Descr: ����� ������� TMenuPopup::Execute
	//
	enum {
		efRet        = 0x0001, // ���������� ��������� ������. ���� ���� ���� ����������, ��
			// �� ������� ������ ����� ���������� ��������.
			// ��� ��������� ����� ���� �� �������� ��������� ���� hWnd.
	};
	int    Execute(HWND hWnd, long flags = 0);
private:
	uint32 H;
	long   State;
	uint   Count; // ���������� "�����" ��������� (�� �����������). ���������� �������� Add � AddSubstr
};

class TWindow : public TGroup {
public:
	static int IsMDIClientWindow(HWND);

	SLAPI  TWindow(const TRect& bounds, const char * pTitle, short aNumber);
	SLAPI ~TWindow();
	/*
	operator HWND() const
	{
		return this ? hWnd : (HWND)0;
	}
	*/
	/* @v9.0.4 virtual*/ void  endModal(ushort command);
	void * SLAPI  messageToCtrl(ushort ctl, ushort command, void * ptr);
	TView * FASTCALL getCtrlView(ushort ctl);
	HWND   SLAPI H() const
	{
		return this ? /*hWnd*/HW : (HWND)0;
	}
	HWND   FASTCALL getCtrlHandle(ushort ctlID);
	void   FASTCALL setCtrlReadOnly(ushort ctlID, int set);
	void   FASTCALL disableCtrl(ushort ctl, int toDisable);
	void   __cdecl  disableCtrls(int toDisable, ...);
	void   FASTCALL selectCtrl(ushort ctl);
	int    SLAPI  selectButton(ushort cmd);
	void   SLAPI  setCtrlOption(ushort id, ushort flag, int s);
	int    SLAPI  destroyCtrl(uint ctl);
	//
	// ������� setCtrlData � getCtrlData ���������� !0 ���� ����������
	// ����������� ������� � ��. ctl � 0 � ��������� ������.
	//
	int    FASTCALL setCtrlData(ushort ctl, void *);
	int    FASTCALL getCtrlData(ushort ctl, void *);

	uint16 FASTCALL getCtrlUInt16(uint ctlID);
	long   FASTCALL getCtrlLong(uint ctl);
	double FASTCALL getCtrlReal(uint ctl);
	int    FASTCALL getCtrlString(uint ctlID, SString &);
	LDATE  FASTCALL getCtrlDate(uint ctlID);
	LTIME  FASTCALL getCtrlTime(uint ctlID);
	int    SLAPI getCtrlDatetime(uint dtCtlID, uint tmCtlID, LDATETIME & rDtm);

	int    FASTCALL setCtrlUInt16(uint ctlID, int s);
	int    FASTCALL setCtrlLong(uint ctlID, long);
	int    FASTCALL setCtrlReal(uint ctlID, double);
	int    FASTCALL setCtrlString(uint ctlID, const SString &);
	int    FASTCALL setCtrlDate(uint ctlID, LDATE val);
	int    FASTCALL setCtrlTime(uint ctlID, LTIME val);
	int    SLAPI setCtrlDatetime(uint dtCtlID, uint tmCtlID, LDATETIME dtm);
	int    SLAPI setCtrlDatetime(uint dtCtlID, uint tmCtlID, LDATE dt, LTIME tm);
	//
	// Descr: ��������������� �������, ��������������� ����� option ��� SmartListBox::def
	//   � ��������������� ctlID (�������� ListBoxDef::SetOption(option, 1)
	//
	int    SLAPI setSmartListBoxOption(uint ctlID, uint option);

	void   FASTCALL drawCtrl(ushort ctlID);
	void   SLAPI showCtrl(ushort ctl, int s /* 1 - show, 0 - hide */);
	void   SLAPI showButton(uint cmd, int s /* 1 - show, 0 - hide */);
	int    SLAPI setButtonText(uint cmd, const char * pText);
	int    SLAPI setButtonBitmap(uint cmd, uint bmpID);
	void   FASTCALL setTitle(const char *);
	void   FASTCALL setOrgTitle(const char *);
	void   FASTCALL setSubTitle(const char *);
	int    SLAPI setStaticText(ushort ctl, const char *);
	int    SLAPI getStaticText(ushort ctl, SString &);
	const  SString & getTitle() const;
	void   close();
	DECL_HANDLE_EVENT;
	virtual void setState(uint aState, bool enable);
	int    SLAPI translateKeyCode(ushort keyCode, uint * pCmd) const;
	void   setupToolbar(const ToolbarList * pToolBar);
	int    AddLocalMenuItem(uint ctrlId, uint buttonId, long keyCode, const char * pText);
	HWND   showToolbar();
	void   showLocalMenu();
	//
	//
	//
	TRect  getClientRect() const;
	TRect  getRect() const;
	int    invalidateRect(const TRect &, int erase);
	int    invalidateRegion(const SRegion & rRgn, int erase);
	int    invalidateAll(int erase);
	//
	// Descr: �������������� �������� �������� ������� ��������� ����� ���������� �������
	//   ���� � (���) "���������" ���� ��� ���������� �������� (MouseHover).
	//   ���� ����� ������ ���� ������� ��������� ���� �� ��������� �������, ��
	//   ���� ������� ��������� WM_MOUSELEAVE ��� WM_MOUSEHOVER, �������������.
	//   ���� leaveNotify == 0 && hoverTimeout < 0 ����� ���������� �������� (���� ����) ����������.
	// Note: ��. �������� ������� WinAPI _TrackMouseEvent().
	// ARG(leaveNotify  IN): ������� ����� ����������� �� ��������� ����� ���������� ������� ����.
	// ARG(hoverTimeout IN): ���� hoverTimeout < 0, �� ������� �� ����� ����������� ��
	//   ������� "���������" ����. ���� hoverTimeout > 0, �� ��������� WM_MOUSEHOVER
	//   ����� ������� ����� hoverTimeout ����������� "���������". ���� hoverTimeout == 0,
	//   �� ������� "���������" ������������ ���������� �����������.
	// Returns:
	//   !0 - ������� ������� ���������.
	//   0  - ������.
	//
	int    RegisterMouseTracking(int leaveNotify, int hoverTimeout);
	//
	//
	HWND   PrevInStack;
	HWND   HW; // hWnd;
	ToolbarList Toolbar;
private:
	void   FASTCALL Helper_SetTitle(const char *, int setOrgTitle);
	TButton * FASTCALL SearchButton(uint cmd);

	class LocalMenuPool {
	public:
		LocalMenuPool(TWindow * pWin);
		int    AddItem(uint ctrlId, uint buttonId, long keyCode, const char * pText);
		int    GetCtrlIdByButtonId(uint buttonId, uint * pCtrlId) const;
		int    GetButtonIdByCtrlId(uint ctrlId, uint * pButtonId) const;
		int    ShowMenu(uint buttonId);
	private:
		struct Item {
			uint   CtrlId;
			uint   ButtonId;
			long   KeyCode;
			uint   StrPos;
		};
		SArray List;
		StringSet StrPool;
		TWindow * P_Win; // @notowned
	};
	SString Title;
	SString OrgTitle; // ���� ��������� ��������������� �������� setTitle(..., 1)
		// ��� ����, ����� � ���������� ����� ���� ������������ ������������ � ���
		// �������������� �������� setSubTitle
	LocalMenuPool * P_Lmp;
};
//
// @construction {
//
class SRectLayout {
public:
	//
	// ���� �� ����� ���� dfOpp, �� ���������� ������������� �� ������ �������� ����.
	// ���� ����� ���� dfOpp, �� ���������� ������������� �� ������� ������� ����.
	//
	enum {
		dfAbs     = 0x0001, // ���������� ������ � ���������� ��������
		dfRel     = 0x0002, // ���������� ������ � ����� �� ������ ������� ����������.
			// �������� ���������� ��� ����� � ������������� ������ (4 �����)
		dfGravity = 0x0004, // ���������� ������������� � ��������������� ������� ����������.
		dfOpp     = 0x0008  // ���������� ������������� �� ��������������� ������� ����������.
	};
	enum {
		inoOverlap = 1,
		inoVStack,
		inoHStack
	};
	struct Dim {
		int    Set(int v, int f);
		int16  Val;
		int16  Flags; // SRectLayout::dfXXX
	};
	struct Item {
		DECL_INVARIANT_C();

		Item();
		Item & SetLeft(int size, int pct);
		Item & SetRight(int size, int pct);
		Item & SetTop(int size, int pct);
		Item & SetBottom(int size, int pct);
		Item & SetCenter();

		Dim    Left;
		Dim    Top;
		Dim    Right;
		Dim    Bottom;
		int16  EmptyWidth;  // ������ ������� �������� (-1 - �� ��, ��� � ������������)
		int16  EmptyHeight; // ������ ������� �������� (-1 - �� ��, ��� � ������������)
		int    InnerOrder;
	};

	SRectLayout();
	~SRectLayout();

	int    Add(long id, const Item &);
	int    InsertWindow(long itemId, TView * pView, int minWidth, int minHeight);
	int    RemoveWindow(long winId);
	int    GetWindowBounds(long winId, TRect & rBounds);

	int    SetContainerBounds(const TRect &);
	int    Arrange();
	int    GetItemBounds(long id, TRect & rBounds);
	int    Locate(TPoint p, uint * pItemPos) const;
private:
	struct RItem {
		enum {
			stNotEmpty = 0x0001
		};
		long   Id;
		Dim    Left;
		Dim    Top;
		Dim    Right;
		Dim    Bottom;
		int16  EmptyWidth;  // ������ ������� �������� (-1 - �� ��, ��� � ������������)
		int16  EmptyHeight; // ������ ������� �������� (-1 - �� ��, ��� � ������������)
		int    InnerOrder;
		long   State;
		TRect  Bounds;
	};
	struct WItem {
		long   ItemId;
		TView * P_View;       // @notowned
		int16  MinWidth;
		int16  MinHeight;
		TRect  Bounds;
	};

	int    IsEmpty(long itemId) const;
	int    CalcCoord(Dim dim, int containerLow, int containerUpp, int gravitySide) const;

	TSArray <RItem> List;
	TSArray <WItem> WinList;
	TRect ContainerBounds;
};
//
//
//
struct TScrollBlock {
	TScrollBlock();
	int    SetupWindow(HWND hWnd) const;
	int    MoveToEdge(int side);
	int    Move(int side, int delta);
	int    Set(int x, int y);

	IntRange Rx;        // �������� ��������������� �������������� //
	IntRange Ry;        // �������� ������������� ��������������   //
	int    ScX;         // �������������� ������� ���������        //
	int    ScY;         // ������������ ������� ���������          //
};
//
//
//
class TWindowBase : public TWindow {
public:
	static int RegWindowClass(int iconId);

	//
	// Descr: ���������, ��������� �� ������� ���������� � ���������� cmInit
	//   (������ ��� �����������, ����������� �� TWindowBase).
	//
	struct CreateBlock {
		long   H_Process;
		long   H_Parent;
		long   H_Menu;
		TRect  Coord;
		uint32 Style;
		uint32 ExStyle;
		long   Param;
		const  char * P_WndCls;
		const  char * P_Title;
	};
	//
	// ����� Capability
	//
	enum {
		wbcDrawBuffer      = 0x0001, // ���� ���������� �������������� �����������
		wbcLocalDragClient = 0x0002  // ���� �������� �������� ���������� (� ������ ��������) ������ Drag'n'Drop
	};
	//
	// Descr: ����� ������� TWindowBase::Create
	//
	enum {
		// @# coChild ^ coPopup
		coChild   = 0x0001,
		coPopup   = 0x0002,
		coMDI     = 0x0004,
		coScX     = 0x0008, // ���� ��������� � �������������� ����������
		coScY     = 0x0010, // ���� ��������� � ������������ ����������
		coScXY    = (coScX|coScY),
		coMaxSize = 0x0020  // ���� ��������� � ������������� ���������, ������������ ������������ �����
	};

	TWindowBase(int capability = 0);
	~TWindowBase();
	int    Create(long parent, long createOptions);
	int    AddChild(TWindowBase *, long createOptions, long zone);
protected:
	DECL_HANDLE_EVENT;
	int    SetDefaultCursor();

	SRectLayout Layout;
	SPaintToolBox Tb;
	TScrollBlock Sb;
private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	int    MakeMouseEvent(uint msg, WPARAM wParam, LPARAM lParam, MouseEvent & rMe);
	void   RegisterMouseTracking(int force);

	static const char * P_ClsName;

	enum {
		wbsMDI                  = 0x0001,
		wbsUserSettingsChanged  = 0x0002,
		wbsMouseTrackRegistered = 0x0004
	};
	long   WbState;
	long   WbCapability;
	uint32 H_DrawBuf;
};
//
//
//
struct TArrangeParam { // @persistent
	TArrangeParam();
	TArrangeParam & Init(int dir = DIREC_HORZ);
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);

	int16  Dir;        // DIREC_HORZ || DIREC_VERT
	uint16 RowSize;    // ���������� ��������� � ����
	TPoint UlGap;      // ����� ����� ����� � ������� ��������� ������� � ���������
	TPoint LrGap;      // ����� ����� ������ � ������ ��������� ������� � ���������
	TPoint InnerGap;   // ����� ����� ���������
	TPoint ForceSize;  // @unused �������� ������ �������� �� ForceSize. ���� ForceSize.x <= 0,
		// �� ������ �� ��������, ���� ForceSize.y <= 0, �� ������ �� ��������.
};
//
// Descr: ������, ����������� ���������� ������������, ��������� ��� ������� � TWhatman.
//   ������ ���������� ����� ���� ����������� ������� � �������. ���� ������ �� ����������
//   ��� �������� �����������, ����� ��� ������������ ������������� �� ������ (�� ����������������
//   �� ������� ������).
//   ������ ���������� ���������� � ������ ������������. ������ ������������� ��� ����������� //
//   ���������� ��� �� ������� �������.
//   ����������� ���������������� ��������. ���� ��� �������� ����������� ������ ��� ��
//   ���������, �� ����� ������������� ����������� ��� ���������� 8-������� ������.
//
class TWhatmanToolArray : private SArray {
public:
	//
	// Descr: ���������������� ������������� �����������.
	//
	struct Item { // @transient [@wtmtoolitem]
		Item(const TWhatmanToolArray * pOwner = 0);
		enum {
			fDontEnlarge   = 0x0001, // �� ����������� ������ ��� ��������� �������� ������� ����� ������������������ �������
			fDontKeepRatio = 0x0002, // �� ��������� ��������� ������ ��� ��������� ������� �������
			fGrayscale     = 0x0004  // @v9.5.4 (testing option) ��������������� ������ � �����-����� ����
		};
		uint32  Id;           // @v9.1.9 ������������� ������������� ��������
		SString Symb;         // ���������� ������ ��������.
		SString Text;         // ��������� �������� �����������.
		SString WtmObjSymb;   // ������ ������ ��������� TWhatmanObject, ������� ��������� �����������
			// ������� �����������.
		SString FigPath;      // ��� �����, ����������� ����������� ������ �����������
		SString PicPath;      // ��� �����, ����������� ����������� ������ �����������
		TPoint FigSize;       // ��������� ������ ������ �����������.
		TPoint PicSize;       // ���� !isZero() �� �������������� TWhatmanToolArray::Param::PicSize
		long   Flags;         // @flags
		SColor ReplacedColor; // @v9.2.7 ����, ������� ������ ���������� �� �����-���� ������� ����. ���� ��� ����������
			// ���� ����� 0, �� ���������� ���� �� ���������.
		const  TWhatmanToolArray * P_Owner; // @notowned
		uint32 ExtSize;       // ������ ������, ������������ ��������� � ������ ExtData
		uint8  ExtData[256];
	};
	//
	// Descr: ���������������� ������������� ���������� ���������.
	//
	struct Param { // @transient
		Param();

		SString Symb;         // ������ ���������.
		SString Text;         // �������� ���������.
		SString FileName;     // ��� �����, �� �������� ��� �������� ������
		long   Flags;         // @flags
		TPoint PicSize;       // ������ ������ �� ���������.
		TArrangeParam Ap;     // ��������� �������������� ������
	};
	TWhatmanToolArray();
	~TWhatmanToolArray();
	TWhatmanToolArray & Init();
	int    SetParam(const Param &);
	int    GetParam(Param &) const;
	uint   GetCount() const;
	int    Set(Item & rItem, uint * pPos);
	int    Remove(uint pos);
	int    Get(uint pos, Item * pItem) const;
	int    SearchById(uint id, uint * pPos) const;
	int    SearchBySymb(const char * pSymb, uint * pPos) const;
	int    GetBySymb(const char * pSymb, Item * pItem) const;
	int    Pack();
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	const  SDrawFigure * GetFig(int figOrPic, uint pos, TWhatmanToolArray::Item * pItem) const;
	const  SDrawFigure * GetFig(int figOrPic, const char * pSymb, TWhatmanToolArray::Item * pItem) const;
	const  SDrawFigure * GetFigById(int figOrPic, uint id, TWhatmanToolArray::Item * pItem) const;

	int    Store(const char * pFileName);
	int    Load(const char * pFileName);
	//int    LockStorage(const char * pFileName);
private:
	SString & MakeUniqueSymb(SString & rBuf) const;
	int    CreateFigure(Item & rItem, const char * pPath, int pic);
	int    UpdateFigures(Item & rItem);

	struct Entry { // @persistent
		uint32 TextP;          // ������� ������ � ������ Pool
		uint32 SymbP;          // ������� ������� � ������ Pool
		uint32 WtmObjSymbP;    // ������� ������� ������ ������� � ������ Pool
		uint32 FigPathP;       // ������� ���� �� ����� ������ � ������ Pool
		uint32 PicPathP;       // ������� ���� �� ����� ����������� � ������ Pool
		TPoint FigSize;        // �������� ������ ������ ��� ���������� �� �������
		TPoint PicSize;        // ������ ������
		int32  Flags;          // @flags
		uint32 ExtDataP;       // ������� �������������� ������ � ������ Pool (� ��������� MIME64)
		uint32 Id;             // @v9.1.9 ������������� ������������� ��������
		SColor ReplacedColor;  // @v9.2.7
		uint8  Reserve[24];    // @reserve @v9.1.9 [32]-->[28] // @v9.2.7 [28]-->[24]
	};
	uint32 SrcFileVer;  // @transient @v9.1.9 ������ ������� �������� �����, �� �������� ��� �������� ������ ��������� �������
	uint32 SymbP;
	uint32 TextP;
	uint32 FileP;   // @transient ��� �����, �� �������� ��� �������� ������ ���������
	int32  Flags;
	TPoint PicSize;
	TArrangeParam Ap;

	StringSet Pool; // ��������� ��������� �������� (�������, ��������, ���� � ������ � �.�.)
	SDrawGroup Dg;  // ��������� ��� ������ � �����.
	//
	// �������������� ������ � ����� �������� ���������� ������������ �� ������� Item.Symb.
	// ������ �������� � �������� "{SYMB}-PIC", � ������ � �������� "{SYMB}-FIG"
	//
};
//
// Descr: ������� �������� ���������� ������� �������.
//
class TWhatmanObject;
typedef TWhatmanObject * (*FN_WTMOBJ_FACTORY)();

#define IMPLEMENT_WTMOBJ_FACTORY(symb, name) \
	struct FClsWtmo##symb { \
		static TWhatmanObject * Factory() { return new WhatmanObject##symb; } \
		FClsWtmo##symb() { TWhatmanObject::Register(#symb, name, FClsWtmo##symb::Factory); } \
	}; static FClsWtmo##symb _RegWtmo##symb;

#define WTMOBJ_REGISTER(symb, name) \
	IMPLEMENT_WTMOBJ_FACTORY(symb, name); \
	void WhatmanObject##symb::OnConstruction() { Symb = #symb; }
//
// Descr: ������� �������.
//
class TWhatmanObject { // @persistent
public:
	//
	// Descr: ��������� ������, ��������������� �������.
	//
	struct TextParam { // @persistent
		TextParam()
		{
			SetDefault();
		}
		void   SetDefault()
		{
			Side = SIDE_BOTTOM;
			Flags = 0;
			AlongSize = -1.0f;
			AcrossSize = -0.5f;
			CStyleIdent = 0;
			ParaIdent = 0;
		}
		int16  Side;        // SIDE_XXX ������� �������, � ������� ������������� �����.
			// ���� Side == SIDE_CENTER, �� ����� ������ ������������� �� ������ �������.
		uint16 Flags;       // @flags
		float  AlongSize;   // ������ ������ ����� ��� �������, � ������� �� ���������.
			// ���� �������� �������������, �� ���������� �������� ���������� ������ � ����� �� �������.
			// ������� �������� ��������, ��� ������ ������ ����� ����������� ������� ����� ���� ������.
			// ���� Side == SIDE_CENTER, �� ���� ������ ���������� ������������ �������������� �������.
		float  AcrossSize;  // ������ ������ ��������������� �������, � ������� �� ���������.
			// ���� �������� �������������, �� ���������� �������� ���������� ������ � ����� �� �������,
			// ���������������� ���, � ������� ��������� �����.
			// ������� �������� ��������, ��� ������ ������ �������������� ����������� ������� �� ���������.
			// ���� Side == SIDE_CENTER, �� ���� ������ ���������� ������������ ������������ �������.
		int32  CStyleIdent;
		int32  ParaIdent;
	};

	struct SelectObjRetBlock {
		SString WtmObjTypeSymb;
		int32  Val1;
		int32  Val2;
		SString ExtString;
	};

	friend class TWhatman;
	//
	// Descr: ������������ �����, ����������� �� TWhatmanObject.
	//
	static int SLAPI Register(const char * pSymb, const char * pName, FN_WTMOBJ_FACTORY factory);
	//
	// Descr: ������� ��������� ������, ������������ �� TWhatmanObject �� ����������������
	//   �������������� id.
	//
	static TWhatmanObject * SLAPI CreateInstance(long id);
	//
	// Descr: ������� ��������� ������, ������������ �� TWhatmanObject �� �������������������
	//   ������� pSymb.
	//
	static TWhatmanObject * SLAPI CreateInstance(const char * pSymb);
	static int GetRegSymbById(long id, SString & rSymb);
	static long GetRegIdBySymb(const char * pSymb);
	static StrAssocArray * MakeStrAssocList();

	virtual ~TWhatmanObject();
	enum {
		cmdNone = 0,
		cmdSetBounds,      // (TRect *)
		cmdSetupByTool,    // (TWhatmanToolArray::Item *)
		cmdEditTool,       // (TWhatmanToolArray::Item *)
		cmdEdit,
		cmdDblClk,
		cmdMouseHover,
		cmdGetSelRetBlock, // (TWhatmanObject::SelectObjRetBlock *)
		cmdObjInserted     // ���������� ������� ����� ����, ��� �� ��� �������� � ���������.
	};
	virtual int HandleCommand(int cmd, void * pExt);
	virtual TWhatmanObject * Dup() const;
	virtual int Draw(TCanvas2 & rCanv);
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	//
	// Descr: ����� ������� GetTextLayout()
	//
	enum {
		gtloBoundsOnly            = 0x0001, // ���� ������ ������� ��������� ������, ��
			// ������� ������� ���������� STextLayout � ������� ������ ����������
			// ������� ������.
		gtloQueryForArrangeObject = 0x0002  // ���� ���� ����������, �� ������� ������
			// ������� >0 ���� ����� ������� ������ ��� �������������� ����������
			// �������� �������� TWhatman::ArrangeObjects.
			// � ���� ������ GetTextLayout ������� ���������� ������� ������.
	};
	virtual int GetTextLayout(STextLayout & rTl, int options) const;
	//
	// Descr: ����������� ������� ������ ������������.
	//   ���� ���� pItem->Symb ������, �� ������� - ����� (�����������),
	//   � ��������� ������ ������� ��� ���������� � ���������� ������������
	//   � ��� ������ ������ ������.
	// Returns:
	//   >0 - �������������� �������� ������ �������. ������� ������� ���������.
	//   <0 - ����� �� ��������������. ������� ��������� �� �����������.
	//   0  - ������.
	//
	int    EditTool(TWhatmanToolArray::Item * pItem); // @>>HandleCommand(cmdEditTool, pItem)
	//
	// Descr: ����������� ��������� �������.
	//   >0 - �������������� ���������� ������ �������. ������� ������� ���������.
	//   <0 - ������ �� ��������������. ������� ��������� �� �����������.
	//   0  - ������.
	//
	int    Edit(); // @>>HandleCommand(cmdEdit, 0)
	//
	// Descr: ������������� ��������� ������� � ������������ � ���������
	//   ���������� �����������, ����������� �������� ���� ������ ��� ������.
	//
	int    Setup(const TWhatmanToolArray::Item * pWtaItem); // @>>HandleCommand(cmdSetupByTool, 0)
	int    SetBounds(const TRect & rRect); // @>>HandleCommand(cmdSetBounds, 0)
	int    FASTCALL Copy(const TWhatmanObject & rS);
	TRect  GetBounds() const;
	TRect  GetTextBounds() const;
	//
	// Descr: ���������� !0 ���� ����� f ����������� � ���� TWhatmanObject::Options.
	//
	int    FASTCALL HasOption(int f) const;
	int    FASTCALL HasState(int f) const;
	int    SetTextOptions(const TextParam * pParam);
	const TextParam & GetTextOptions() const;
	//
	// Descr: ��������������� �������, ������������ ������������� �����������,
	//   ��������������� �������� �������.
	//
	TRect  GetInvalidationRect() const;
	int    DrawToImage(SPaintToolBox & rTb, SImageBuffer & rImg);
	//
	// Descr: ���������� ����, ��������� ���� �������� (���� ������� �������)
	//   ������������ ������ ������.
	//
	int    Redraw();
	TWhatman * GetOwner() const;
	TWindow * GetOwnerWindow() const;

	enum {
		stCurrent  = 0x0001,
		stSelected = 0x0002
	};
	enum {
		oMovable        = 0x0001, // ������ ����� ������������ �������������
		oResizable      = 0x0002, // ������������ ����� ������ ������ �������
		oDraggable      = 0x0004, // ������ ������������ ��� Drag'n'Drop ������
		oBackground     = 0x0008, // ������� ������. ����� ������ ����� ���� ������
			// ����. ��� ������ ����� ������� �������.
			// ��� ���������� ������ ������� � ���� ���������, ���������� ������������.
		oSelectable     = 0x0010, // ������ ����� ���� ������ � ����, ����� �������� ������������
			// ����� ���������� �������.
		oFrame          = 0x0020, // �������� ������ ������� �������� ������ �����.
		oMultSelectable = 0x0040  // ������ ����� ���� ������� � ������ �������������� ������ ��������
	};
protected:
	TWhatmanObject(const char * pSymb);

	SString Symb;   //
	TextParam TextOptions;
	long   Options;
	long   State;       // @transient
private:
	TRect  Bounds;
	TWhatman * P_Owner; // @transient
};
//
// Descr: ������. ���������, ���������� ������� TWhatmanObject, ������� ����� �������������� ��
//   �������, ������������ � ��������� ������ ������� ������������.
//
class TWhatman { // @persistent
public:
	struct Param {
		enum {
			fRule               = 0x0001, // ���������� �������
			fGrid               = 0x0002, // ���������� �����
			fDisableMoveObj     = 0x0004, // ������� ������ ����������, ���� ���� ���������� ������ ��������� ���
			fDisableReszObj     = 0x0008, // ������� ������ �������� � ��������, ���� ���� ���������� ������ ��������� ���
			fSnapToGrid         = 0x0010, // ��� ����������� ��� ��������� �������� �������� ����������� �� ���������� � �������
				// (��������� ������ ���� ���������� ���� fGrid).
			fOneClickActivation = 0x0020  // ������ (TWhatmanObject::oSelectable)
				// ������������ ����� ������ ���� (������ �������� ������).
		};
		Param();

		int32  Unit;
		double UnitFactor;
		double Scale;
		long   Flags;        // @flags
		TRect  InitSize;
		TRect  ScrollMargin; // �������������� ������������ �������������� (���� � �������, ���������� ��������) //
		uint8  Reserve[64];
		SString Name;
		SString Symb;
	};
	enum {
		toolPenObjBorder = 1,
		toolPenObjBorderSel,
		toolPenObjBorderCur,
		toolPenObjRszSq,        // �������� ��� ��������� �������� �������
		toolBrushObjRszSq,      // ������� ��������� ��� ��������� �������� �������
		toolPenObjNonmovBorder, // ������� ��������������� �������
		toolPenRule,
		toolBrushRule,
		toolPenGrid,
		toolPenSubGrid
	};

	TWhatman(TWindow * pOwnerWin);
	~TWhatman();

	TPoint FASTCALL TransformPointToScreen(TPoint p) const;
	FPoint FASTCALL TransformPointToScreen(FPoint p) const;
	TPoint FASTCALL TransformScreenToPoint(TPoint p) const;

	TWindow * GetOwnerWindow() const;
	const  Param & GetParam() const;
	int    SetParam(const Param &);
	int    Clear();
	int    InsertObject(TWhatmanObject * pObj, int beforeIdx = -1);
	int    EditObject(int objIdx);
	int    RemoveObject(int idx);
	//
	// Descr: ���������� ������ � �������� idx �� �������� ����.
	//   �� ����, ������ ���������� ��������� � ������ ObjList.
	// Returns:
	//   >0 - ������ ��������� �� �������� ����.
	//   <0 - ������� ������ �� ������� ���� ������, ��� ������ idx �������
	//     �� ������� ����� ObjList ���� ������, ��� � ������ ����� ���� �������.
	//
	int    BringObjToFront(int idx);
	//
	// Descr: ���������� ������ � �������� idx �� ������ ����.
	//   �� ����, ������ ���������� ������ � ������ ObjList.
	// Returns:
	//   >0 - ������ ��������� �� �������� ����.
	//   <0 - ������� ������ �� ������� ���� ������, ��� ������ idx �������
	//     �� ������� ����� ObjList ���� ������, ��� � ������ ����� ���� �������.
	//
	int    SendObjToBack(int idx);
	int    SetCurrentObject(int idx, int * pPrevCurObjIdx);
	int    FASTCALL GetCurrentObject(int * pIdx) const;
	//
	// Descr: �������� ������ � �������� idx � ������� �������������� ������.
	// Returns:
	//   >0 - ������ ������� ��������
	//   -1 - ������ �� ����� ���� �������� ��-������� �� ����� �������� TWhatmanObject::oMultSelectable
	//   -2 - ������ �� ��������, ��-������� ��� ��������� � ������
	//    0 - ������: ���� ������ � �������� idx �� ����������, ���� ����� ������ (��������, SLERR_NOMEM).
	//
	int    AddMultSelObject(int idx);
	//
	// Descr: ������� ������ � �������� idx �� ������ �������������� ������.
	//   ���� idx == -1, �� ������ �������������� ������ ��������� ���������.
	// Returns:
	//   >0 - ������ ������� ������ �� ������, ���� (���� idx == -1) ������ ������� ������.
	//   <0 - ������ �� ���������� � ������, ���� (���� idx == -1) ������ ��� ����.
	//
	int    RmvMultSelObject(int idx);
	int    SetupMultSelBySelArea();
	//
	// Descr: ����������, ������� �� ������ � �������� idx � ������ �������������� ������.
	// Returns:
	//   !0 - ������ � �������� idx ������� � ������ �������������� ������.
	//   0  - ������ � �������� idx �� ������� � ������ �������������� ������.
	//
	int    FASTCALL IsMultSelObject(int idx) const;
	int    FASTCALL HaveMultSelObjectsOption(int f) const;
	const  LongArray * GetMultSelIdxList() const;
	int    FindObjectByPoint(TPoint p, int * pIdx) const;
	int    MoveObject(TWhatmanObject * pObj, const TRect & rRect);
	uint   GetObjectsCount() const;
	TWhatmanObject * FASTCALL GetObject(int idx);
	const  TWhatmanObject * FASTCALL GetObjectC(int idx) const;
	int    FASTCALL InvalidateObjScope(const TWhatmanObject * pObj);
	int    GetObjTextLayout(const TWhatmanObject * pObj, STextLayout & rTl, int options);

	int    Draw(TCanvas2 & rCanv);
	//
	// Descr: ������������ ������ pObj �� ������� rCanv � ����������
	//   �������������� ���������. ����������� �������� ������ ��� ���������
	//   �������, �� ����������� � ��������� this.
	//
	int    DrawSingleObject(TCanvas2 & rCanv, TWhatmanObject * pObj);
	int    DrawObjectContour(TCanvas2 & rCanv, TWhatmanObject * pObj, TPoint * pOffs);
	int    DrawMultSelContour(TCanvas2 & rCanv, TPoint * pOffs);
	int    InvalidateMultSelContour(TPoint * pOffs);
	//
	// @ARG(dir IN): SOW_XXX
	//
	int    ResizeObject(TWhatmanObject * pObj, int dir, TPoint toPoint, TRect * pResult);
	int    SetArea(TRect & rArea);
	const TRect & GetArea() const;
	//
	// ARG(mode):
	//   1 - start point
	//   2 - end point
	//   0 - reset
	//
	int    SetSelArea(TPoint p, int mode);
	const  TRect & GetSelArea() const;
	int    SetScrollPos(TPoint p);
	int    GetScrollRange(IntRange * pX, IntRange * pY) const;
	TPoint GetScrollDelta() const;
	int    SetTool(int toolId, int paintObjIdent);
	int    ArrangeObjects(const LongArray * pObjPosList, TArrangeParam & rParam);
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	int    Store(const char * pFileName);
	int    Load(const char * pFileName);
private:
	struct ObjZone {
		int    I;
		FRect  R;
	};
	//
	// Descr: ������� �������
	//
	struct RuleNotch {
		enum {
			fSnap = 0x0001 // ������� ������ "�����������" ������
		};
		float  P; // ������� �������
		float  H; // ������ ������� � ����� �� ������ �������
		long   F; // �����
	};
	//
	// Descr: ��������� �������
	//
	struct Rule {
		void   Init();
		int    AddNotch(float p, float h, long f = 0);
		int    AddNotchList(uint c, float subDelta, const float * pHiList);
		int    GetNearest(float p, float size, float * pNearestP) const;
		double OneUnitLog10;
		double OneUnitDots;
		int    ScrollDelta; // @unit{px} ����� �������������� //
		TSArray <RuleNotch> NotchList;
	};

	int    DrawObject(TCanvas2 & rCanv, TWhatmanObject * pObj);
	int    GetResizeRectList(const TWhatmanObject * pObj, ObjZone * pList) const;
	int    GetFrameRectList(const TWhatmanObject * pObj, ObjZone * pList) const;
	int    CalcRule(double ptPerInch, Rule & rResult) const;
	int    GetNotchList(const Rule & rRule, float size, float offs, int kind, TSArray <RuleNotch> & rList) const;
	int    SnapX(float p, float * pDest) const;
	int    SnapY(float p, float * pDest) const;
	int    CalcScrollRange();

	uint32 SrcFileVer;  // @transient @v9.1.9 ������ ������� �������� �����, �� �������� ��� �������� ������ ��������� �������
	TRect  Area;        // @transient ������� �������
	TRect  SelArea;     // @transient �������, ���������� ������������� ��� ������ ���������� ��������
	TRect  ScrollRange; // @transient ������������� �������������� �������� �������������� ������� ��������� //
	TPoint ScrollPos;   // @transient ������� ����������
	Rule   RuleX;       // @transient
	Rule   RuleY;       // @transient
	Param  P;
	TSCollection <TWhatmanObject> ObjList;
	int    CurObjPos; // ������� ��������� �������. -1 - ��������� ������� ���.
	LongArray * P_MultObjPosList; // ������� ��������, � ������� �������� ������������� �����.
	TWindow * P_Wnd; // @notowned @transient

	int    TidPenObjBorder;
	int    TidPenObjBorderSel;
	int    TidPenObjBorderCur;
	int    TidPenObjRszSq;
	int    TidBrushObjRszSq;
	int    TidPenObjNonmovBorder; // toolPenObjNonmovBorder
	int    TidPenRule;
	int    TidBrushRule;
	int    TidPenGrid;
	int    TidPenSubGrid;
};
//
//
//
class CtrlGroup {
public:
	CtrlGroup();
	virtual ~CtrlGroup();
	virtual void handleEvent(TDialog*, TEvent&);
	virtual int setData(TDialog*, void*);
	virtual int getData(TDialog*, void*);
private:
	friend class TDialog;
	uint   Id;
};

typedef int (*DialogPreProcFunc)(TDialog *, long extraParam);

// @v9.1.1 int IsLargeDlg(); // @v5.8.6 VADIM
//
// Descr: ��������� �������� ���� �������� ����������������� ����������
//
struct UiItemKind { // @transient
	//
	// Descr: �������� ����������� ����� ��������� ����������������� ����������
	// Attention: @persistent
	//
	enum {
		kUnkn = 0,        // ��������������
		kDialog = 1,      // @anchor ������
		kInput = 2,       // ���� �����
		kStatic,          // ����������� �����
		kPushbutton,      // ������
		kCheckbox,        // ��������� ����
		kRadioCluster,    // ������� ��������������
		kCheckCluster,    // ������� ������
		kCombobox,        // �����-����
		kListbox,         // ������ (��������, ���������������)
		kTreeListbox,     // ����������� ������
		kFrame,           // �����
		kLabel,           // ��������� ��������, ����������� � ������� ��������
		kRadiobutton,     // ����������� (����������� ������ ��� ��������� � kRadioCluster �������)

		kCount            // @anchor ����������� �������, ������ ���������� �����
	};

	static int  GetTextList(StrAssocArray & rList);
	static int  GetIdBySymb(const char * pSymb);

	UiItemKind(int kind = kUnkn);
	int    Init(int kind);

	int32  Id;
	SString Symb;
	SString Text;
	TView * P_Cls;
};

class TDialog : public TWindow {
public:
	//
	// Descr: ����� ������� TDialog::LoadDialog
	//
	enum {
		ldfDL600_Cvt = 0x0001
	};
	static int SLAPI LoadDialog(TVRez * rez, uint dialogID, TDialog * dlg, long flags);
	//
	// Descr: ������������������� �������, ������������ ��� ��������� �������� ��������.
	//
	static int SLAPI GetSymbolBody(const char * pSymb, SString & rBodyBuf);
	//
	// Descr: ����� ��������� ������� (DlgFlags)
	//
	enum {
		fCentered      = 0x0001,
		fModified      = 0x0002,
		fUserSettings  = 0x0004,
		fCascade       = 0x0008,
		fResizeable    = 0x0010,
		fMouseResizing = 0x0020,
		fLarge         = 0x0040, // ������ �������� � �������� ��� ������������� � TouchScreen
		fExport        = 0x0080, // ��������� ������� ������ ��� ��������
		// @v9.6.2 fInitModal     = 0x0100  // @v7.7.6 ���������� ����, ��������������� � ���, ��� ��� �������� ������ ��� ���������.
			// ��������� ��� ������������� �������� ���������� ������������� ������ ���� ������� �� ���������� ������ ��� ������� OK ��� Cancel.
	};

	friend  BOOL CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
	static  void centerDlg(HWND);
	int     SetCtrlFont(uint ctrlID, const char * pFontName, int height);
	int     SetCtrlFont(uint ctlID, const SFontDescr & rFd);
	int     __cdecl SetCtrlsFont(const char * pFontName, int height, ...);
	int     SetCtrlToolTip(uint ctrlID, const char * pToolTipText);
	SLAPI  TDialog(const TRect & bounds, const char * pTitle);
	SLAPI  TDialog(uint resID, DialogPreProcFunc, long extraParam);
	SLAPI  TDialog(uint resID);

	enum ConstructorOption {
		coNothing = 0,
		coExport = 1
	};
	SLAPI  TDialog(uint resID, ConstructorOption); // special constructor.

	SLAPI ~TDialog();
	virtual int    TransmitData(int dir, void * pData);
	virtual int    FASTCALL valid(ushort command);
	int    SetFont(const SFontDescr & rFd); // @v7.9.9
	void   ToCascade();
	//
	// Descr: ���������� !0 ���� � ���� ��������� ���������� ���� f.
	//
	int    FASTCALL CheckFlag(long f) const;
	int    FASTCALL addGroup(ushort grpID, CtrlGroup*);
	int    FASTCALL setGroupData(ushort, void *);
	int    FASTCALL getGroupData(ushort, void *);
	CtrlGroup * FASTCALL getGroup(ushort);
	long   getVirtButtonID(uint ctlID);
	TLabel * SLAPI getCtlLabel(uint ctlID);
	int    SLAPI getLabelText(uint ctlID, SString & rText);
	int    SLAPI setLabelText(uint ctlID, const char * pText);

	int    GetCtlSymb(uint id, SString & rBuf) const;
	int    SaveUserSettings();
	int    RestoreUserSettings();
#ifndef _WIN32_WCE // {
	int    SetDlgTrackingSize(MINMAXINFO * pMinMaxInfo);
#endif // } _WIN32_WCE
	//int    SetRealRangeInput(uint ctlID, const RealRange *);
	//int    GetRealRangeInput(uint ctlID, RealRange *);
	//int    SetPeriodInput(uint ctlID, const DateRange *);
	//int    GetPeriodInput(uint ctlID, DateRange *);
	int    FASTCALL AddClusterAssoc(uint ctlID, long pos, long val);
	//
	// Descr: �� ��, ��� � AddClusterAssoc, ��, ����� ����, ��� radio-buttons
	//   ������������� ��� �� �������� val ��� �������� �� ��������� (pos = -1)
	//
	int    FASTCALL AddClusterAssocDef(uint ctlID, long pos, long val);
	int    FASTCALL SetClusterData(uint ctlID, long);
	int    FASTCALL GetClusterData(uint ctlID, long *);
	int    FASTCALL GetClusterData(uint ctlID, int16 *);
	long   FASTCALL GetClusterData(uint ctlID);
	int    DisableClusterItem(uint ctlID, int itemNo /* 0.. */, int toDisable = 1);
	int    SetClusterItemText(uint ctlID, int itemNo /* 0.. */, const char * pText);
	int    GetClusterItemByAssoc(uint ctlID, long val, int * pPos);
	int    SetDefaultButton(uint ctlID, int setDefault);
	int    SetCtrlBitmap(uint ctlID, uint bmID);
	int    SetupInputLine(uint ctlID, TYPEID typ, long fmt);
	int    SetupSpin(uint ctlID, uint buddyCtlID, int low, int upp, int cur);
	int    SetupCalendar(uint calCtlID, uint inputCtlID, int kind);
	int    SetupCalDate(uint calCtlID, uint inputCtlID);
	int    SetupCalPeriod(uint calCtlID, uint inputCtlID);
	void   SetCtrlState(uint ctlID, uint state, bool enable);
	//
	// ����������� � ��������� ������� ����������� ������ ������, ������� ����������� �� ���� ����� ������.
	// ���� proc = 0, �� ������������ GetListFromSmartLbx
	// ���� wordSelExtra = 0 � ������� ctlID �������� ������� ��� �����������, �� wordSelExtra = (long)SmartListBox*
	//
	// int SetupWordSelector(uint ctlID, WordSelectionProc proc, long wordSelExtra, long id, int minSymbCount); // @v7.x.x AHTOXA
	int    SetupWordSelector(uint ctlID, WordSel_ExtraBlock * pExtra, long id, int minSymbCount, uint16 flags);
	int    ResetWordSelector(uint ctlID);

	TView * P_Frame;
protected:
	DECL_HANDLE_EVENT;

	struct UserSettings {
		int    Ver;
		int    Left;
		int    Top;
	};
	//
	// ������������� ��������� ���� � ������������ � �������.
	// if (DlgFlags & fUserSettings)
	//     ������� �������� ������ ���� ��������������� �� ����������� Settings.Left
	//     � Settigns.Top � ��������� �� ������� ������.
	// else if(DlgFlags & fCentered)
	//     ���� ������������ //
	// ���������� �� ������� execute()
	//
	int    setupPosition();
	//
	// ��������� �������� ���� �������
	//
	enum CtrlResizeFlags {
		crfLinkLeft   = 0x0001,
		crfLinkRight  = 0x0002,
		crfLinkTop    = 0x0004,
		crfLinkBottom = 0x0008,
		crfResizeable = 0x0010,
		crfWClusters  = 0x0020
	};
#define CRF_LINK_LEFTRIGHT        (crfLinkLeft  | crfLinkRight)
#define CRF_LINK_LEFTTOP          (crfLinkLeft  | crfLinkTop)
#define CRF_LINK_RIGHTTOP         (crfLinkRight | crfLinkTop)
#define CRF_LINK_LEFTBOTTOM       (crfLinkLeft  | crfLinkBottom)
#define CRF_LINK_RIGHTBOTTOM      (crfLinkRight | crfLinkBottom)
#define CRF_LINK_TOPBOTTOM        (crfLinkTop   | crfLinkBottom)
#define CRF_LINK_LEFTTOPBOTTOM    (crfLinkLeft  | crfLinkTop   | crfLinkBottom)
#define CRF_LINK_RIGHTTOPBOTTOM   (crfLinkRight | crfLinkTop   | crfLinkBottom)
#define CRF_LINK_LEFTRIGHTBOTTOM  (crfLinkLeft  | crfLinkRight | crfLinkBottom)
#define CRF_LINK_LEFTRIGHTTOP     (crfLinkLeft  | crfLinkRight | crfLinkTop)
#define CRF_LINK_ALL              (crfLinkLeft  | crfLinkRight | crfLinkTop | crfLinkBottom)
	//
	// Descr: ������������� ��������� ��������� �������� ��� ��������� ������� �������
	// ARG(ctrlID IN):                 ID ������������ ��������
	// ARG(xCtrl (x = l, t, r, b) IN): ��������, � ������� �������� ������� (0 - � ������� �������, -1 - �� ��������);
	//   ���� (ctrlResizeFlags & crfLinkXXX), �� �������� ������� ��������������� ��������,
	//   ����� ����� ��������������� ��������� (������� Helper_ToRecalcCtrlSet)
	//   ��� xCtrl == 0 ��������������� crfLinkXXX ��������������� �� ���������;
	// ARG(ctrlResizeFlags IN):        CtrlResizeFlags, ���� (ctrlResizeFlags | crfResizeable),
	//   �� ���������� ����� ���������� ����������, ����� ������������ ���������������
	//
	int    SetCtrlResizeParam(long ctrlID, long lCtrl, long tCtrl, long rCtrl, long bCtrl, long ctrlResizeFlags);
	int    __cdecl LinkCtrlsToDlgBorders(long ctrlResizeFlags, ...);
	int    ResizeDlgToRect(const RECT * pRect);
	int    ResizeDlgToFullScreen();

	UserSettings Settings;
	long   DlgFlags;
	void * P_PrevData;
private:
	void   SLAPI Helper_Constructor(uint resID, DialogPreProcFunc dlgPreFunc, long extraParam, ConstructorOption co); // @<<TDialog::TDialog
	uint   GrpCount;
	CtrlGroup ** PP_Groups;
	SArray * P_FontsAry;
	HWND   ToolTipsWnd;
	//
	struct ResizeParamEntry {
		long   CtrlID;
		HWND   CtrlWnd;
		long   Left;
		long   Top;
		long   Right;
		long   Bottom;
		long   Flags; // CtrlResizeFlags
	};
	TSArray <ResizeParamEntry> ResizeParamAry;
	//
	// Descr: ������������� ���������� ��������
	// ARG(firstCoord IN):           ����� 1-�� ���������� ���������, � ������� ������ ���������� �������
	// ARG(secondCoord IN):          ����� 2-�� ���������� ���������, � ������� ������ ���������� �������
	// ARG(pFirstCtrlCoord IN/OUT):  IN  - �������� �� ������ 1-�� ���������� �� ��������,
	//                               OUT - ����� 1-�� ���������� ��������
	// ARG(pSecondCtrlCoord IN/OUT): IN  - �������� �� ������ 2-�� ���������� �� ��������,
	//                               OUT - ����� 2-�� ���������� ��������
	// ARG(ctrlSize IN):             ������ ��������
	// ARG(recalcParam IN):          �������� ���������: 1 - �������� � 1-�� ����������, 2 - �������� � 2-�� ����������,
	//   3 - ���������� � ��� �� ��������� �� ����� ���������, 4 - ��������� �� ���� ��������
	//
	void   RecalcCtrlCoords(long firstCoord, long secondCoord, long * pFirstCtrlCoord, long * pSecondCtrlCoord, long ctrlSize, int recalcParam);
	int    Helper_ToRecalcCtrlSet(const RECT * pNewDlgRect, ResizeParamEntry * pCtrlParam, TSArray <ResizeParamEntry> * pCoordAry, LongArray * pCalcedCtrlAry, int isXDim);
	int    Helper_ToResizeDlg(const RECT * pNewDlgRect);
	//
	// Descr: ��������������� �������, ����������� ��� ������������ ������� �� ��������
	//
	int    InsertCtl(TView * pCtl, uint id, const char * pSymb);
	int    SetCtlSymb(uint id, const char * pSymb);
	//
	TRect  InitRect;
	RECT   ResizedRect;  // @todo RECT-->TRect
	RECT   ToResizeRect; // @todo RECT-->TRect
	StrAssocArray * P_SymbList; // ����������� ��������� ��� �������� ����������� ��������������� ��������� � �� ��������.
public:
	long   resourceID;
	int    DefInputLine;
};

#if 0 // @v7.7.7 {

struct TabbedDialogPage {
	long   id;
	TDialog * dialog;
	char * tabName;
	ushort command;
	HWND   hWnd;
};

class TabbedDialog {
public:
	static TabbedDialog * topTabbedDialog; // @global

	TabbedDialog(char *, TabbedDialogPage *, int);
	~TabbedDialog();
	HWND   setupPage(int,TDialog *);
	void   clearPage(int);
	int    findPageByHandle(HWND);
	int    findPageByID(long);
	int    current;
	TabbedDialogPage * pages;
	int    num;
	TabbedDialog * prevTabbedDialog;
	char * name;
	ushort lastCommand;
};

#endif // } 0 @v7.7.7

class TInputLine : public TView {
public:
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	TInputLine(const TRect& bounds, TYPEID aType, long fmt);
	~TInputLine();
	virtual int    TransmitData(int dir, void * pData);
	virtual void   setState(uint aState, bool enable);
	virtual int    handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	int    setupCombo(ComboBox *);
	void   setFormat(long fmt);
	long   getFormat() const;
	void   setType(TYPEID typ);
	TYPEID getType() const;
	const char * getText();
	size_t getMaxLen() const;
	void   setMaxLen(int newMaxLen);
	size_t getCaret();
	void   setCaret(size_t);
	void   getText(SString & rBuf) const;
	void   setText(const char *);
	void   disableDeleteSelection(int _disable);
	void   selectAll(int enable);

	struct Statistics {
		enum {
			fSerialized = 0x0001,
			fPaste      = 0x0002
		};
		int    SymbCount;
		long   Flags;
		double IntervalMean;
		double IntervalStdDev;
	};

	int    GetStatistics(Statistics * pStat) const;
	ComboBox * GetCombo();

	static LPCTSTR WndClsName;
protected:
	DECL_HANDLE_EVENT;
	// @v9.1.3 virtual int Paint_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// @v9.6.2 virtual void   draw();
	int    Implement_GetText();
	void   Implement_Draw();

	SString Data;
	size_t maxLen;
	TYPEID type;
	long   format;
	enum {
		stValidStr      = 0x0001,
		stDisableDelSel = 0x0002,
		stPaste         = 0x0004, // ������� ����� � ���� �������� � ���� ������, ��������� ������������ �� ������
		stSerialized    = 0x0008  // ������� ����� ��� ������ "������-��-��������"
	};
	long   InlSt;
	//
	// Descr: ���������� ����� ��������
	//
	struct InputStat {
		InputStat();
		void   Reset();
		void   CheckIn();

		clock_t Last;
		double TmSum;
		double TmSqSum;
	};
	InputStat Stat;
	ComboBox * combo;
private:
	int    Init();
	int    OnMouseWheel(int delta);
	int    OnPaste();
};

struct VirtButtonWndEx {
	VirtButtonWndEx(const char * pSignature);

    char    Signature[24]; // @anchor - ������ ��������� ������ � ������ ���������
	TDialog * P_Dlg;
	uint   FieldCtrlId;
	uint   ButtonCtrlId;
	WNDPROC PrevWndProc;
	HBITMAP HBmp;
};

class TCalcInputLine : public TInputLine {
public:
	TCalcInputLine(uint virtButtonId, uint buttonCtrlId, TRect& bounds, TYPEID aType, long fmt);
	~TCalcInputLine();
private:
	virtual int  handleWindowsMessage(UINT, WPARAM, LPARAM);
	DECL_HANDLE_EVENT;
	VirtButtonWndEx Vbwe;
	uint   VirtButtonId;
};

class TImageView : public TView {
public:
	TImageView(const TRect & rBounds, const char * pFigSymb);
	~TImageView();
	virtual int  TransmitData(int dir, void * pData);
private:
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual int  handleWindowsMessage(UINT, WPARAM, LPARAM);
	void * P_Image;
	SDrawFigure * P_Fig;
	SString FigSymb; // @v9.5.6 ������ ��������� ������ ��� ����������� //
	SColor ReplacedColor; // @v9.6.5 ���������� ���� � ��������� �����������
};

class TButton : public TView {
public:
	TButton(const TRect& bounds, const char *aTitle, ushort aCommand, ushort aFlags, uint bmpID = 0);
	~TButton();
	virtual int  handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual int  TransmitData(int dir, void * pData);
	virtual void setState(uint aState, bool enable);
	void   press(ushort = 0);
	void   drawState(bool down);
	int    makeDefault(int enable, int sendMsg = 0);
	HBITMAP GetBitmap() const;
	uint   GetBmpID() const;
	int    LoadBitmap(uint bmpID);
	int    SetBitmap(uint bmpID);
	ushort GetCommand() const;
	int    IsDefault() const;
	SString Title;
private:
	DECL_HANDLE_EVENT;

	ushort command;
	ushort flags;
	uint   BmpID;
	HBITMAP HBmp;
};

#define RADIOBUTTONS 1
#define CHECKBOXES   2

class TCluster : public TView {
public:
	TCluster(const TRect& bounds, int aKind, const StringSet & rStrings);
	~TCluster();
	virtual int  TransmitData(int dir, void * pData);
	virtual void setState(uint aState, bool enable);
	bool   mark(int item);
	void   press(ushort item);
	uint   getNumItems() const;
	int    getText(int pos, char *, uint bufLen);
	int    setText(int pos, const char *);
	void   addItem(int, const char *);
	void   deleteItem(int);
	int    disableItem(int pos /* 0.. */, int disable);
	int    isItemEnabled(int item) const; // item = ����� �������� � ������ 0..
	void   deleteAll();
	int    isChecked(ushort item) const;  // item = (ushort)GetWindowLong(hWnd, GWL_ID);
	int    isEnabled(ushort item) const;  // item = (ushort)GetWindowLong(hWnd, GWL_ID);
	//
	// ��� ������� ��� �������������� ��������� �������� � ����������� ����������.
	// pos == -1 ������������� �������� �� ���������.
	//
	int    addAssoc(long pos, long val);
	int    setDataAssoc(long);
	int    getDataAssoc(long *);
	int    getItemByAssoc(long val, int * pItem) const;
	//
	//
	//
	int    getKind() const
	{
		return (int)Kind;
	}
	// @v9.1.3 virtual int Paint_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual int handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	int16  Kind;  // RADIOBUTTONS || CHECKBOXES
	ushort Value;
	int    Sel;
	int    DisableMask;
	SStrCollection Strings;
private:
	int    column(int item) const;
	int    row(int item) const;
	LAssocArray ValAssoc;
};

class TStaticText : public TView {
public:
	TStaticText(const TRect& bounds, const char * pText = 0);
	SString & getText(SString & rBuf) const;
	int    setText(const char *);
	int    handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	SString Text;
};

class TLabel : public TStaticText {
public:
	TLabel(const TRect& bounds, const char *aText, TView *aLink);
	DECL_HANDLE_EVENT;
protected:
	TView * link;
};
#if 0 // @v9.1.5 @unused {
class TInfoPane : public TView {
public:
	TInfoPane(TRect & r);
	~TInfoPane();
	// @v9.6.2 virtual void draw();
	void setText(char *);
	int handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	char * text;
};
#endif // } 0 @v9.1.5
//
// msgbox {
//
ushort messageBox(const char *msg, ushort aOptions);
// @v9.3.4 ushort messageBox(ushort aOptions, const char *msg, ...);
// @v9.3.4 ushort messageBoxRect(const TRect &r, ushort aOptions, const char *msg, ...);
// @v9.3.4 ushort inputBox(const char *Title, const char *aLabel, char *s, uchar limit);
// @v9.3.4 ushort inputBoxRect(const TRect &bounds, const char *title, const char *aLabel, char *s, uchar limit);
//
// } msgbox
//
// void * message(TView * receiver, ushort what, ushort command, void * infoPtr);

#define CLUSTER_ID(x) ((x)&4095)
#define BUTTON_ID(x) ((x)>>12)
#define MAKE_BUTTON_ID(id,bid) ((id)+((bid)<<12))

#define DEFAULT_MAX_LEN  128
#define DEFAULT_CBX_CHAR 31      // Thick down arrow
#define DEFAULT_CBX_KEY  kbDown
//
// �������������� ���� ������ (��. srchXXX constants in SArray.H)
//
#define lbSrchByID  0x0800
//
// ����� Sign '���������� ������ ListBoxDef
//
#define LBDEFSIGN_INVALID 0x00000000 // Invalid (destroyed) instance
#define LBDEFSIGN_DEFAULT 0x1234ABCD // Instance of base class ListBoxDef or subclass hasn't own Sign
//
// Descr: ����������� �������� ������ ��� �������
//
class ListBoxDef {
public:
	//
	// Capability flags
	//
	enum {
		cTree      = 0x0001, // tree view
		cCountable = 0x0002, // ����� ����� ������� ���������� ���������� �������
		cFullInMem = 0x0004  // ������ ��������� ���������� � ������: ����� ������������ ������ ����������� ��� ������ ������.
	};

	SLAPI  ListBoxDef(uint aOptions, TYPEID aType);
	virtual SLAPI ~ListBoxDef();
	virtual void   SLAPI setViewHight(int vh);
	virtual void   SLAPI getScrollData(long * pScrollDelta, long * pScrollPos);
	virtual int    SLAPI getCurID(long * pId);
	virtual int    SLAPI getCurString(SString & rBuf);
	virtual int    SLAPI getCurData(void *);
	virtual int    SLAPI search(const void *, CompFunc, int srchMode);
	virtual int    SLAPI valid();
	virtual int    SLAPI go(long);
	virtual int    SLAPI step(long);
	virtual int    SLAPI top();
	virtual int    SLAPI bottom();
	virtual long   SLAPI getRecsCount();
	virtual int    SLAPI getIdList(LongArray &);
	virtual void * FASTCALL getRow_(long);
	//
	// Descr:
	//   (dir > 0): � ����������� �� ������ options �� ������ pData ���������� ���� ID ���� ������.
	//   (dir < 0): ��������, ��������� ListBoxDef::search(), � ������������ �
	//     options ����� � ������ ��������� ������. ���� ����� ������,
	//     �� ���������� 1, � ��������� ������ - 0.
	//
	//
	virtual int    TransmitData(int dir, void * pData);
	virtual int    SLAPI refresh();
	virtual int    SLAPI addItem(long id, const char *, long * pPos = 0);
	virtual int    SLAPI removeItem(long pos);
	virtual int    SLAPI freeAll();
	virtual int    SLAPI GetFrameSize();
	virtual int    SLAPI GetFrameState();
	const  char * SLAPI getText(long item, SString & rBuf);
	long   SLAPI _topItem() const;
	long   SLAPI _curItem() const;
	int    SLAPI _isTreeList() const;
	int    SLAPI SetOption(uint option, int set = 1);

	int    SLAPI SetUserData(const void * pData, size_t size);
	int    SLAPI GetUserData(void * pData, size_t * pSize) const;

	int    SLAPI HasCapability(long c) const { return BIN(CFlags & c); }
	int    SLAPI GetImageIdxByID(long id, long * pIDx);
	HIMAGELIST SLAPI CreateImageList(HINSTANCE hInst);
	int    SLAPI AddImageAssoc(long itemID, long imageID);
	int    SLAPI ClearImageAssocList();

	int    SLAPI SetItemColor(long itemID, SColor fgColor, SColor bckgColor);
	int    SLAPI ResetItemColor(long itemID);
	int    SLAPI HasItemColorSpec() const;
	int    SLAPI GetItemColor(long itemID, SColor * pFgColor, SColor * pBckgColor) const;
	long   SLAPI GetCapability() const
	{
		return CFlags;
	}
	StrAssocArray * GetListByPattern(const char * pText);
//protected:
	uint   Options;
	int    ViewHight;
protected:
	//
	// Descr: ������� ������������� ��������� ������������ ������������ ������. ������ �������� ������ �� ������������.
	//
	void   SetCapability(long cflags, uint32 sign);
	long   Format;
	long   ScrollDelta;
	TYPEID Type;
	long   TopItem;
	long   CurItem;
private:
	uint32 Sign;               // @v7.8.1 ������� ���������� ������. ������������ ��� ������������� ����������� ������� � ���������� �����������
		//
	long   CFlags;
	SBaseBuffer UserData;       // @obsolete
	LAssocArray ImageAssoc;    // ������ �� ��������� � ��������������� � ��� id ������
	LAssocArray ImageIdxAssoc; // ������ �� ��������� � ��������������� � ��� �������� �������� ������������ � HIMAGELIST
	struct ColorItem {
		long   Id; // �� ��������
		SColor F;  // ���� ��������
		SColor B;  // ���� ����
	};
	TSArray <ColorItem> ColorAssoc;
};

#define LBDEFSIGN_STD      0xABCD0001

class StdListBoxDef : public ListBoxDef {
public:
	SLAPI  StdListBoxDef(SArray * pArray, uint aOptions, TYPEID);
	SLAPI ~StdListBoxDef();
	virtual int    SLAPI search(const void *, CompFunc, int srchMode);
	virtual int    SLAPI valid();
	virtual long   SLAPI getRecsCount();
	virtual int    SLAPI getIdList(LongArray & rList);
	virtual void * FASTCALL getRow_(long r);
	virtual int    SLAPI GetFrameSize();
	virtual int    SLAPI GetFrameState();
	int    SLAPI setArray(SArray *);
//protected:
	SArray * P_Data;
};

#define LBDEFSIGN_STRASSOC 0xABCD0002

class StrAssocListBoxDef : public ListBoxDef {
public:
	SLAPI  StrAssocListBoxDef(StrAssocArray *, uint options);
	SLAPI ~StrAssocListBoxDef();
	virtual int    SLAPI search(const void *, CompFunc, int srchMode);
	virtual int    SLAPI valid();
	virtual long   SLAPI getRecsCount();
	virtual int    SLAPI getIdList(LongArray & rList);
	virtual void * FASTCALL getRow_(long r);
	virtual int    SLAPI GetFrameSize();
	virtual int    SLAPI GetFrameState();
	virtual int    SLAPI addItem(long id, const char *, long * pPos = 0);
	virtual int    SLAPI removeItem(long pos);
	virtual int    SLAPI freeAll();
	int    SLAPI setArray(StrAssocArray *);
	const StrAssocArray * SLAPI getArray() const { return P_Data; }
protected:
	StrAssocArray * P_Data;
private:
	SBaseBuffer OneItem; // ��������� �����, ������������ �������� getRow_()
};

#define LBDEFSIGN_STDTREE  0xABCD0003

class StdTreeListBoxDef : public ListBoxDef {
public:
	friend class SmartListBox;

	SLAPI  StdTreeListBoxDef(StrAssocArray * pList, uint aOptions, TYPEID);
	SLAPI ~StdTreeListBoxDef();
	virtual void   SLAPI setViewHight(int);
	virtual void   SLAPI getScrollData(long * pScrollDelta, long * pScrollPos);
	virtual int    SLAPI valid();
	virtual int    SLAPI go(long);
	virtual int    SLAPI step(long);
	virtual int    SLAPI top();
	virtual int    SLAPI bottom();
	virtual long   SLAPI getRecsCount();
	virtual int    SLAPI getIdList(LongArray & rList);
	virtual void * FASTCALL getRow_(long);
	virtual int    SLAPI getCurString(SString & rBuf);
	virtual int    SLAPI getCurID(long *);
	virtual int    SLAPI getCurData(void *);
	virtual int    SLAPI search(const void * pPattern, CompFunc fcmp, int srchMode);
	virtual int    SLAPI GetFrameSize();
	virtual int    SLAPI GetFrameState();
	int    SLAPI setArray(StrAssocArray *);
	int    SLAPI GetStringByID(long id, SString & rBuf);

	int    SLAPI GoByID(long id);
	int    FASTCALL HasChild(long id);
	int    SLAPI GetListByParent(long parentId, LongArray & rList) const;
	int    SLAPI GetParent(long child, long * pParent);
	int    SLAPI GetChildList(long parentId, LongArray * pChildList);
protected:
	void   SLAPI setupView();
	int    SLAPI Helper_CreateTree();
	int    SLAPI Helper_AddTreeItem(uint idx, UintHashTable & rAddedIdxList, uint32 * pPos);
private:
	StrAssocArray * P_SaList;
	struct TreeItem {
		long   Id;
		long   ParentId;
		HTREEITEM H;
		uint   P;
	};
	STree  T;
	struct Item {
		long   Id;
		long   ParentId;
		char   Txt[256];
	};
	Item   TempItem;
};

#define LBDEFSIGN_STRING 0xABCD0004

class StringListBoxDef : public StdListBoxDef {
public:
	SLAPI  StringListBoxDef(uint stringSize, uint aOptions);
	virtual int    SLAPI addItem(long id, const char *, long * pPos = 0);
	virtual int    SLAPI removeItem(long pos);
	virtual int    SLAPI freeAll();
};

#define LBDEFSIGN_DBQ    0xABCD0005
#define DEF_DBQLISTBOXDEF_FRAMESIZE 64

class DBQListBoxDef : public ListBoxDef {
public:
	SLAPI  DBQListBoxDef(DBQuery & rQuery, uint aOptions, uint aBufSize = DEF_DBQLISTBOXDEF_FRAMESIZE);
	SLAPI ~DBQListBoxDef();
	virtual void   SLAPI setViewHight(int);
	virtual void   SLAPI getScrollData(long * pScrollDelta, long * pScrollPos);
	virtual int    SLAPI valid();
	virtual int    SLAPI go(long);
	virtual int    SLAPI step(long);
	virtual int    SLAPI top();
	virtual int    SLAPI bottom();
	virtual long   SLAPI getRecsCount();
	virtual int    SLAPI getIdList(LongArray & rList);
	virtual void * FASTCALL getRow_(long);
	virtual int    TransmitData(int dir, void * pData);
	virtual int    SLAPI refresh();
	virtual int    SLAPI search(const void * pPattern, CompFunc fcmp, int srchMode);
	virtual int    SLAPI GetFrameSize();
	virtual int    SLAPI GetFrameState();
	int    SLAPI setQuery(DBQuery & rQuery, uint aBufSize = 32);
	int    SLAPI setRestrict(DBQ & rQ);
protected:
	void   SLAPI setupView();
	DBQuery * query;
};

class UiSearchTextBlock {
public:
	static int ExecDialog(HWND hWnd, uint ctlId, SString & rText, int isFirstLetter, WordSel_ExtraBlock * pBlk, int linkToList);
private:
	static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK InputCtlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	UiSearchTextBlock(HWND h, uint ctlId, char * pText, int firstLetter, WordSel_ExtraBlock * pBlk, int linkToList);
	~UiSearchTextBlock();

	HWND   H_Wnd; // SmartListBox window handle
	uint   Id;    // SmartListBox control id
	//char * P_Text;
	SString Text;
	int    IsBnClicked;
	int    FirstLetter;  // ������ ������, �� �������� ��� ������ ������, ������� ��������� � ���� ����� �����������
		// �������� ������� �������.
	WordSel_ExtraBlock * P_WordSelBlk; // not owner
	WordSelector * P_WordSel; //
	WNDPROC PrevInputCtlProc;
	int    LinkToList; // ������ ���� ����� ����������� ��������������� � ������, � ��������� ������� ����� ��� ����������� ����������� � �����������.
};
//
//
//
extern const char * SLBColumnDelim; // "/^"

class SmartListBox : public TView {
	friend class ComboBox;
	friend class ListWindow;
public:
	//
	// Descr: ����� ��������� �������
	//   ��� ������� � ��������� ����������� ����� HasState(SmartListBox::stXXX)
	//
	enum {
		stTreeList                   = 0x0001,  // ����������� ������
		stOwnerDraw                  = 0x0002,  // ������-�������� ������ ��� ��������� ������� �����������
		stDataFounded                = 0x0004,  // ������� ����, ��� (def->setData() != 0)
		stLButtonDown                = 0x0008,  // ����� ������ ���� ������ �� ������
		stInited                     = 0x0010,  // ������������ �������� SmartListBox::onInit.
		stLBIsLinkedUISrchTextBlock  = 0x0020   // ���� ������ ����� ������������ ��������������� � ������. ��� ����������� ����� ����� �������� �� ������.
	};

	SmartListBox(const TRect & rRect, ListBoxDef * pDef, int isTree = 0);
	~SmartListBox();
	void   FASTCALL setDef(ListBoxDef*);
	int    search(void * pattern, CompFunc fcmp, int srchMode);
	int    FASTCALL getCurID(long * pId);
	int    FASTCALL getCurData(void * pData);
	int    FASTCALL getCurString(SString & rBuf);
	int    getText(long itemN  /* 0.. */, SString & rBuf);
	int    getID(long itemN, long * pID);
	int    isTreeList() const;
	// @v9.6.2 virtual void   draw();
	DECL_HANDLE_EVENT;
	virtual void   selectItem(long item);
	virtual int    TransmitData(int dir, void * pData);
	virtual void   setState(uint aState, bool enable);
	//
	// Descr: ��������� ������� � ��������������� ������.
	// ARG(pos    IN): �������, � ������� ������ ���� ��������� �������.
	//   ���� (pos < 0), �� ������� ����������� � ����� ������.
	// ARG(pTitle IN): @#{vptr0} ����� ��������� �������. ���� ���������
	//   �������, �� ��������� ����� ������.
	// ARG(width  IN): ������ ������� � �������� (�� � ������). ����
	//   width == 0, �� ������ ���������� ������ ����� ������ pTitle ���
	//   1, ���� pTitle == 0.
	//   ���� widht > 255, �� ������ ������� ��������������� ������ 255.
	// ARG(format IN): �������� ������������ �������. ������ ����
	//   ���� �� ��������� ��������: 0, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER.
	//   ���� ����� ALIGN_XXX � format �� �������, �� ������� ������������� �� ������ ����.
	//
	//   ���� � ���� format ���������� ���� STRF_OEM, �� ������� �������, ���
	//   ������ pTitle �������� � OEM-���������.
	//   ���� � ���� format ���������� ���� STRF_ANSI, �� ������� �������, ���
	//   ������ pTitle �������� � ANSI-���������.
	//   ���� format �� �������� �� ����� STRF_OEM, �� ����� STRF_ANSI, ���� ��� ��� �����,
	//   �� ������� �������, ��� ������ �������� � OEM-���������.
	//
	// Returns:
	//   !0 - ������� ���������
	//   0  - ������
	//
	int    AddColumn(int pos, const char * pTitle, uint width, uint format, long ident);
	int    SearchColumnByIdent(long ident, uint * pPos) const;
	int    RemoveColumn(int pos);
	int    RemoveColumns();
	int    SetupColumns(const char * pColsBuf);
	int    GetOrgColumnsDescr(SString & rBuf) const;

	void   setHorzRange(int);
	void   setRange(long aRange);
	void   search(char * firstLetter, int srchMode);
	void   setCompFunc(CompFunc f) { SrchFunc = f; }
	int    addItem(long id, const char * s, long * pPos = 0);
	int    removeItem(long pos);
	int    freeAll();
	void   FASTCALL focusItem(long item);
	virtual int handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	int    SetupTreeWnd(HTREEITEM hParent, long parentID); // @recursion
	void   Scroll(short sbCmd, int value);
	void   CreateScrollBar(int create);
	void   SetScrollBarPos(long pos, LPARAM lParam);
	//
	// Descr: ������������� ��� ������� ��������� ������ stTreeList.
	// Returns:
	//   1 ��������� �����������
	//   0 ��������� �����
	//
	int    SetTreeListState(int yes);
	void   SetOwnerDrawState();
	void   SetLBLnkToUISrchState();
	int    HasState(long s) const;
	//
	// ���������� ���� Scrollbar � ������������ �� �������. ��� ���� ������ ���� Scrollbar ����������� � ��������� �����
	//
	void   MoveScrollBar(int autoHeight);

	ListBoxDef * def;
protected:
	int    GetStringByID(long id, SString & rBuf);
	int    GetImageIdxByID(long id, long * pIdx);
	void   SelectTreeItem();
	void   onInitDialog(int useScrollbar);
	int    FASTCALL onVKeyToItem(WPARAM wParam);
	int    GetMaxListHeight();
	void   Implement_Draw();
private:
	void   Helper_InsertColumn(uint pos);
	void   Helper_ClearTreeWnd();
	int    SetupTreeWnd2(uint32 parentP);

	struct ColumnDescr {
		uint   Width;
		uint   Format; // ALIGN_XXX
		uint   TitlePos; // ������� ������ ��������� � StrPool
		long   Ident;    // @v9.2.9 ������������� �������. ������������ �� �����������.
	};
	long   State;
	long   Range;
	long   Top;
	long   Height;
	CompFunc SrchFunc;
	uint   SrchPatternPos; // ������� ���������� ������� ������ � StrPool
	uint   ColumnsSpcPos;  // ������� ������ ������������ ������� � StrPool
	SArray Columns;
	StringSet StrPool;
	HIMAGELIST HIML;
};

class ListWindowSmartListBox : public SmartListBox {
public:
	ListWindowSmartListBox(const TRect & r, ListBoxDef * pDef, int = 0);
	ComboBox * combo;
};

class ListWindow : public TDialog {
	friend class ComboBox;
public:
	ListWindow();
	ListWindow(ListBoxDef * pDef, const char * pTitle, int);

	DECL_HANDLE_EVENT;
	void executeNM(HWND parent); // ����������� ������
	//
	// Descr: ���� ������ ���� �������� ������ ���� �������, �� ����������
	//   ��� �� �� ������ pVal.
	// Returns:
	//   !0 - ������ ���� �������� ������ ���� ������, ������� ������� �������� �� ��������� pVal
	//   0  - ���� ������ ���� �������� ����� ������ ��������, ���� �������� 0 ���������, ����
	//     �� ������� �������� ������������ ������� ������.
	//
	int    getSingle(long * pVal);
	int    getResult(long *);
	int    getString(SString & rBuf);
	int    getListData(void *);
	int    isTreeList() const;
	int    FASTCALL setDef(ListBoxDef * pDef);
	void   setCompFunc(CompFunc f);
	ListWindowSmartListBox * listBox() const;
	int    MoveWindow(HWND linkHwnd, long right);
	int    MoveWindow(RECT & rRect);
	ListBoxDef * getDef() const;
	void   SetToolbar(uint tbId);
	uint   GetToolbar() const;
protected:
	void   prepareForSearching(int firstLetter);
	ListBoxDef * def;
	ListWindowSmartListBox * P_Lb; // box;
	int    PrepareSearchLetter;
	int    IsLargeListBox;
	uint   TbId;
};

class WordSelectorSmartListBox : public ListWindowSmartListBox {
public:
	WordSelectorSmartListBox(const TRect & r, ListBoxDef * pDef);
	virtual int handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class WordSelector : public ListWindow {
public:
	WordSelector(WordSel_ExtraBlock * pBlk);
	int    FASTCALL setDef(ListBoxDef * pDef);
	int    Refresh(const char * pText);
	int    Activate();
	int    ActivateInput();
	int    CheckVisible() const;
	int    CheckActive() const;
private:
	DECL_HANDLE_EVENT;
	void   DrawListItem(TDrawItemData * pDrawItem);

	enum {
		dummyFirst = 1,
		brSel,
		brOdd,
		brBkgnd,
		clrFocus,
		clrOdd,
		clrBkgnd,
		font
	};

	int    IsVisible;
	int    IsActive;
	SPaintToolBox Ptb;
	WordSel_ExtraBlock * P_Blk; // not owner
};

ListWindow * SLAPI CreateListWindow(DBQuery & rQuery, uint options);
ListWindow * SLAPI CreateListWindow(SArray * pAry, uint options, TYPEID);
ListWindow * SLAPI CreateListWindow(StrAssocArray * pAry, uint options);
ListWindow * SLAPI CreateListWindow(uint sz, uint options);
// WordSelector * SLAPI CreateWordSelector(StrAssocArray * pAry, uint optons, UiWordSel_Helper * pHelper);
WordSelector * SLAPI CreateWordSelector(WordSel_ExtraBlock * pBlk);


class ComboBoxInputLine : public TInputLine {
public:
	ComboBoxInputLine(ushort aId);
	virtual int  TransmitData(int dir, void * pData);
};

class ComboBox : public TView {
public:
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	ComboBox(const TRect &, ushort aFlags);
	ComboBox(const TRect &, ListBoxDef * aDef);
	~ComboBox();
	void   FASTCALL setDef(ListBoxDef*);
	//
	// Descr: ������������� ������� ComboBox::Undef.
	//
	void   setUndefTag(int set);
	void   setUndefID(long undefID);
	int    setDataByUndefID();
	int    setListWindow(ListWindow * pListWin);
	int    setListWindow(ListWindow * pListWin, long val);
	ListWindow * getListWindow() const;
	void   setInputLineText(const char *);
	int    getInputLineText(char * pBuf, size_t bufLen);
	ListBoxDef * listDef() const { return P_Def; }
	void   setRange(long aRange);
	int    setupListWindow(int noUpdateSize);
	int    setupTreeListWindow(int noUpdateSize);
	void   search(const char * pFirstLetter, int srchMode);
	int    search(const void * pPattern, CompFunc fcmp, int srchMode);
	int    addItem(long id, char * pS, long * pPos = 0);
	int    removeItem(long pos);
	int    freeAll();
	TInputLine * link(void) const;
	void   SetLink(TInputLine * pLink);

	virtual void   setState(uint aState, bool enable);
	virtual int    TransmitData(int dir, void * pData);
	virtual void   selectItem(long item);

	DECL_HANDLE_EVENT;
	virtual int    handleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void   Init(long flags);
	void   Scroll(short);

	HWND   hScrollBar;
	long   Range;
	long   Flags; // cbxXXX
	long   NoDefID; // ����������� id ��� ����� ����� ������������ ������ ������ ��� �������� ������ listbox
	enum {
		stExecSemaphore = 0x0001, // ������� ����, ��� (def->setData() != 0)
		stDataFounded   = 0x0002, // ���� P_Def == 0 � � �����-����� ������� ������, �� ����������
		stUndef         = 0x0004,
		stNoDefZero     = 0x0008
	};
	long   State;
	ListBoxDef * P_Def;
	TInputLine * P_ILink;
	ListWindow * P_ListWin;
	long   Top;
	SString SearchPattern;
	SString Text;
	CompFunc SrchFunc;
};
//
// Descr: ���������, ������������ � ���������� cmDrawItem
//
#define ODT_CHECKBOX (ODT_STATIC+10) // @v5.4.10 AHTOXA
#define ODT_RADIOBTN (ODT_STATIC+11) // @v5.4.10 AHTOXA
#define ODT_EDIT     (ODT_STATIC+12) // @v5.4.10 AHTOXA

struct TDrawItemData {
	uint   CtlType;
	uint   CtlID;
	uint   ItemID;
	enum {
		iaDrawEntire = 0x0001, // =ODA_DRAWENTIRE
		iaSelect     = 0x0002, // =ODA_SELECT
		iaFocus      = 0x0004, // =ODA_FOCUS
		iaBackground = 0x0008  // ���������� ���
	};
	uint   ItemAction;
	uint   ItemState;
	HWND   H_Item;
	HDC    H_DC;
	RECT   ItemRect;
	TView * P_View;
	ulong  ItemData;
};
//
// Descr: ���������, ������������ � ���������� cmCtlColor
//
struct TDrawCtrlData {
	enum {
		cStatic = 1,
		cEdit,
		cScrollBar
	};
	int    Src;   // IN   TDrawCtrlData::cXXX ��� ������������ ��������-��������� ���������.
	HWND   H_Ctl; // IN   ����������� �������, ������� ����� �����������
	HDC    H_DC;  // IN   �������� ��������� ������������ ��������
	HBRUSH H_Br;  // OUT  �����, ������� �������� �������������� ��������� ������� //
		// ��� ��������� ������������ ��������
};

int SetWindowTransparent(HWND hWnd, int transparent /*0..100*/);

BOOL    CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK ListSearchDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK PropertySheetDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK logListProc(HWND, UINT, WPARAM, LPARAM);
//
// Toolbar
// There's a mine born by Osolotkin, 2000
//
#define TOOLBAR_ON_TOP		0
#define TOOLBAR_ON_BOTTOM	1
#define TOOLBAR_ON_LEFT		2
#define TOOLBAR_ON_RIGHT	3
#define TOOLBAR_ON_FREE		4

#define TBS_TEXT			0x0001
#define TBS_MENU			0x0002
#define TBS_AUTOSIZE		0x0004
#define TBS_NOMOVE			0x0008
#define TBS_LIST			0x0010

class TToolbar {
public:
	friend class TuneToolsDialog;

	TToolbar(HWND hw, DWORD style = 0);
	~TToolbar();
	void   DestroyHWND();
	BOOL   Valid() const;
	int    GetCurrPos() const;
	HWND   H() const;
	HWND   GetToolbarHWND() const;

	uint   getItemsCount() const;
	const  ToolbarItem & getItem(uint idx/* 0.. */) const;
	const  ToolbarItem & getItemByID(uint /* 0.. */);
	int    Init(uint res = 0, uint type = 0);
	int    Init(const ToolbarList *);
	int    Init(uint cmdID, ToolbarList * pList);
	int    Show();
	int    Hide();
	LRESULT OnMainSize(int rightSpace = 0);

	int    SaveUserSettings(uint typeID);
	int    RestoreUserSettings(uint typeID, ToolbarList * pTbList);

	int    TranslateKeyCode(ushort keyCode, uint * pCmd) const;
private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ToolbarProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK TuneToolsDlgProc(HWND, UINT, WPARAM, LPARAM);

	// Message Callback functions {
	LRESULT OnMoving(WPARAM, LPARAM);
	LRESULT OnLButtonDown(WPARAM, LPARAM);
	LRESULT OnLButtonDblclk(WPARAM, LPARAM);
	LRESULT OnNotify(WPARAM, LPARAM);
	LRESULT OnCommand(WPARAM, LPARAM);
	LRESULT OnSize(WPARAM, LPARAM);
	LRESULT OnMove(WPARAM, LPARAM);
	// }
	int    SetupToolbarWnd(DWORD style, const ToolbarList *);
	int    SetStyle(DWORD style = 0, int copy = 0);
	int    SelectMode();
	int    GetRegTbParam(uint typeID, char * pBuf, size_t bufLen);

	HWND   H_Wnd;
	HWND   H_Toolbar;
	HWND   H_MainWnd;
	HMENU  H_Menu;
	uint   VisibleCount;
	DWORD  Style;
	ToolbarList Items;
	long   Width;
	long   Height;
	// For movin'n'resizin'
	RECT   ClientRect;
	RECT   CurrRect;
	POINTS MousePoint;
	int    CurrPos;
	WNDPROC PrevToolProc;
};

#define WM_USER_KEYDOWN (WM_USER + 102)
//
//
//
//SchemaID 1(default scheme), 2 etc.
//Title color
//TitleDelim color
//Background color
//Txt color
//Cursor color
//CursorOverText color
//LineCursor color
//LineCursorOverText color
//Grid Horz color
//Grid Vert color

struct BrowserColorsSchema {   // size=42
	uint   SchemaID; // 1, 2 etc. // @v9.2.1 uint16-->uint
	COLORREF Title;
	COLORREF TitleDelim;
	COLORREF Background;
	COLORREF Text;
	COLORREF Cursor;
	COLORREF CursorOverText;
	COLORREF LineCursor;
	COLORREF LineCursorOverText;
	COLORREF GridHorizontal;
	COLORREF GridVertical;
};

#define NUMBRWCOLORSCHEMA        3
extern const BrowserColorsSchema BrwColorsSchemas[NUMBRWCOLORSCHEMA]; // @global

#define UISETTINGS_VERSION_MAJOR 1
#define UISETTINGS_VERSION_MINOR 9 // @v4.7.12 2->3 // @v5.0.7 3->4 // @v5.4.10 4->5 // @v5.6.15 5->6 // @v5.7.0 6->7 // @v7.9.9 7-->8 // @v8.1.11 8-->9
#define TOOLBAR_OFFS 100000L

class UserInterfaceSettings { // @persistent @store(WinReg[HKCU\Software\Papyrus\UI]) @size=256
public:
	static const char * SubKey;  // "Software\\Papyrus\\UI";
	enum {
		fDontExitBrowserByEsc            = 0x0001,
		fShowShortcuts                   = 0x0002,
		fAddToBasketItemCurBrwItemAsQtty = 0x0004, // @v5.5.10 ��� ���������� � ������� ����� ������� �� ������
			// � �������� ���������� ���������� ������������ ������� ������ � �������� ��������
		fShowBizScoreOnDesktop           = 0x0008, // @v5.5.11  ���������� ������ ���������� �� ������� �����
		fDisableNotFoundWindow           = 0x0010, // @v6.0.4 �� ���������� ���� "�� �������" ��� ������ � ��������
		fUpdateReminder                  = 0x0020, // ���������� ����������� �� ��������� ����������� ���������
		fTcbInterlaced                   = 0x0040, // @v7.9.3 �������������� ������ ��������� ��������� ���������� � �������������
			// ���������� �����. � ��������� ������ = �������� ������ �������.
		fShowLeftTree                    = 0x0080, // @v7.9.9 ���������� ����������� ��������� � ����� ����� ����
		fShowObjectsInLeftWindow         = 0x0100, // @unused @v8.x.x ���������� ������� �������������� ������ �������� � ����� ����� ����
		fDisableBeep                     = 0x0200, // @v8.1.6 ��������� �������� ������� (������������ ����������)
		fBasketItemFocusPckg             = 0x0400, // @v8.3.7 ��� ����� ������ �������� �������� ������� ����� ����� ������������� ��
			// ���������� �������� (� �� ������, ��� �� ���������).
		fOldModifSignSelection           = 0x0800  // @v8.5.0 ������������ ������� ������ ����� ��� ������ ��������� �����������
			// ������, ������������� �� v8.4.12 (����� ������ - ����� �����)
	};
	enum {
		wndVKDefault = 0,
		wndVKFlat    = 1,
		// @v9.2.7 wndVKKind2,
		wndVKVector  = 2, // @v9.1.12
		wndVKFancy   = 3  // @v9.2.7 �����, ����� ������������� ��� wndVKKind2 ������ ������������ wndVKFancy. �� ����� ��������,
			// ������ ��� ������ ����� �������������� ����� wndVKVector
	};
	UserInterfaceSettings();
	void   Init();
	void   SetVersion();
	uint32 GetBrwColorSchema() const;

	int    Save();
	int    Restore();
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);

	int32  Ver;
	int32  Flags;
	int    WindowViewStyle;
	int    TableViewStyle;
	int    ListElemCount;
	SFontDescr TableFont;
	SFontDescr ListFont;
	SString SupportMail;
	SString SpecialInputDeviceSymb; // @v8.1.11
};

class TStatusWin : public TWindow {
public:
	struct StItem {
		long   Icon;
		uint   Cmd;
		COLORREF Color;
		COLORREF TextColor;
		char   str[160]; // @v9.0.6 [128]-->[160] ������������ �� ������� 32-����
	};

	TStatusWin();
	int    GetRect(RECT *);
	int    AddItem(const char * pStr, long icon = 0, COLORREF color = 0, uint cmd = 0, COLORREF textColor = 0);
	int    Update();
	int    RemoveItem(int pos); //if(pos == -1) freeAll()
	uint   GetCmdByCoord(POINT coord, TStatusWin::StItem * pItem = 0);
private:
	DECL_HANDLE_EVENT;
	int    SetItem(int pos, const char *);
	TSArray <StItem> Items; // @todo �������� �� StringSet
};
//
// ���� �������
//
class ShortcutsWindow {
public:
	static INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	ShortcutsWindow();
	~ShortcutsWindow();
	HWND   Create(HWND parentWnd);
	void   Destroy();

	void   SelItem(void * ptr);
	void   AddItem(const char * pTitle, void * ptr);
	void   UpdateItem(const char * pTitle, void * ptr);
	void   DelItem(void * ptr);
	int    IsVisible() const;
	void   GetRect(RECT & rRect);
	int    MoveWindow(const RECT & rRect);

	HWND   Hwnd;
private:
	HWND   HwndTT;
};
//
// ������� ����
//
class TreeWindow {
public:
	struct ListWindowItem {
		ListWindowItem(long cmd, ListWindow * pLw);
		~ListWindowItem();
		long   Cmd;
		ListWindow * P_Lw;
	};

	TreeWindow(HWND parentWnd);
	~TreeWindow();
	static INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void   DelItemCmdList(void * ptr);
	void   UpdateItemCmdList(const char * pTitle, void * ptr);
	void   AddItemCmdList(const char * pTitle, void * ptr);
	void   Setup(HMENU hMenu);
	int    IsVisible();
	void   MoveWindow(const RECT &rRect);
	void   GetRect(RECT & rRect);
	void   Show(int show);
	void   MoveChilds(const RECT & rRect);
	void   Insert(long cmd, const char * pTitle, ListWindow * pLw);

	int    TranslateKeyCode(ushort keyCode, uint * pCmd) const;

	HWND   Hwnd;
private:
	ListWindowItem * GetListWinByCmd(long cmd, uint * pPos);
	ListWindowItem * GetListWinByHwnd(HWND hWnd, uint * pPos);

	void   MenuToList(HMENU hMenu, long parentId, StrAssocArray * pList);
	void   SetupCmdList(HMENU hMenu, HTREEITEM hP);
	void   CloseItem(HWND hWnd);
	void   SelItem(HWND hWnd);
	void   ShowList(ListWindow * pLw);
	int    OnCommand(WPARAM wParam, LPARAM lParam);

	HWND   H_CmdList;
	TSCollection<ListWindowItem> Items;
	ListWindow * P_CurLw;
	ShortcutsWindow ShortcWnd;
	TToolbar * P_Toolbar;
};
//
//
//
#define DLG_SHORTCUTS       4096
#define CTL_SHORTCUTS_ITEMS 1014
#define SHCTSTAB_MAXTEXTLEN 20
#define SPEC_TITLEWND_ID    1200 + 100

class TProgram : public TGroup {
public:
	enum {
		wndtypNone = 0,
		wndtypDialog = 1,
		wndtypChildDialog,
		wndtypListDialog
	};
	//
	// Descr: ����� ��������� State
	//
	enum {
		stUiToolBoxInited = 0x0001 // ��������� UiToolBox ��� ��������������� ������� InitUiToolBox
	};
	//
	// @todo �������� ��� ������ TProgram::GetInst �� SLS.GetHInst
	//   ���������: ��� ���� � �� ��.
	//
	static HINSTANCE GetInst();
	static void IdlePaint();
	static void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, const RECT & rDestRect, long xOffs,
		long yOffs, COLORREF cTransparentColor, COLORREF newBkgndColor, long fmt, POINT * pBmpSize);

	TProgram(HINSTANCE hInst, const char * pAppSymb, const char * pAppTitle);
	virtual ~TProgram();
	DECL_HANDLE_EVENT;
	virtual void run();
	TView * validView(TView *p);
	void   idle();
	void   SetupTreeWnd(HMENU hMenu, HTREEITEM hP);
	int    SizeMainWnd(HWND);
	int    GetStatusBarRect(RECT *);
	int    GetClientRect(RECT *);
	int    ClearStatusBar();
	int    AddStatusBarItem(const char *, long icon = 0, COLORREF = 0, uint cmd = 0, COLORREF textColor = 0);
	int    UpdateStatusBar();
	void   GotoSite();
	//
	// ���� ���� ����� ������, �� � ����������� �� ����� showSelDlg ���� ������� ������ ������ ����� ������, ���� ���������� 1.
	//
	int    ViewNewVerList(int showSelDlg);
	//
	// ARG(kind IN): 0 - BrowserWindow, 1 - STimeChunkBrowser
	//
	TBaseBrowserWindow * FindBrowser(uint resID, int kind, const char * pFileName = 0);
	int    CloseAllBrowsers();
	HWND   CreateDlg(uint dlgID, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	INT_PTR DlgBoxParam(uint dlgID, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	HBITMAP FASTCALL LoadBitmap(uint bmID);
	HBITMAP FASTCALL FetchBitmap(uint bmID);
	HBITMAP FASTCALL FetchSystemBitmap(uint bmID);
	int    AddListToTree(long cmd, const char * pTitle, ListWindow * pLw);
	int    AddItemToMenu(const char * pTitle, void * ptr);
	int    UpdateItemInMenu(const char * pTitle, void * ptr);
	int    DelItemFromMenu(void * ptr);
	int    SelectTabItem(void * ptr);
	int    SetWindowViewByKind(HWND hWnd, int wndType);
	int    DrawControl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int    EraseBackground(TView * pView, HWND hWnd, HDC hDC, int ctlType);
	//
	// Descr: ���� ��������� Windows-���������.
	//   ����������� ���� ��� �������� ������� MsgLoopIter <= 0,
	//   ���� ����� rExitSignal != 0.
	//
	int    SLAPI MsgLoop(TWindow * pV, int & rExitSignal);
	TRect  SLAPI MakeCenterRect(int width, int height) const;
	HWND   GetFrameWindow() const;
	void   NotifyFrame(int post);

	int    PushModalWindow(TWindow * pV, HWND h);
	int    PopModalWindow(TWindow * pV, HWND * pH);
	int    TestWindowForEndModal(TWindow * pV);

	int    IsTreeVisible() const;
	void   GetTreeRect(RECT & rRect);
	HWND   GetTreeHWND() const;
	//
	// Descr: ����������� �������� ���������������, ������������ ������ ���������
	// �������������� ��������. ������� ����������� ����� �������������� ������� 10,
	// �������� �������� - �������� �� 1 �� 9. ���� � ������ ������������ �����������
	// ������� � �������� ��������������� ���������, �� ������� ������������ ���������
	// �������� ������� 10, �� ����������� ��������.
	//
	enum {
		tbisBase = 0,
		tbisSelect,
		tbisFocus,
		tbisDisable,
		tbisHover,
		tbisDefault
	};
	//
	// Descr: �������������� ������������ ��������� ����������������� ����������
	//
	enum {
        tbiDummyFirst       =  1,
        //
		tbiIconRegColor     =  2, // ���������� ���� ������
		tbiIconAlertColor   =  3, //
		tbiIconAccentColor  =  4,
		tbiIconPassiveColor =  5,
        //
        tbiButtonBrush      = 10,
        tbiButtonPen        = 20,
        tbiButtonTextColor  = 30,
        tbiButtonFont       = 40,
        //
        tbiButtonBrush_F    = 50, // ������������� ��� Fancy-���������� (����� ����������� �� �����)
        tbiButtonPen_F      = 60  // ������������� ��� Fancy-���������� (����� ����������� �� �����)
	};

    int    InitUiToolBox();
	/*const*/ SPaintToolBox & GetUiToolBox() /*const*/
	{
		return UiToolBox;
	}
	const SDrawFigure * LoadDrawFigureBySymb(const char * pSymb, TWhatmanToolArray::Item * pInfo) const;

	static TProgram * application;   // @global

	TGroup   * P_DeskTop;
	TWindow  * P_TopView;
	TToolbar * P_Toolbar;
	HWND   H_MainWnd;
	HWND   H_Desktop;
	HACCEL H_Accel;
	HICON  H_Icon;
	HWND   H_ShortcutsWnd;
	HWND   H_LogWnd;
	HWND   H_CloseWnd;
	HWND   H_TopOfStack;
	TSStack <HWND> ModalStack;
	UserInterfaceSettings UICfg;
	TreeWindow * P_TreeWnd;
protected:
	virtual int  InitStatusBar();
	//
	// Descr: ����������� ����� ����� ����������� �������� ������ �����������
	//   � ��������� TWhatmanToolArray.
	//   ���� �� ��� �������� �������, �� ������ ������� �������� >0, ����
	//   �� �������� - <0.
	//   � ������ ������ - 0.
	//   ���� ���������� ��������� pT == 0, �� ����� ������ ������ ��������
	//   � ������������ �������� � ����� ����������� ����������� ��������.
	//
	virtual int  LoadVectorTools(TWhatmanToolArray * pT);
	TStatusWin * P_Stw;
	SString AppSymbol;
	SString AppTitle;
private:
	static HINSTANCE hInstance;      // @global @threadsafe
	//
	// Pattern
	// ������� ���� ������������� � ����������� ������ � �������
	// �� ������������.
	// � ���� ������� ������ ���� ����������������:
	//       P_MenuBar, P_DeskTop, P_Stw �, ��������, P_Rez.
	//
	//int    InitDeskTop(); // Pattern. Must be defined in derived class and called from constructor.
	static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
	static BOOL    CALLBACK CloseWndProc(HWND, UINT, WPARAM, LPARAM);

	SString & MakeModalStackDebugText(SString & rBuf) const;
	int    DrawButton2(HWND hwnd, DRAWITEMSTRUCT * pDi);
	int    DrawButton3(HWND hwnd, DRAWITEMSTRUCT * pDi);
	int    DrawInputLine3(HWND hwnd, DRAWITEMSTRUCT * pDi);
	int    GetDialogTextLayout(const SString & rText, int fontId, int penId, STextLayout & rTlo, int adj);

	long   State;
	WNDPROC PrevCloseWndProc;
	HWND   H_FrameWnd;
	TBitmapHash BmH;
	SPaintToolBox UiToolBox; // @v9.1.5 ����� ������������ ��� ��������� ����������� ����������������� ����������.
	TWhatmanToolArray DvToolList; // @v9.1.9 ��������� �����������, ����������� �� �������� �����
};

#define APPL    (TProgram::application)

struct TBButtonCfg { // size = 4
	uint16 KeyCode;
	uint8  State;
	uint8  Style;
};

struct ToolbarCfg { // size = sizeof(uint16) + Count * sizeof(TBButton)
	ToolbarCfg();
	~ToolbarCfg();
	int    Init();
	int    Init(const void * pBuf);
	size_t GetSize() const { return (sizeof(Count) + Count * sizeof(TBButtonCfg)); }
	int    GetBuf(void ** ppBuf, size_t bufLen) const;

	uint16 Count;
	uint16 Reserve; // @v9.3.4 @alignment
	TBButtonCfg * P_Buttons;
};
//
//
//
class STooltip {
public:
	STooltip();
	~STooltip();
	int    Init(HWND parent);
	int    Destroy();
	int    Add(const char * pText, const RECT * pRect, long id);
	int    Remove(long id);
private:
	HWND   HwndTT;
	HWND   Parent;
};

class SMessageWindow {
public:
	enum {
		fShowOnCenter      = 0x00000001,
		fShowOnCursor      = 0x00000002,
		//
		fTextAlignLeft     = 0x00000010,
		//
		//
		//
		fCloseOnMouseLeave = 0x00000100,
		fOpaque            = 0x00000200, // �� ���������� ���
		fSizeByText        = 0x00000400,
		fChildWindow       = 0x00000800,
		fTopmost           = 0x00001000,
		fPreserveFocus     = 0x00002000,
		fLargeText         = 0x00004000, // ������� ����� (default * 2)
		fMaxImgSize        = 0x00008000  // @v9.5.10 ������������ ������ ���� ��� ���������� ����������� ��������
	};

	//
	// Descr: ��������� ��� ���� ���������, ������� ����� ������������ ���� parent.
	//   ���� parent == 0, �� ��������� ��� ����, ������� ���� �������.
	//
	static int DestroyByParent(HWND parent);

	SMessageWindow();
	~SMessageWindow();

	int    Open(SString & rText, const char * pImgPath, HWND parent, long cmd, long timer, COLORREF color, long flags, long extra);
	int    Destroy();
	int    Paint();
	int    Move();
	int    DoCommand(TPoint p);
	void * GetImage()
	{
		return P_Image;
	}
	COLORREF Color;
	HBRUSH   Brush;
	WNDPROC PrevImgProc;
private:
	static BOOL CALLBACK Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	int    SetFont(HWND hCtl);

	long   Cmd;
	long   Flags;
	long   Extra;
	POINT  PrevMouseCoord;
	SString Text;
	SString ImgPath;
	HWND   HWnd;
	HFONT  Font;
	void * P_Image;
};

//
//
//
#define MIN_COLUMN_WIDTH  8 // @v6.2.10 2-->8

// @v8.7.2 SPTR2DEF(SArray);
// @v8.7.2 SCLASSDEF(DBQuery);
// @v8.7.2 SCLASSDEF(TVRez);
// @v8.7.2 SSTRUCTDEF(BroColumn);
// @v8.7.2 SSTRUCTDEF(BroGroup);
// @v8.7.2 SCLASSDEF(BrowserDef);
// @v8.7.2 SCLASSDEF(BrowserView);

#define CLASSNAME_DESKTOPWINDOW "PPYDESKTOPWINDOW"

struct SBrowserDataProcBlock {
	int    SetZero();
	int    FASTCALL Set(int32 i);
	int    Set(double i);
	int    FASTCALL Set(const char * pS);
	int    FASTCALL Set(const SString & rS);
	int    FASTCALL Set(LDATE dt);
	int    FASTCALL Set(LTIME tm);

	void * ExtraPtr;         // IN
	int    ColumnN;          // IN
	TYPEID TypeID;           // IN
	const  void * P_SrcData; // IN
	uint32 Color;            // OUT
	void * P_DestData;       // OUT
	SString TempBuf;         // Helper ����� �������������� ����������� ������� SBrowserDataProc
		// ��� ��������� ������ (����� �������������� ���� ���)
};

typedef int (*SBrowserDataProc)(SBrowserDataProcBlock * pBlk);

struct BroColumn {
	BroColumn();
	// @nodestructor
	TYPEID T;              // Data type
	uint   Offs;           // Offset from begining of row
	SBrowserDataProc UserProc;
	long   format;         // Output format
	uint   Options;        //
	char * text;           // Column's title
	uint   width;          // Width of display field (Internal use)
	uint   x;              // Internal use
	uint   index;          // Internal use
	uint   OrgOffs;        // �������� ����, �������� ��� �������� �������
};

struct BroGroup {
	SLAPI  BroGroup();
	uint   SLAPI NextColumn() const;

	uint   first;
	uint   count;
	uint   hight;
	char * text;
	uint   index;    // Internal use
};

struct BroCrosstab {
	TYPEID Type;
	long   Format;   // Output format
	uint   Options;
	char * P_Text;   // Column's title
	uint   Width;    // Width of display field (Internal use)
};

class BrowserDef : public SArray {
public:
	SLAPI  BrowserDef(int captionHight, uint aOptions, void * extraPtr = 0);
	SLAPI ~BrowserDef();
	//
	// Descr: ������� ������ ��������� ������� � ����� ������ �������
	// ARG(atPos IN): �������, � ������� ����������� �������. ���� atPos < 0, �� �������
	//   ����������� � ����� �������
	// ARG(pTxt  IN): ��������� �������. ����� ��������� ������� \n ��� �������� �� ����� ������
	// ARG(fldNo IN): ����� ������� � ��������� ������ ��� �������� (� ����������� �� ���������)
	// ARG(typ   IN): ��� ������. ���� typ == 0, ������� �������� �������������� ����������� ���
	//   ������ �� �������, ���������� ���������� fldNo
	// ARG(fmt   IN): ������ ������ ������ (����������� MKSFMT ��� MKSFMTD)
	// ARG(opt   IN): ����� ������ ������ � ������� (BCO_XXX see tvdefs.h)
	//
	virtual int   SLAPI insertColumn(int atPos, const char * pTxt, uint fldNo, TYPEID typ, long fmt, uint opt);
	virtual int   SLAPI insertColumn(int atPos, const char * pTxt, const char * pFldName, TYPEID typ, long fmt, uint opt);
	virtual void  SLAPI setViewHight(int);
	virtual void  SLAPI getScrollData(long * pScrollDelta, long * pScrollPos);
	virtual int   SLAPI initOffset(int);
	virtual int   SLAPI valid();
	virtual int   FASTCALL go(long);
	virtual int   FASTCALL step(long);
	virtual int   SLAPI top();
	virtual int   SLAPI bottom();
	virtual long  SLAPI getRecsCount();
	virtual void * FASTCALL getRow(long);
	virtual int   FASTCALL getData(void *);
	virtual int   FASTCALL setData(void *);
	virtual int   SLAPI refresh();
	virtual int   SLAPI search(void *, CompFunc, int srchMode, int srchCol);
	virtual int   SLAPI search2(const void * pSrchData, CompFunc, int srchMode, size_t offs);

	BroColumn & FASTCALL at(uint) const;
	int    SLAPI addColumn(const BroColumn *, int = UNDEF);
	int    SLAPI removeColumn(int);
	int    SLAPI setColumnTitle(int colN, const char * pText);
	int    SLAPI addGroup(BroGroup *);
	const  BroGroup * SLAPI groupOf(uint column, uint * pGrpPos = 0) const;
	uint   SLAPI groupWidth(uint group, uint atColumn) const;
	uint   SLAPI groupWidth(const BroGroup *, uint atColumn) const;
	int    SLAPI GetCellData(long row, int column, TYPEID * pType, void * pDataBuf, size_t dataBufLen);
	char * SLAPI getText(long row, int column, char * pBuf);
	//
	// Descr: ��������� ����� ��������� (512 ��������), ���������� �� ������ ������� � ��������
	//
	SString & SLAPI getFullText(long row, int column, SString & rBuf);
	SString & SLAPI getFullText(const void * pRowData, int column, SString & rBuf);
	char * SLAPI getMultiLinesText(long, int, char *, uint = 0, uint * = 0);
	int    SLAPI setText(long, int, const char *);
	long   SLAPI _topItem() const { return topItem; }
	long   SLAPI _curItem() const { return curItem; }
	int    FASTCALL isColInGroup(uint col, uint * idx) const;
	int    SLAPI GetCapHeight() const;
	void   SLAPI VerifyCapHeight();
	uint   SLAPI GetGroupCount() const;
	const  BroGroup * FASTCALL GetGroup(uint) const;

	void   SLAPI ClearGroupIndexies();
	uint * SLAPI GetGroupIndexPtr(uint grpN);

	int    SLAPI AddCrosstab(BroCrosstab *);
	uint   SLAPI GetCrosstabCount() const;
	const  BroCrosstab * SLAPI GetCrosstab(uint) const;
	int    SLAPI FreeAllCrosstab();

	int    SLAPI IsBOQ() const;
	int    SLAPI IsEOQ() const;
	int    SLAPI CheckFlag(uint) const;

	void   SLAPI SetUserProc(SBrowserDataProc proc, void * extraPtr)
	{
		UserProc = proc;
		ExtraPtr = extraPtr;
	}
protected:
	SBrowserDataProc UserProc;
	void * ExtraPtr;

	int    capHight;
	uint   NumGroups;
	BroGroup * P_Groups;
	int    viewHight;
	long   scrollDelta;
	int    isBOQ;
	int    isEOQ;
	long   topItem;
	long   curItem;
private:
	SArray * P_CtList;         // ������ �����-��� ��������
	SBrowserDataProcBlock DpB;
public:
	uint   options;

	void   FASTCALL freeItem(void *);
};

class AryBrowserDef : public BrowserDef {
public:
	SLAPI   AryBrowserDef(SArray * pData, const BNFieldList * pFl, int captionHight, uint aOptions, void * extraPtr = 0);
	SLAPI  ~AryBrowserDef();
	int     SLAPI setArray(SArray * pData, const BNFieldList * pFl, int setupPosition /*= 1*/);
	const   SArray * SLAPI getArray() const;
	virtual int   SLAPI valid();
	virtual int   SLAPI insertColumn(int atPos, const char * pTxt, uint fldNo, TYPEID typ, long fmt, uint opt);
	virtual long  SLAPI getRecsCount();
	virtual void * FASTCALL getRow(long);
	virtual int   FASTCALL getData(void *);
	virtual int   FASTCALL setData(void *);
protected:
	SArray * P_Array;
	BNFieldList * P_Fields;
};
//
//
//
#define DEFDBQFRAMESIZE 100

class DBQBrowserDef : public BrowserDef {
public:
	SLAPI  DBQBrowserDef(DBQuery & rQuery, int captionHight, uint aOptions, uint aBufSize = DEFDBQFRAMESIZE);
	SLAPI ~DBQBrowserDef();
	const  DBQuery * SLAPI getQuery() const { return query; }
	int    SLAPI setQuery(DBQuery & rQuery, uint aBufSize = DEFDBQFRAMESIZE);
	virtual int   SLAPI insertColumn(int atPos, const char * pTxt, uint fldNo, TYPEID, long fmt, uint opt);
	virtual int   SLAPI insertColumn(int atPos, const char * pTxt, const char * pFldName, TYPEID typ, long fmt, uint opt);
	virtual void  SLAPI setViewHight(int);
	virtual void  SLAPI getScrollData(long * pScrollDelta, long * pScrollPos);
	virtual int   SLAPI valid();
	virtual int   FASTCALL go(long);
	virtual int   FASTCALL step(long);
	virtual int   SLAPI top();
	virtual int   SLAPI bottom();
	virtual long  SLAPI getRecsCount();
	virtual void * FASTCALL getRow(long);
	virtual int   FASTCALL getData(void *);
	virtual int   FASTCALL setData(void *);
	virtual int   SLAPI refresh();
protected:
	void   SLAPI setupView();
	DBQuery * query;
};

#ifdef _TURBOVISION // {
//
// Messages
//
#define BRO_GETCURREC     WM_USER+1
#define BRO_GETCURCOL     WM_USER+2
#define BRO_DATACHG       WM_USER+3
#define BRO_SETDATA       WM_USER+4 // LPARAM = far ptr to new data
	// WPARAM = parameter for BrowseDef::setData virtual member function
//
// Next messages are sending to parent window
//
#define BRO_ROWCHANGED    WM_USER+5 // WPARAM = HWND, LPARAM = MAKELPARAM(Vert Scroll Pos, 0)
#define BRO_COLCHANGED    WM_USER+6 // WPARAM = HWND, LPARAM = MAKELPARAM(Horz Scroll Pos, 0)
#define BRO_LDBLCLKNOTIFY WM_USER+7 // WPARAM = HWND, LPARAM = MAKELPARAM(xPos, yPos)
#define BRO_RDBLCLKNOTIFY WM_USER+8 // WPARAM = HWND, LPARAM = MAKELPARAM(xPos, yPos)

#define MAXCAP            64 // ������������ ����� ��������� ������� ��� ������
#define MAXDEPS           32 // ������������ ���������� �������� ������������ ������� ���� bcoCalc
#define BRWCLASS_CEXTRA    0 // �������������� ������ ������ "BROWSE"
#define BRWCLASS_WEXTRA    8 // �������������� ������ ���� ������ "BROWSE"
#define BRWL_USERDATA      4 // �������� � BrowseWindow ��� ������ ������������ //

struct BrowserRectCursors {
	RECT   CellCursor;
	RECT   LineCursor;
};

struct BrowserPens {
	BrowserPens();
	void   Destroy();

	HPEN   GridHorzPen;
	HPEN   GridVertPen;
	HPEN   DrawFocusPen;
	HPEN   ClearFocusPen;
	HPEN   TitlePen;
	HPEN   FocusOuterPen;
	HPEN   DefPen;
};

struct BrowserBrushes {
	BrowserBrushes();
	void   Destroy();

	HBRUSH DrawBrush;
	HBRUSH ClearBrush;
	HBRUSH TitleBrush;
	HBRUSH DefBrush;
	HBRUSH CursorBrush;
};

struct RowHeightInfo {
	long   Top;
	uint   HeightMult;
};

HWND FASTCALL GetNextBrowser(HWND hw, int reverse);
// HWND GetPrevBrowser(HWND hw);

class TBaseBrowserWindow : public TWindow {
public:
	struct IdentBlock {
		int    IdBias;
		SString ClsName;
		SString InstanceIdent;
	};
	virtual TBaseBrowserWindow::IdentBlock & GetIdentBlock(TBaseBrowserWindow::IdentBlock & rBlk);
	//
	// Descr: ������� ���� � ��������� ��� � ����� ���� ���� � ����������� ������.
	//   ���� ������� �������� ����� �� �������� APPL->H_MainWnd, �� ���� ����������� //
	//   � ��������� ������.
	//   ��������: � ������, ���� ���� ���� �������� � ��������� ������, ����� ���������� //
	//   ������� ��� �������� �����������, �� ���� ��������� this ������ ���� ������������.
	// Returns:
	//   <0 - ���� ���� ��������� � ����������� ������ � �������� �� ������
	//        ����� ������ �� �������.
	//   >0 - ���� ���� ��������� � ��������� ������. ���� ��������� ������������ //
	//        �������, �� ������� ��� �������� ��������� ����.
	//   0  - ������.
	//
	int    Insert();
	uint   SLAPI GetResID() const;
	void   SLAPI SetResID(uint res);
	void   SLAPI SetToolbarID(uint toolbarID);

	enum {
		IdBiasBrowser          = 0x00100000,
		IdBiasTimeChunkBrowser = 0x00200000,
		// @v7.0.0 @obsolete IdBiasPaintCloth       = 0x00400000
		IdBiasTextBrowser      = 0x00800000
	};
protected:
	TBaseBrowserWindow(LPCTSTR pWndClsName);
	DECL_HANDLE_EVENT;

	enum {
		bbsIsMDI        = 0x00000001,
		bbsDataOwner    = 0x00000002, // ������ ������� ����������� ��-��� �������.
		bbsWoScrollbars = 0x00000004
		// ������� � 0x00010000 ����� ��������������� �� ������������ ��������
	};
	uint    ToolbarID;   // ID Toolbar'a ��� ���������� � ������� = LastCmd
		// (������� �� ������� ��� ������� ������ �������) + TOOLBAR_OFFS (��������)
	SString ClsName;     // Window class name
	uint   ResourceID;
	TPoint PrevMouseCoord;
	long   BbState;
};

class BrowserWindow : public TBaseBrowserWindow {
public:
	struct CellStyle {
		enum {
			fCorner           = 0x0001,
			fLeftBottomCorner = 0x0002,
			fRightFigCircle   = 0x0004
		};

		COLORREF Color;
		COLORREF Color2; // ���� ��� ������� ������ ����
		COLORREF RightFigColor; // @v8.9.5 ���� ������ � ������ ����� ������
		long   Flags;
	};

	typedef int (* CellStyleFunc)(const void * pData, long col, int paintAction, CellStyle *, void * extraPtr);

	static int RegWindowClass(HINSTANCE hInst);
	static LRESULT CALLBACK BrowserWndProc(HWND, UINT, WPARAM, LPARAM);

	static LPCTSTR WndClsName;

	SLAPI  BrowserWindow(uint resID, DBQuery *, uint broDefOptions = 0);
	SLAPI  BrowserWindow(uint resID, SArray *, uint broDefOptions = 0);
	SLAPI ~BrowserWindow();
	//
	// Descr: ������ ������ �, ��������, ��������� ������ ������ ������� ��� �����������.
	//   ���� resID �� ����� �������� �������� ResourceID, �� ��������� ������ resID.
	// Returns:
	//   1 - ������ pQuery ��� ����������, �� ������ resID �� ���������� �� this->ResourceID.
	//   2 - ������ pQuery ��� ���������� � �������� ������ resID
	//   0 - ������
	//
	int    ChangeResource(uint resID, DBQuery * pQuery, int force = 0);
	int    ChangeResource(uint resID, SArray * pArray, int force = 0);
	int    LoadToolbar(uint toolbarId);
	void   CalcRight();
	void   SetupScroll();
	int    insertColumn(int atPos, const char * pTxt, uint fldNo, TYPEID typ, long fmt, uint opt);
		// @>>BrowserDef::insertColumn(in, const char *, uint, TYPEID, long, uint)
	int    insertColumn(int atPos, const char * pTxt, const char * pFldName, TYPEID typ, long fmt, uint opt);
	int    removeColumn(int atPos);
	void   SetColumnWidth(int colNo, int width);
	void   SetupColumnWidth(uint colNo);
	int    SetColumnTitle(int conNo, const char * pText);
	void   SetFreeze(uint);
	LPRECT ItemRect(int hPos, int vPos, LPRECT, BOOL isFocus);
	LPRECT LineRect(int vPos, LPRECT, BOOL isFocus);
	void   ItemByPoint(TPoint point, long * pHorzPos, long * pVertPos);
	void   ItemByMousePos(long * pHorzPos, long * pVertPos);
	//
	// ARG(action IN):
	//   -1 - clear (alt + left button down)
	//   0  - clear, add one column (left button down)
	//   1  - add column (ctrl+left button down)
	//
	int    SelColByPoint(LPPOINT, int action);
	void   FocusItem(int hPos, int vPos);
	int    CheckResizePos(LPPOINT);
	int    IsResizePos(TPoint);
	void   Resize(TPoint p, int mode); // mode: 0 - toggle off, 1 - toggle on, 2 - process
	int    refresh();
	void   init(BrowserDef * pDef = 0);
	void   initWin();
	BrowserDef * getDef();
	int    SetDefUserProc(SBrowserDataProc proc, void * extraPtr);
	void   go(long p);
	void   top();
	void   bottom();
	void   setRange(ushort aRange);
	void   search(char * pFirstLetter, int srchMode);
	int    search2(void * pSrchData, CompFunc, int srchMode, size_t offs);
	//
	// Descr: ���������� ������� ������� � �������.
	// Returns:
	//   ����� ������� ������� [0..P_Def->getCount()-1]
	//
	int    GetCurColumn() const;
	int    SetCurColumn(int col);
	void   setInitPos(long p);
	void   SetColorsSchema(uint32 schemaNum);
	long   CalcHdrWidth(int plusToolbar = 0) const;
	int    CopyToClipboard();
	//
	// For modeless
	//
	void * SLAPI getCurItem();
	void * SLAPI getItemByPos(long pos);
	//
	// Descr: ��������� � ������� ��������� �������, ������������� �������������.
	// ARG(ifChangedOnly IN): ���� !0, �� ��������� ����� ��������� ������ � ������, ����
	//   ���� ������������� ��������� (���������� IsUserSettingsChanged).
	//
	int    SaveUserSettings(int ifChangedOnly);
	int    RestoreUserSettings();
	const  UserInterfaceSettings * GetUIConfig() const;
	int    SetCellStyleFunc(CellStyleFunc, void * extraPtr);
	//
	// Descr: ���������� ���� ��������� ������
	//
	int    GetCellColor(long row, long col, COLORREF * pColor);
	uint   GetRezID() const { return RezID; } // @v6.4.8 AHTOXA

	BrowserWindow * view;
	enum {
		paintFocused = 0,
		paintNormal  = 1,
		paintClear   = 2
	};
protected:
	DECL_HANDLE_EVENT;

	int    WMHScroll(int sbType, int sbEvent, int thumbPos);
	int    WMHScrollMult(int sbEvent, int thumbPos, long * pOldTop);
	int    LoadResource(uint, void *, int, uint uOptions = 0);

	uint   RezID;
	TToolbar * P_Toolbar;
private:
	virtual void Insert_(TView *p);
	virtual TBaseBrowserWindow::IdentBlock & GetIdentBlock(TBaseBrowserWindow::IdentBlock & rBlk);
	void   WMHCreate(LPCREATESTRUCT);
	int    IsLastPage(uint viewHeight); // AHTOXA
	void   ClearFocusRect(LPRECT);
	void   DrawCapBk(HDC, LPRECT, BOOL);
	void   DrawFocus(HDC, LPRECT, BOOL DrawOrClear, BOOL isCellCursor = 0);
	void   Paint();
	void   PaintCaption(HDC);
	//
	// col = -1 - ������������ ������� �������
	//
	int    PaintCell(HDC hdc, RECT r, long row, long col, int paintAction);
	int    search(void * pPattern, CompFunc fcmp, int srchMode);
	int    DrawTextUnderCursor(HDC hdc, char * pBuf, RECT * pTextRect, int fmt, int isLineCursor);
	void   AdjustCursorsForHdr();
	int    CalcRowsHeight(long topItem, long bottom = 0);
	int    DrawMultiLinesText(HDC hdc, char * pBuf, RECT * pTextRect, int fmt);
	int    FASTCALL CellRight(const BroColumn & rC) const;
	int    FASTCALL GetRowHeightMult(long row) const;
	int    FASTCALL GetRowTop(long row) const;

	long   InitPos;
	TView * P_Header;
	int    ToolBarWidth;
	BrowserDef * P_Def;
	LOGFONT FontRec;
	HGDIOBJ Font;
	BrowserPens Pens;
	BrowserBrushes Brushes;
	HFONT   DefFont;
	HCURSOR MainCursor;
	HCURSOR ResizeCursor;
	BrowserRectCursors RectCursors;
	int    ResizedCol;
	TPoint CliSz;   // ������ ���������� ������� ����
	TPoint ChrSz;   // ������� ������ ��������.
	int    YCell;   // Hight of cell
	int    CapOffs; // Offset from top of window to begining of rows
	uint   Left;
	uint   Right;
	uint   Freeze;
	int    IsUserSettingsChanged;
	uint   ViewHeight;
	uint   VScrollMax;
	uint   VScrollPos;
	uint   HScrollMax;
	uint   HScrollPos;
	SString SearchPattern;
	CompFunc SrchFunc;

	CellStyleFunc F_CellStyle;
	void * CellStyleFuncExtraPtr;

	int    LastResizeColumnPos; // ������������ � ������ Resize()
	SArray * P_RowsHeightAry;   // ������ ����� ������� �������� (��� ������������� ���������)
	UserInterfaceSettings UICfg;
	LongArray SelectedColumns;  // ��������� �������, ��� ����������� � ����� ������
};
//
//
//
class STimeChunkBrowser : public TBaseBrowserWindow {
public:
	struct Param {
		Param();
		Param & Clear();
		int    SetScrPeriod(const DateRange & rPeriod);
		enum {
			fSnapToQuant = 0x0001, // ��� �������� ��� �������������� �������� ����� ��������� �� ���������� ������
			fUseToolTip  = 0x0002, // ���������� ��������� ��� ��������� ������� �� ��������������
			fInterlaced  = 0x0004  // @v7.9.3 ���������� �������������� ������ � ������������� �������� �����
		};
		enum {
			vPrcTime = 0, // ����� - ����������, ������ - ����� //
			vHourDay      // ����� - ����, ������ - ��� //
		};
		uint   Quant;          // ����� ��������� ������� (������)
			// ��� vHourDay - ����� ������ ��� (������)
		uint   PixQuant;       // ����� ��������� ������� (� ��������)
			// ��� vHourDay - ����� ������ ��� (� ��������)
		uint   PixRow;         // ������ ����� ������ (� ��������)
		uint   PixRowMargin;   // ������������ ���� ������ � ����� ������� (� ��������)
		uint   HdrLevelHeight; // ������ ��������� ������ ������ ��������� (� ��������)
		uint   TextZonePart;   // ���������� ���� ����� (���������) ������ ����
		DateRange DefBounds;   // ������� ������� �� ���������.
		long   ViewType;       // @v7.5.0 Param::vXXX ��� ����������� //
		DateRange ScrPeriod;   // @v7.5.0 ������, ������� ������ ������������ � ���� (�� ��� X).
		long   DaysCount;      // @v7.6.10 (���������������) ���������� ����, ������������ �� ��� X � ������ vHourDay.
			// default=7
		long   SingleRowIdx;   // @v7.6.12 �������� � ������ vHourDay ������������ ������ P_Data � �������� SingleRowIdx. -1 - undefined
		long   Flags;
		SString RegSaveParam;  // ��� ���������, �� �������� ������ ���� ��������� ���������
			// ������� � ��������� �������
	};
	//
	// Descr: ��������� �������� ����� � ������� ����
	//
	struct Loc {
		enum {
			kWorkspace = 1,
			kChunk,
			kHeader,
			kLeftZone,
			kLeftHeader,     //
			kSeparator,      // ����������� ����� � ������ ������
			kPicMode         // ������ ������������ ������ ���������
		};
		enum {
			pInner = 1,      // ���������� ����� �������
			pLeftEdge,       // ����� �����
			pTopEdge,        // ������� �����
			pRightEdge,      // ������ �����
			pBottomEdge,     // ������ �����
			pMoveSpot        // ����� � �������� �������, ��� ������� �� ������� ����� ���������� �������
		};
		union {
			long   RowId;    // ������������� ������ �����, ������� ������������� �����
			long   HdrLevel; // ������ ������ ���������, �������� ������������� �����
		};
		long   EntryId;      // ������������� �������, �������� ������������� �����
		STimeChunk Chunk;    // �������� ���������� ������� EntryId
		LDATETIME Tm;        // "������" �����, �������� ������������� �����
		LDATETIME TmQuant;   // �����, ����������� �� ������ (� ������� �������), �������� ������������� �����
		long   Kind;         // kXXX ��� �������
		long   Pos;          // pXXX ����������� ���������������� ����� � �������� �������
		long   Flags;        //
	};
	static int RegWindowClass(HINSTANCE hInst);
	static const char * WndClsName;

	STimeChunkBrowser();
	~STimeChunkBrowser();

	enum {
		bmpModeGantt = 1,
		bmpModeHourDay,
		bmpBack
	};
	void   SetBmpId(int ident, uint bmpId)
	{
		switch(ident) {
			case bmpModeGantt: BmpId_ModeGantt = bmpId; break;
			case bmpModeHourDay: BmpId_ModeHourDay = bmpId; break;
			case bmpBack: BmpId_Back = bmpId; break;
		}
	}
	int    SetParam(const Param *);
	int    SetData(STimeChunkGrid *, int takeAnOwnership);
	int    FASTCALL IsKeepingData(const STimeChunkGrid *) const;
	//
	// Descr: ��������� ������, ���������� ��� ���������
	//
	int    UpdateData();
	int    Locate(TPoint p, Loc * pLoc) const;
	int    RestoreParameters(STimeChunkBrowser::Param & rParam);
private:
	struct ResizeState {
		void   Setup(int kind, TPoint p);
		enum {
			kNone = 0,    // ��� ������ ��������� ������� (��������)
			kRescale = 1, // ��������� �������� �������� ����� ����� �������� ���������
			kSplit,       // ��������� ������ ����� ����� ����
			kChunkLeft,   // ��������� ����� ������� �������� ����� ����� �������
			kChunkRight,  // ��������� ����� ������� �������� ������ ����� �������
			kMoveChunk,   // ����������� �������
			kRowHeight,   // ������ ������
			kScroll,      // ��������� �������� ����� ����� � ������� �������
			kSwitchMode   // ������������ ������ ����������� //
		};
		int16  Kind;
		int16  HdrLevel;     // ������� ���������, �� ������� ���������� ��������� ��������
		union {
			long   Quant;    // ��� �������� ��������� �� �����, ����� ����� �������� ��������� �����.
			long   ChunkId;  // �������, ������ �������� �������� //
		};
		long   RowId;        // �� ������, � ������� ��������� � ������� ������ ������������ �������
		long   Shift;        // ������� �������� �� ��������� �������
		TPoint Org;          // ����� ������� ��� ��������� �������� (��������)
		TRect  Prev;         //
	};
	struct State {
		STimeChunk Bounds;      // ������� ������� ����������� //
		uint   ScrollX;         // ��������������� ���������� ��������� �������
		uint   ScrollY;         // ��������������� ���������� ����� �����
		uint   ScrollLimitY;    // ������������ �������� ������ ������������� �������������� //
		uint   QBounds;         // @<=Bounds ������� ������� ����������� � ������� (���� ������ ��� �� ���������)
		uint   HdrLevelCount;   // ���������� ������� ����������
		long   HdrLevel[8];     // ������ ����������. ������ ������� �������� ����������� ������
		uint   TextZonePart;    // ���������� ���� ����� (���������) ������ ����. ������� �� �����������
			// ��-��� �������� Param::TextZonePart, ����������� ��������� ������ �������� ������ ��� ����
			// ������ � ������ ������������.
		uint   Hd_VPixQuant;    // ���������� �������� � ����� ��� ��� ������ vHourDay
		uint   Hd_HQuant;       // ����� ������� (������) �� ������������ ����� � ������ vHourDay
		uint   Hd_HPixQuant;    // ���������� �������� � ����� ������ �� ������������ ����� � ������ vHourDay
		long   SelChunkId;      // -1 - undefined
		ResizeState Rsz;
	};
	struct RowState {
		long   Id;
		uint   Order;
		LAssocArray OrderList; // ������������ ����� �������� �������� � ������ � ���
			// ������������ ������������� � ������.
			// ������ �������� [1..], ������������ ��������� [0..Order-1]
			// ���� Order == 0 || Order == 1, �� ������ ����.
	};
	struct Area {
		Area();
		Area & Clear();

		enum {
			fInited = 0x0001 // ��������� ���������������� ������� STimeChunkBrowser::GetArea()
		};
		long   Flags;          // ��������� �����
		TRect  Full;           // ������ ������� ���������
		TRect  Left;           // ����� ����� - ��������� (�������� ��� ������ Full)
		TRect  Right;          // ������ ����� - ������� (�������� ��� ������ Full)
		TRect  Separator;      // ������������ ����������� ����� � ������ ��������
		TRect  LeftHeader;     // ����� ����� ������� ������������ �������
		TRect  RightHeader;    // ������ ����� ������� ������������ �������
		TRect  PicMode;        // ������� ��������� ������ ������������ ������. ���� PicMode::IsEmpty, �� - �� ������������
		//
		uint   Quant;          // ���������� ������ � ����� �������������� ������ �������.
			// ��� P.ViewType == vPrcTime this->Quant == P.Quant,
			// ��� P.ViewType == vHourDay this->Quant �������������� ������� STimeChunkBrowser::GetArea().
		uint   PixQuant;       // ������������ ���������� �������� �� �������������� ����� �������.
			// ��� P.ViewType == vPrcTime this->PixQuant == P.PixQuant,
			// ��� P.ViewType == vHourDay this->PixQuant �������������� ������� STimeChunkBrowser::GetArea().
		uint   PixPerHour;     // � ������ vHourDay ������ ������ ���� � ��������. � ����� ������, ����� ���� ���������.
	};
	struct SRect : public TRect {
		SRect();
		SRect & Clear();

		long   RowId;
		uint   DayN;  // ��������� ��������, ���������������� ����� ���� �� '����� � ������ vHourDay
		STimeChunkAssoc C;
	};
	class SRectArray : public TSArray <SRect> {
	public:
		SRectArray();
		const SRect * FASTCALL SearchPoint(TPoint p) const;
	};

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual TBaseBrowserWindow::IdentBlock & GetIdentBlock(TBaseBrowserWindow::IdentBlock & rBlk);
	int    FASTCALL GetArea(Area & rArea) const;
	STimeChunk FASTCALL GetBoundsTime(const Area & rArea) const;
	void   CalcHdTimeBounds(const Area & rArea, DateRange & rPeriod, uint & rMinHour, uint & rMaxHour) const;
	long   DiffTime(const LDATETIME & rEnd, const LDATETIME & rStart) const;
	LDATETIME AddTime(const LDATETIME & rStart, long sec) const;
	int    FASTCALL IsQuantVisible(long q) const;
	int    FASTCALL SecToPix(long t) const;
	long   FASTCALL PixToSec(int  p) const;
	int    ChunkToRectX(uint leftEdge, const STimeChunk & rChunk, const LDATETIME & rStart, TRect & rRect) const;
	TRect  GetMargin() const;
	int    FASTCALL InvalidateChunk(long chunkId);
	void   Paint();
	void   DrawMoveSpot(TCanvas & rCanv, TPoint p);
	//
	// Descr: ��������� ������� ����, ���������� ���������� ���������
	//
	int    CalcChunkRect(const Area * pArea, SRectArray & rRectList);
	//
	// Descr: ��������� ������������� ������� � �������, ��������������� ������ � ��������������� rowId
	// ARG(side IN): 0 - full, 1 - left, 2 - right
	//
	int    GetRowRect(long rowId, int side, int * pOrder, TRect * pRect) const;
	//
	// Descr: ��������� � ��������� ���������������� ������� �����������
	//   ��� ��������� �������� ���������� ���������
	//
	int    InvalidateResizeArea();
	uint   GetBottomRowIdx(uint startIdx) const;
	uint   GetScrollLimitY() const;
	int    Scroll(int sbType, int sbEvent, int thumbPos);
	//
	// ARG(mode IN): 1 - start, 0 - end, 2 - continue
	// Returns:
	//   >0 - ���� ��������� � ������ ��������� ��������
	//   <0 - ���� �� ��������� � ������ ��������� ��������
	//
	int    Resize(int mode, TPoint p);
	int    ProcessDblClk(TPoint p);
	void   OnUpdateData();
	void   SetupScroll();
	int    GetChunkText(long chunkId, SString & rBuf);
	void   RegisterMouseTracking();
	int    FASTCALL GetStartPageDate(LDATE * pDt);
	const  RowState & FASTCALL GetRowState(long id) const;
	int    SelectChunkColor(const STimeChunkAssoc * pChunk, HBRUSH * pBrush);
	int    SaveParameters();
	int    SetupDate(LDATE dt);
	const  STimeChunkArray * GetCollapseList_() const;
	enum {
		dummyFirst = 1,
		colorHeader,            // ���� ��������� ��������� �������
		colorMain,              // �������� ���� ����
		colorInterleave,        // ���� ���� ������������� �����
		colorWhiteText,         // ����� ���� ������
		colorBlackText,         // ������ ���� ������
		penQuantSeparator,      // ������������ ����� �� ���� �����, ��������� ���� �� ����� �� ������� ������ ����������
		penMainQuantSeparator,  // ������������ ����� �� ���� �����, ��������� ���� �� ����� �� ����������, ������ ������ //
		penDaySeparator,        // @v6.8.1 ������������ ����� �� ���� �����, ��������� ���� �� ����� �� ����������, ������ ������ ��� //
		penDefChunk,            // ������ ����������� �������
		penSelectedChunk,       // ������ ���������� �������
		penChunk,               // ������ ������������� �������
		penResizedChunk,        // ������ �������, ������� �������� ���������� //
		penMainSeparator,       // ����� ������� ��� � ������� ����������
		penCurrent,             // ����� �������� �������� �������
		brushNull,              // ������ ����� (��� ������������� ��������)
		brushHeader,            // (colorHeader) ����� ��� ��������� ��������� �������
		brushMain,              // (colorMain)   ����� ��� ��������� �������� ������� ����
		brushDefChunk,          // ����� �� ��������� ��� ���������������, ������������ ��������� �������
		brushMovedChunk,        // ����� ��� ��������� ��������� ������������� �������
		brushRescaleQuant,      // ����� ��� ��������� �������������� ��������� ����� ��������
		brushInterleave,        // (colorInterleave) ����� �������������� ��������� //
		brushHoliday,           // ����� ����������� �������� ����
		brushHolidayInterleave, // ����� ����������� �������� ����
		fontHeader,             // ����� ��� ������ �������� � ������������ �����
		fontLeftText,           // ����� ��� ������ ������ � ����� ����� ����
		fontChunkText,          // ����� ��� ������ ������ ��������� ��������

		curRegular,            // ���������� ������
		curResizeHorz,         // ������ ��������� �������������� ��������
		curResizeVert,         // ������ ��������� ������������ ��������
		curResizeRoze,         // ������ ����������� //
		curCalendar,           // ������ ������ ����
		curHand                // ������ ��� ���������������� �������
	};
	STimeChunkGrid * P_Data; // @notowned
	STimeChunkGrid DataStub; // �������� ��� �������� ��������� P_Data //
	SPaintToolBox Ptb;
	Param  P;
	State  St;
	enum {
		stMouseTrackRegistered = 0x0004  // ���� ������� ������� RegisterMouseTrack()
	};
	long   Flags;
	uint   BmpId_ModeGantt;
	uint   BmpId_ModeHourDay;
	uint   BmpId_Back;
	TSCollection <RowState> RowStateList;
	StrAssocArray ChunkTextCache;
	LAssocArray ChunkColorCache; // ������ ���� {status; brush}
	LAssocArray ColorBrushList;  // ������ ������, ��������� ��� ��������� ��������
	SRectArray RL;               // ������ ���������������, ��������������� '�������� P_Data.
	TToolTip * P_Tt;
};
//
//
//
class SKeyAccelerator : public LAssocArray {
public:
	SKeyAccelerator();
	int    Set(KeyDownCommand & rK, int cmd);
};

class STextBrowser : public TBaseBrowserWindow {
public:
	typedef void * SciDocument;
	static int RegWindowClass(HINSTANCE hInst);
	static LPCTSTR WndClsName;
	//
	// Descr: ����� �������� ����� � ���������� ������
	//
	enum {
		ofReadOnly          = 0x0001,
		ofInteractiveSaveAs = 0x0002
	};
	//
	// Descr: ����������� ������ ������ � ���������� �����������
	//
	enum {
		spcmNo = 0,     // ������� �����
		spcmSartrTest   // ����� ������������ ���� ������ SARTR
	};

	STextBrowser();
	STextBrowser(const char * pFileName, int toolbarId = -1);
	~STextBrowser();

	int    Init(const char * pFileName, int toolbarId = -1);
	int    SetSpecialMode(int spcm);
	int    WMHCreate();
	HWND   GetSciWnd() const
	{
		return HwndSci;
	}
	int    Resize();
	int    CmpFileName(const char * pFileName);

	int    FileLoad(const char * pFileName, SCodepage cp, long flags);
	int    FileSave(const char * pFileName, long flags);
	int    FileClose();

	int    SetKeybAccelerator(KeyDownCommand & rK, int cmd);

	class Document {
	public:
		enum {
			stInit     = 0x0001,
			stDirty    = 0x0002,
			stReadOnly = 0x0004,
			stUtf8Mode = 0x0008,
			stNewFile  = 0x0010
		};
		Document();
		Document & Reset();
		long   SetState(long st, int set);

		SCodepageIdent OrgCp;
		SCodepageIdent Cp;
		SEOLFormat Eolf;
		long   State;
		STextBrowser::SciDocument SciDoc;
		SString FileName;
	};
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ScintillaWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual TBaseBrowserWindow::IdentBlock & GetIdentBlock(TBaseBrowserWindow::IdentBlock & rBlk);
	virtual int ProcessCommand(uint ppvCmd, const void * pHdr, void * pBrw);
	int    LoadToolbar(uint tbId);
	int    CallFunc(int msg, int param1, int param2);
	int    GetText(SString & rBuf);
	int    SetText(SString & rBuf);
	int    SaveChanges();
	int    SetEncoding(SCodepage cp);
	SCodepage SelectEncoding(SCodepage initCp) const;
	int    InsertWorkbookLink();
	int    BraceHtmlTag();

	int32  GetCurrentPos();
	int32  FASTCALL SetCurrentPos(int32 pos);
	int    FASTCALL GetSelection(IntRange & rR);
	int    FASTCALL SetSelection(const IntRange * pR);

	enum {
		srfUseDialog = 0x0001
	};
	int    SearchAndReplace(long flags);

	Document Doc;
	enum {
		sstLastKeyDownConsumed = 0x0001
	};
	long   SysState;
	int    SpcMode; // @v9.2.0
	SKeyAccelerator KeyAccel; // ���������� ������������ ����� � ���������. {KeyDownCommand Key, long Val}
	SKeyAccelerator OuterKeyAccel; // ���������� ������������ ����� � ���������, �������� ��-���: ��������� � KeyAccel
	int    (*P_SciFn)(void *, int, int, int);
	HWND   HwndSci;
	void * P_SciPtr;
	TToolbar * P_Toolbar;
	long   ToolBarWidth;
	uint   ToolbarId;
	SSearchReplaceParam LastSrParam;
	WNDPROC OrgScintillaWndProc;
	SrDatabase * P_SrDb; // @v9.2.0 ���� ������ SARTR
};
//
//
//
class TVRez {
public:
	struct WResHeaderInfo {
		uint16 Type;
		int16  IdKind;   // 0 - uint16, !0 - string
		union {
			uint16 IntID;
			char   StrID[256];
		};
		uint16 Flags;
		uint32 Size;
		uint32 Next;
	};
	enum ResPosition {
		beginOfResource, sizeField, beginOfData, nextResource
	};
	enum {
		hdrUnknown, hdr16, hdr32
	};
	SLAPI  TVRez(const char * fName, int useIndex = 0);
	SLAPI ~TVRez();
	int    SLAPI open(const char *, int useIndex = 0);
	int    SLAPI setHdrType();
	int    SLAPI buildIndex();
	int    SLAPI getChar();
	uint   SLAPI getUINT();
	char * SLAPI getString(char *, int kind = 0 /*0 - 866, 1 - w_char, 2 - 1251*/);
	SString & SLAPI getString(SString & rBuf, int kind /*0 - 866, 1 - w_char, 2 - 1251*/);
	TRect  SLAPI getRect();
	TYPEID SLAPI getType(int defaultLen);
	long   SLAPI getFormat(int defaultLen);
	int    SLAPI readHeader(ulong ofs, WResHeaderInfo * hdr, ResPosition);
	int    SLAPI findResource(uint rscID, uint rscType, long * pOffs = 0, long * pSz = 0);
	int    SLAPI getSizeField(long *);
	long   SLAPI getStreamPos();
	int    SLAPI enumResources(uint rscType, uint * rscID, ulong * dwPos);
	FILE * SLAPI getStream() const { return Stream; }
	int    SLAPI CheckDialogs(const char * pLogFileName); // @debug

	int    error;
private:
	int    SLAPI _readHeader16(ulong ofs, WResHeaderInfo * hdr, ResPosition);
	int    SLAPI _readHeader32(ulong ofs, WResHeaderInfo * hdr, ResPosition);
	SString FileName;
	FILE   * Stream;
	SArray * Index;
	int      HeaderType;
};

extern int (SLAPI * getUserControl)(TVRez *, TDialog*);

// @v9.5.10 HMENU  SLAPI LoadMenu(TVRez *, uint menuID);
int    SLAPI LoadToolbar(TVRez *, uint tbType, uint tbID, ToolbarList *);

#endif // } _TURBOVISION
//
#endif // } __TV_H
