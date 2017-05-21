// SARTR.H
// Copyright (c) A.Sobolev 2011, 2012, 2016, 2017
//

/*
	<...> - �����������

	//
	//
	//
	����� ����:
	<NOUN>
		��� (Gender): ������� (Masculine) | ������� (Feminine) | ������� (Neuter) | ����� [������; ���������]
		����������� �����: ������������ | ����������� | ����������� //
		����� (): ������������ (Singular) | ������������� (Plural)
		����� (Case):
			������������ (Nominative) |
			����������� (Genitive) |
			��������� (Dative) |
			����������� (Accusative) |
			������������ (Ablative)  |
			���������� (Prepositional) |
			(������ ������ - ����� �� ����� 32)

	<ADJV>
	<VERB>
	//
	// ������ �������
	//
	(pluralia tantum) - ���������������, ������������� ������ � ��. ����� [�����; �����; ����]
*/
//
// �������������� �������� ����
// ������� @bin ����� � ���������, ������� ����� ����� �������� 1 ��� �������������
//
#define SRWG_LANGUAGE                 1 // ����
/*
	#define SRLANG_META                   1
	#define SRLANG_LAT                    2
	#define SRLANG_ENG                    3
	#define SRLANG_RUS                    4
	#define SRLANG_GER                    5
*/
#define SRWG_CLASS                    2 // SRWC_XXX       ����� �������
	#define SRWC_ALPHA                    1 // ALPHA        ����� ��������
	#define SRWC_DIGIT                    2 // DIGIT        �������� �����
	#define SRWC_NOUN                     3 // NOUN         ��������������� (Noun)
	#define SRWC_NUMERAL                  4 // NUM          ������������ (Numeral) [One]
	#define SRWC_NUMERALORD               5 // ORD          ���������� ������������ (Order numeral) [First]
	#define SRWC_ADJECTIVE                6 // ADJ          �������������� (Adjective)
	#define SRWC_VERB                     7 // VERB         ������ (Verb)
	// @v9.2.0 #define SRWC_VERBMODAL                8 // VERBMODAL    ��������� ������ (Modal verb) [Should]
	#define SRWC_PRONOUN                  9 // PRONOUN      ����������� (Personal pronoun)
	#define SRWC_PRONOUNPOSS             10 // PRONOUNPOSS  �����������-�������������� (Possessive pronoun) [His, Her]
	#define SRWC_PRAEDICPRO              11 // PRAEDICPRO   �����������-���������� @ex "������"
	#define SRWC_PRAEDIC                 12 // PRAEDIC      ���������� @ex "���������"
	#define SRWC_ADVERB                  13 // ADVERB       ������� (Adverb) [freely; ��������]
	#define SRWC_PREPOSITION             14 // PREP         ������� (Preposition) [beside]
	#define SRWC_POSTPOSITION            15 // POSTP        �������� @ex "���� ����"
	#define SRWC_CONJUNCTION             16 // CONJ         ���� (Conjunction) [and]
	#define SRWC_INTERJECTION            17 // INTERJ       ���������� (Interjection) [oh]
	#define SRWC_PARENTH                 18 // PARENTH      ������� ����� @ex "�������"
	#define SRWC_ARTICLE                 19 // ARTICLE      ������� (Article) [the]
	#define SRWC_PARTICLE                20 // PARTICLE     ������� (Particle) [to]
	#define SRWC_PARTICIPLE              21 // PARTCP       ��������� (Participle) [reading]
	#define SRWC_GERUND                  22 // GERUND       �������� (Gerund) [reading books]
	#define SRWC_GPARTICIPLE             23 // GPARTC       ������������ @ex "�����������, �� ������ �������"
	#define SRWC_PHRAS                   24 // PHRAS        ���������������� ����� (��� �� ������ ���� ��������� �����
		// ����� �������������� - � ������� ������� ����������� ������ ���� ����� ������ ���� - "����").
	#define SRWC_PREFIX                  25 // PREFIX       ���������
	#define SRWC_AFFIX                   26 // AFFIX        ���������
	#define SRWC_SKELETON                27 // SKEL         ������������ ������ �����
	//#define SRWC_PRONOUNREFL             28 // PRONOUNREFL  ���������� ����������� (reflexive pronoun)
	#define SRWC_PRONOUNNOUN             29 // PRONOUNNOUN  �����������-��������������� [all, anybody]
	#define SRWC_POSSESSIVEGROUP         30 // POSSGRP possessive (english)
		// �������� �������� "����� ����", ��������� ���������� ���� �������������� ��������� ��������� �� � ������ �����,
		// �� � ����� ������� ������ � �������������� ������ � ���������� ��� �����.
		// [the King of England's daughter] [the girl I was dancing with's name]
#define SRWG_ABBR                     3 // SRABBR_XXX     ���� �����������
	#define SRABBR_ABBR                   1 // ABBR ����� ����������� ������������ ��� �����������
	#define SRABBR_NARROW                 2 // NARR ������ ����������� ������� ����� (��� ������, ������ � �.�.)
#define SRWG_PROPERNAME               4 // SRPROPN_XXX    ��� ����� ������������
	#define SRPROPN_PERSONNAME            1 // PERSN ��� //
	#define SRPROPN_FAMILYNAME            2 // FAMN  ������� //
	#define SRPROPN_PATRONYMIC            3 // PATRN ��������
	#define SRPROPN_ZOONAME               4 // ZOON  ������ ���������
	#define SRPROPN_ORG                   5 // ORG   ������������ �����������
	#define SRPROPN_GEO                   6 // GEO   ������������ ��������������� ������� (���������� �����, ����, ���� � �.�.)
#define SRWG_ANIMATE                  5 // SRANIM_XXX     ��������������
	#define SRANIM_ANIMATE                1 // ANIM   ������������
	#define SRANIM_INANIMATE              2 // INANIM �������������
#define SRWG_USAGE                    6 // SRWU_XXX       ����� ������������ �����
	#define SRWU_LITERARY                 1 // LIT ������������ //
	#define SRWU_PRO                      2 // PRO ���������������
	#define SRWU_ARCHAIC                  3 // ARC ����������� (����������) //
	#define SRWU_SPOKEN                   4 // SPK �����������  //
	#define SRWU_VULGAR                   5 // VUL ����������   //
#define SRWG_GENDER                   7 // SRGENDER_XXX   ���
	#define SRGENDER_MASCULINE            1 // MASC ������� (Masculine)
	#define SRGENDER_FEMININE             2 // FEM  ������� (Feminine)
	#define SRGENDER_NEUTER               3 // NEU  ������� (Neuter)
	#define SRGENDER_COMMON               4 // GCOM ����� [������; ���������]
#define SRWG_TANTUM                   8 // �������������, ����������� �� ����������� ������������� ����� ������ � ����� �����
	#define SRTANTUM_SINGULAR             1 // SINGT ��������������� ������ �� ������������ ����� @ex "����" "���������"
	#define SRTANTUM_PLURAL               2 // PLURT ��������������� ������ �� ������������� ����� @ex "����" "���"
#define SRWG_COUNT                    9 // SRCNT_XXX      �����
	#define SRCNT_SINGULAR                1 // SING ������������ �����
	#define SRCNT_PLURAL                  2 // PLUR ������������� �����
#define SRWG_CASE                    10 // SRCASE_XXX     �����
	#define SRCASE_NOMINATIVE             1 // NOM    ������������
	#define SRCASE_GENITIVE               2 // GEN    �����������
	#define SRCASE_GENITIVE2              3 // GEN2   ������ ����������� @ex "����� ���"
	#define SRCASE_DATIVE                 4 // DAT    ���������
	#define SRCASE_DATIVE2                5 // DAT2   �������������� ��������� @ex "[��] �����, ���������, �������"
	#define SRCASE_ACCUSATIVE             6 // ACC    �����������
	#define SRCASE_ACCUSATIVE2            7 // ACC2   ������ ����������� @ex "���������� � ������" "�� ��� ��������"
	#define SRCASE_INSTRUMENT             8 // INS    ������������ @ex "�������" "�����" "������" "������" "�������"
	#define SRCASE_PREPOSITIONAL          9 // PREP   ����������
	#define SRCASE_PREPOSITIONAL2        10 // PREP2  ������ ���������� @ex "� ����" "�� ���"
	#define SRCASE_VOCATIVE              11 // VOC    ���������� @ex "�������" "����" "�����"
	#define SRCASE_ADNUM                 12 // ADNUM  ������� ����� @ex "��� ����" "��� ����"
	#define SRCASE_OBJECTIVE             13 // OBJCTV ��������� ����� (� ����������)
#define SRWG_TENSE                   11 // SRTENSE_XXX    ����� //
	#define SRTENSE_PRESENT               1 // PRES ���������
	#define SRTENSE_PAST                  2 // PAST ���������
	#define SRTENSE_FUTURE                3 // FUTU �������
	#define SRTENSE_PASTPARTICIPLE        4 // PASTPART ��������� ����� �������������� ����� (English: Past Participle)
#define SRWG_PERSON                  12 // SRPERSON_XXX   ����
	#define SRPERSON_FIRST                1 // P1   ������ ����
	#define SRPERSON_SECOND               2 // P2   ������ ����
	#define SRPERSON_THIRD                3 // P3   ������ ����
#define SRWG_VALENCY                 13 // SRVALENCY_XXX  ����������� �������
	#define SRVALENCY_AVALENT             1 // AVALENT   ���������� (���������) ������. @ex "�������"
	#define SRVALENCY_INTRANSITIVE        2 // INTRANS   ������������ ������. @ex "�����"
	#define SRVALENCY_TRANSITIVE          3 // TRANS     ���������� ������.   @ex "��������"
	#define SRVALENCY_DITRANSITIVE        4 // DITRANS   ���������� ������ � ������������ 3. @ex "He gives her a flower."
#define SRWG_ASPECT                  14 // SRASPECT_XXX   ������ �������
	#define SRASPECT_INFINITIVE           1 // INF       �������������� ����� �������
	#define SRASPECT_PERFECTIVE           2 // PERFV     ����������� ������
	#define SRASPECT_IMPERFECTIVE         3 // IMPERFV   ������������� ������
	#define SRASPECT_HABITUAL             4 // HABIT     ������������� ��������  @ex "I used to go there every day"
	#define SRASPECT_CONTINUOUS           5 // CONTS     �������������� ��������
	#define SRASPECT_STATIVE              6 // CSTATV    ������������ SRASPECT_CONTINUOUS @ex "I know French"
	#define SRASPECT_PROGRESSIVE          7 // CPROGV    ������������ SRASPECT_CONTINUOUS @ex "I am running"
	#define SRASPECT_PERFECT              8 // PERF      ��������������� �� �������� ������ @ex "I have studied well"
#define SRWG_MOOD                    15 // SRMOOD_XXX     ���������� (�����������) �������
	#define SRMOOD_INDICATIVE             1 // INDCTV    Indicative   @ex "Paul is eating an apple" "John eats apples"
	#define SRMOOD_SUBJUNCTIVE            2 // SUBJUNCTV Subjunctive  @ex "John would eat if he 'were' hungry"
	#define SRMOOD_CONJUNCTIVE            3 // CONJUNCTV Conjunctive
	#define SRMOOD_OPTATIVE               4 // OPTV      Optative
	#define SRMOOD_JUSSIVE                5 // JUSSIV    Jussive
	#define SRMOOD_POTENTIAL              6 // POTENT    Potential
	#define SRMOOD_PROHIBITIVE            7 // PROHIBV   Prohibitive
	#define SRMOOD_IMPERATIVE             8 // IMPERV    ������������� ����������
	#define SRMOOD_INTERROGATIVE          9 // INTERRV   �����
#define SRWG_VOICE                   16 // SRVOICE_XXX    ����� �������
	#define SRVOICE_ACTIVE                1 // ACT       �������������� �����  @ex "��������" "�����������"
	#define SRVOICE_PASSIVE               2 // PASS      ������������� �����   @ex "�����������" "�����������"
	#define SRVOICE_MEDIAL                3 // MED       ���������� �����      @ex "����������"
	// .. ���������� ��������� ���� ������� � ������ ������
#define SRWG_ADJCAT                  17 // ��������� ���������������
	#define SRADJCAT_QUALIT               1 // ADJQUAL   ������������ ��������������
	#define SRADJCAT_RELATIVE             2 // ADJREL    ������������� ��������������
	#define SRADJCAT_POSSESSIVE           3 // ADJPOSS   �������������� ��������������
	#define SRADJCAT_NATION               4 // ADJNATION ��������������, ���������� �������������� � ��������������
		// @? � �� ������, ��� ��� ��������� �� ����� ������������ � �������.
#define SRWG_ADJCMP                  18 // SRADJCMP_XXX   ������������� ����� ���������������
	#define SRADJCMP_COMPARATIVE          1 // COMP    ������������� �������������� @ex "�������" "�������"
	#define SRADJCMP_SUPERLATIVE          2 // SUPR    ������������ ������� ���������������
	#define SRADJCMP_COMPARATIVE2         3 // COMP2   ������ ������������� ������� ��������������� @ex "���������" "���������"
#define SRWG_SHORT                   19 // SRSHORT_XXX    ������� ����� (������ ��������������, �� �������� � ������ ������ ����)
	#define SRSHORT_BREV                  1 // BREV  ������� ����� ���������������
	#define SRSHORT_PLEN                  2 // PLEN  ������ ����� ���������������
#define SRWG_INVARIABLE              20 //                ������������ ����� (���� 0, ���� 1)
#define SRWG_ADVERBCAT               21 // SRADVCAT_XXX   ��������� ������� //
	#define SRADVCAT_INTERROGATIVE        1 // ADVINTR �������������� �������
	#define SRADVCAT_RELATIVE             2 // ADVREL  ������������� �������
	#define SRADVCAT_POINTING             3 // ADVPNT  ������������ �������
#define SRWG_LOCAT                   22 // @bin LOC ���������� �������������� @ex "���������������"
#define SRWG_ERROR                   23 // @bin ERR ��������� ��������� �����, ������ �������� � �.�.
#define SRWG_TOBE                    24 // @bin ����������� ������ - ������ to be � ����������. ��� ���������
	// ���� ��������� ������������� ���������� (��� ��������) ������.
#define SRWG_QUEST                   25 // @bin QUEST (�� ���� � ����� ��������� �������) �������������� ����� ����� (�������)
#define SRWG_MODAL                   26 // @bin MODAL (�� ���� � ����� ��������� �������) ��������� ������.
#define SRWG_POSSESSIVE              27 // @bin POSSTAG (��������� � possessive � ����������: ���� ����� ��� ��� ������������ �������������
	// ��������� � ����� ���������). �� ������ ������ ��������� ��� ����� ���� �������� ����������� � ���������������� � english
#define SRWG_PREDICATIVE             28 // @bin PREDICAT ������� �������������� ����������� (english) [yours]
#define SRWG_PRONOUN                 29 // SRPRON_XXX ���� �����������. � ����������� ����� �� ��� ������ ���� (generic)
	#define SRPRON_REFL               1 // PRONREFL ���������� ����������� (reflexive pronoun) [self]
	#define SRPRON_DEMONSTR           2 // PRONDEM  ������������ ����������� (pronomina demonstrativa) [one]
	#define SRPRON_PERSONAL           3 // PRONPERS ������ �����������
#define SRWG_COUNTAB                 30 // SRCTB_XXX ������������� ���������������
	#define SRCTB_UNCOUNTABLE         1 // NUNCNT ������������� ��������������� (english)
	#define SRCTB_MASS                2 // NMASS  mass-��������������� (english)
//
// --------------------------------------------------------
//
typedef uint32 LEXID;
typedef int64  CONCEPTID;
typedef int64  NGID;
//
// ���� �������������� ������ �������������� ���� �����
//
#define SRGRULETYP_AFFIXLIST          1
#define SRGRULETYP_ACCENTLIST         2
//
// Descr: ���� �������������� �����������
//
#define SRGRAMTYP_WORDFORM            1 // ���������� ���������� => SrWordForm
#define SRGRAMTYP_FLEXIAMODEL         2 // FlexiaModel           => SrFlexiaModel
//
//
//
class SrDatabase;
//
//
//
class SrSList : public SBaseBuffer {
public:
	SrSList(int type);
	virtual ~SrSList();
	int    GetType() const;
	size_t GetLength() const;
	int FASTCALL Copy(const SrSList & rS);
	virtual void Clear();
	virtual int IsEqual(const SrSList & rS) const;
	virtual int Validate() const;
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
protected:
	int32  Type;
	uint32 Len;
};
//
// Descr: ���������� ������������� ��������� ����������.
//
class SrWordForm : public SrSList { // @transient
public:
	SrWordForm();
	SrWordForm & FASTCALL operator = (const SrWordForm & rS);
	SrWordForm & FASTCALL Copy(const SrWordForm & rS);
	//
	// Descr: ��������� ����� ���������� �������� �������� �������� ����������� � ����������� ������� �����.
	// Note: ������� �� ������������: Merge(base_form, var_form) != Merge(var_form, base_form)
	//   ����� ����������� �������������� ���������� ������������� ������� Normalize()
	// ARG(rBase  IN): ������ �� ������� ��������
	// ARG(rVar   IN): ������ �� ����������, ���� ������� ��������� � �������.
	// ARG(mode   IN): ����� ��������� �����:
	//   0 - ���� �� ����������, �� ��������� �����, ���� ��� ���������� - ����������.
	//   1 - ���� �� ����������, �� ������ �� ������, ���� ��� ���������� - ����������.
	//   2 - ���� �� ����������, �� ��������� �����, ���� ��� ����������, �� ������ �� ������.
	//
	SrWordForm & Merge(const SrWordForm & rBase, const SrWordForm & rVar, int mode = 0);
	int    Normalize();
	int    FASTCALL IsEqual(const SrWordForm & rS) const;
	//
	// Descr: ������������ ���������� this �� ����������� rS.
	//   ���������� �������� ��������������� ��� Real-�����. ��� ������ �����, ���
	//   ������ ��������� �������������� ���������.
	// Note: ������������ ������� �� ��������� this, ������ �� ������� �������������� �
	//   ����������� ����� � rS. ����� �������, ���� ���������� ����������� ������������ ���������� A,
	//   ���������� �������� ��������, � ������ ��������� B, �� ������� �������� ������� � ���� A.MatchScore(B).
	//
	double FASTCALL MatchScore(const SrWordForm & rS) const;
	//
	// Descr: ������������� ��� tag � �������� val.
	// ARG(tag  IN): ������������� ���� ����������.
	// ARG(val  IN): �������� ���� tag ����������.
	// ARG(mode IN): ����� ��������� ����:
	//   0 - ���� �� ����������, �� ��������� �����, ���� ��� ���������� - ����������.
	//   1 - ���� �� ����������, �� ������ �� ������, ���� ��� ���������� - ����������.
	//   2 - ���� �� ����������, �� ��������� �����, ���� ��� ����������, �� ������ �� ������.
	// Returns:
	//   1 - �������� ���� ���� ��������
	//   2 - ��� �������� ����� ��� � ��������� ���������
	//  -1 - ������ �� ����������
	//   0 - ������
	//
	int    SetTag(int tag, int val, int mode = 0);
	int    FASTCALL RemoveTag(int tag);
	int    FASTCALL GetTag(int tag) const;
	size_t CalcLength() const;
	int    FASTCALL ToStr(SString & rBuf) const;
	int    FASTCALL FromStr(const char * pStr);
private:
	void   CatTok(SString & rBuf, const char * pTok) const;
	size_t Set(size_t pos, int val);
	int    FASTCALL Tag(size_t pos) const;
	int    FASTCALL Get(size_t pos) const;
	size_t FASTCALL Step(size_t pos) const;
};
//
// Descr: ������������� ��������� ��������� � ����������� �� ��������� �, ��������, ���������.
//
class SrFlexiaModel : public SrSList { // @transient
public:
	struct Item {
		LEXID  AffixID;
		LEXID  PrefixID;
		int32  WordFormID;
	};
	SrFlexiaModel();
	int    Normalize();
	int    FASTCALL IsEqual(const SrFlexiaModel & rS) const;
	int    FASTCALL Add(const SrFlexiaModel::Item & rItem);
	int    Search(LEXID afxID, LEXID pfxID, LongArray & rWordFormList) const;
	int    GetNext(size_t * pPos, SrFlexiaModel::Item & rItem) const;
private:
	enum {
		fmiZeroAffix  = 0x01,
		fmiZeroPrefix = 0x02,
		fmiWf16       = 0x04
	};
};
//
//
//
struct SrWordAssoc {
	SLAPI  SrWordAssoc();
	SrWordAssoc & SLAPI Normalize();
	SString & FASTCALL ToStr(SString & rBuf) const;

	enum {
		fHasFlexiaModel = 0x0001,
		fHasAccentModel = 0x0002,
		fHasPrefix      = 0x0004,
		fHasAffixModel  = 0x0008
	};

	int32  ID;             // ���������� ����� ����������
	LEXID  WordID;         // ������������� �����
	long   Flags;          // @flags
	int32  BaseDescrID;
	int32  FlexiaModelID;
	int32  AccentModelID;
	int32  PrefixID;
	int32  AffixModelID;
};
//
// {id;word}
//
class SrWordTbl : public BDbTable {
public:
	enum {
		spcEmpty = 1,
		spcPrefix,
		spcAffix,
		spcConcept,
		spcCPropSymb
	};
	SrWordTbl(BDbDatabase * pDb);
	~SrWordTbl();
	int    Add(const char * pWordUtf8, LEXID * pID);
	int    AddSpecial(int spcTag, const char * pWordUtf8, LEXID * pID);
	int    Search(const char * pWordUtf8, LEXID * pID);
	int    SearchSpecial(int spcTag, const char * pWordUtf8, LEXID * pID);
	int    Search(LEXID id, SString & rBuf);
private:
	int    MakeSpecial(int spcTag, const char * pWordUtf8, SString & rBuf);

	long   SeqID;
};
//
// Descr:
//   �������, �������� ������� �������������� �����������. ���-��:
//     -- ����������� ���������
//     -- FlexiaModel (��������� ���������, ��������������� �� ������������)
//     -- �������� ��������� (��� �������� ����������)
//     -- � �.�.
// ��������� ������:
// {
//     int32 ID;      // ������������� ������
//     int16 Type;    // ��� ������, ���������� � ������
//     uint8 Tail[];  // ���������� �������������� �����������. ������ �������� ������� ������� �� Type.
// }
//
// �������������� ������ ������� �������� �� ���� {Type; Tail}.
//
class SrGrammarTbl : public BDbTable {
public:
	SrGrammarTbl(BDbDatabase * pDb);
	~SrGrammarTbl();
	int    Add(SrWordForm * pWf, long * pID);
	int    Add(SrFlexiaModel * pFm, long * pID);
	int    Search(long id, SrWordForm * pWf);
	int    Search(long id, SrFlexiaModel * pFm);
	int    Search(SrWordForm * pWf, long * pID);
	int    Search(SrFlexiaModel * pFm, long * pID);
private:
	int    Helper_Add(SrSList * pL, long * pID);
	int    Helper_Search(SrSList * pL, long * pID);
	int    Helper_Search(long id, SrSList * pL);

	long   SeqID;
};
//
//
//
class SrWordAssocTbl : public BDbTable {
public:
	SrWordAssocTbl(BDbDatabase * pDb);
	~SrWordAssocTbl();
	int    Add(SrWordAssoc * pWa, int32 * pID);
	int    Update(int32 id, SrWordAssoc * pWa);
	int    Search(int32 id, SrWordAssoc * pWa);
	int    Search(LEXID wordID, TSArray <SrWordAssoc> & rList);
	int    SerializeRecBuf(int dir, SrWordAssoc * pWa, SBuffer & rBuf);
private:
	long   SeqID;
};
//
// Descr:
//
class SrNGram {
public:
	SrNGram();
	NGID   ID;
	int32  Ver;
	LongArray WordIdList;
};
//
// Descr: ������� N-�����. �������� ������������������ ������ ������ �� �����, ����������� ���������.
//   ��������: "����� ������" �������� ��� ���� ������ �� ����� "�����" � "������" � ������� SrWordTbl.
//
class SrNGramTbl : public BDbTable {
public:
	SrNGramTbl(BDbDatabase * pDb);
	~SrNGramTbl();
	int    Add(SrNGram & rNGram);
	int    Search(NGID id, SrNGram * pNGram);
	int    Search(const SrNGram & rNGram, NGID * pID);
	int    SearchByPrefix(const SrNGram & rNGram, TSArray <NGID> & rList);

	int    SerializeRecBuf(int dir, SrNGram * pRec, SBuffer & rBuf);
private:
	long   SeqID;
};

#define SRPROPT_UNKN      0
#define SRPROPT_INT       1
#define SRPROPT_STRING    2
#define SRPROPT_REAL      3
#define SRPROPT_HDATE     4
#define SRPROPT_HPERIOD   5

class SrCPropDecl {
public:
	SrCPropDecl();
	~SrCPropDecl();
	int    FASTCALL IsEqual(const SrCPropDecl & rS) const;
//private:
	CONCEPTID PropID; // �� ���������-��������
	LEXID  SymbID;    // ������������ ������������� ������� ��������
	SBaseBuffer Tail; // ������������ ����� ������������ ��������, ������� ����� ��������� ����������� �������� ����������
};

class SrCPropDeclList {
public:
	SrCPropDeclList();
	~SrCPropDeclList();
	SrCPropDeclList(const SrCPropDeclList & rS);
	SrCPropDeclList & FASTCALL operator = (const SrCPropDeclList & rS);
	SrCPropDeclList & FASTCALL Copy(const SrCPropDeclList & rS);
	SrCPropDeclList & Clear();
	uint   GetCount() const
	{
		return D.getCount();
	}
	int    FASTCALL Add(const SrCPropDecl & rP);
	int    Replace(uint pos, const SrCPropDecl & rP);
	int    Get(uint idx, SrCPropDecl & rP) const;
	int    GetBySymbID(LEXID id, SrCPropDecl & rP) const;
	int    Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	int    FASTCALL IsEqual(const SrCPropDeclList & rS) const;
	int    FASTCALL Merge(const SrCPropDeclList & rS);
private:
	struct Item {
		CONCEPTID PropID;
		LEXID  SymbID;
		uint32 TailS;  // ������ ��������� �����
		uint32 TailP;  // ������� ��������� ����� ����������� �������� � ������ Pool
	};
	TSArray <Item> D;
	uint32 PoolP; // �������, � ������� ������� ��������� ��������� ������� � Pool
	SBaseBuffer Pool;
};

class SrConcept {
public:
	SrConcept();
	int    FASTCALL IsEqual(const SrConcept & rS) const;
	SrConcept & Clear();

	CONCEPTID ID;
	LEXID  SymbID;
	int32  Ver;    // ������ ������� ������ (0..)
	SrCPropDeclList Pdl;
};
//
// Descr: ������� ���������. ��������� ������������ ������ ���������������.
//   � ���������� � �������������� ������ ��������� ����� ��������� ������ ��
//   ���������� (�� ������� SrWordTbl) � ������ ����� �������.
//   ��������� ����� ���� ������� ��� ������ �� ������. � ����� ������ ����� ��
//   ������� ��������� ������ ���� ����������� �� �������� (�� ����-) �����.
// {id;symbol-ref;prop-decl-list}
//
class SrConceptTbl : public BDbTable {
public:
	SrConceptTbl(BDbDatabase * pDb);
	~SrConceptTbl();
	int    Add(SrConcept & rRec);
	int    Update(SrConcept & rRec);
	int    Remove(CONCEPTID id);
	int    SetPropDeclList(CONCEPTID id, SrCPropDeclList * pPdl);
	int    SearchByID(CONCEPTID id, SrConcept * pRec);
	int    SearchBySymb(LEXID symbId, SrConcept * pRec);

	int    SerializeRecBuf(int dir, SrConcept * pRec, SBuffer & rBuf);
private:
	long   SeqID;
};
//
// Descr: �������� �������� ���������
//
class SrCProp {
public:
	SrCProp();
	SrCProp(CONCEPTID cID, CONCEPTID propID);
	int    FASTCALL IsEqual(const SrCProp & rS) const;
	SrCProp & Clear();
	SrCProp & FASTCALL operator = (int);
	SrCProp & FASTCALL operator = (int64);
	SrCProp & FASTCALL operator = (double);
	SrCProp & FASTCALL operator = (const char *);

	int    FASTCALL Get(int64 & rIntVal) const;
	int    FASTCALL Get(double & rRealVal) const;

	CONCEPTID CID;
	CONCEPTID PropID;
	SBuffer Value;
};
//
// Descr: ������ �������� ������� ���������
//
class SrCPropList {
public:
	SrCPropList();
	int    Set(CONCEPTID cID, CONCEPTID propID, const void * pData, size_t dataLen);
	SrCPropList & Clear();
	int    Search(CONCEPTID cID, CONCEPTID propID, uint * pPos) const;
	size_t FASTCALL GetDataLen(uint pos) const;
	const  void * GetDataPtr(uint pos, size_t * pDataLen) const;
	size_t GetData(uint pos, void * pData, size_t bufLen) const;
	uint   GetCount() const;
	int    GetProp(uint pos, SrCProp & rProp) const;
private:
	struct Item {
		CONCEPTID CID;
		CONCEPTID PropID;
		uint32 P;         // ������� ������ � ������ SrCPropList::D
		uint32 S;         // ����� ������ � ������ SrCPropList::D
	};
	int    SetData(uint pos, const void * pData, size_t dataLen);
	TSArray <Item> L;
	SBuffer D;
};
//
// Descr: ������� �������� ������� ���������
// {concept-id; prop-id; prop-value}
//
class SrConceptPropTbl : public BDbTable {
public:
	SrConceptPropTbl(SrDatabase & rSr);
	int    Set(SrCProp & rProp);
	int    Search(SrCProp & rRec);
	int    GetPropIdList(CONCEPTID cID, Int64Array & rPropIdList);
	int    GetPropList(CONCEPTID cID, SrCPropList & rPropList);
	int    Remove(CONCEPTID cID, CONCEPTID propID);

	int    SerializeRecBuf(int dir, SrCProp * pRec, SBuffer & rBuf);

	static int FASTCALL EncodePrimeKey(BDbTable::Buffer & rKeyBuf, const SrCProp & rRec);
	static int FASTCALL DecodePrimeKey(const BDbTable::Buffer & rKeyBuf, SrCProp & rRec);
private:
	SrDatabase & R_Sr; // @notowned
};
//
// Descr: ������� ������������ ����� N-�������� � �����������. ����������, ��� -
//   �������, �������������� ������ ����� ���������������� (�������) �� ������������ ������
//   � ���������������� �����������.
// {concept-id, ngram-id}
//
class SrConceptNgTbl : public BDbTable {
public:
	SrConceptNgTbl(BDbDatabase * pDb);
	int    Set(CONCEPTID cID, NGID ngID);
	int    GetNgList(CONCEPTID cID, Int64Array & rNgList);
	int    GetConceptList(NGID ngID, Int64Array & rConceptList);
};
//
//
//
class SrGeoNodeTbl : public BDbTable {
public:
	SLAPI  SrGeoNodeTbl(BDbDatabase * pDb);
	SLAPI ~SrGeoNodeTbl();
	int    SLAPI Add(PPOsm::NodeCluster & rNc, uint64 outerID);
	int    SLAPI Update(PPOsm::NodeCluster & rNc, uint64 outerID);
	int    SLAPI Search(uint64 id, PPOsm::Node * pNode, PPOsm::NodeRefs * pNrList, uint64 * pLogicalID);
	int    SLAPI Search(uint64 id, PPOsm::NodeCluster * pCluster, uint64 * pLogicalID);
private:
	int    SLAPI Helper_Set(PPOsm::NodeCluster & rNc, uint64 outerID, int update);
	int    SLAPI Helper_Search(uint64 id, PPOsm::NodeCluster * pCluster, PPOsm::Node * pNode, PPOsm::NodeRefs * pNrList, uint64 * pLogicalID);
	//
	// ������ ��� ���������� �������������. ���������� ��� ����� ������
	// ���� �������� ������ ������������� ������.
	//
	BDbTable::Buffer KeyBuf;
	BDbTable::Buffer DataBuf;
};

class SrGeoWayTbl : public BDbTable {
public:
	SLAPI  SrGeoWayTbl(BDbDatabase * pDb);
	SLAPI ~SrGeoWayTbl();
	int    SLAPI Add(PPOsm::Way & rW, PPOsm::WayBuffer * pBuffer);
	int    SLAPI Search(uint64 id, PPOsm::Way * pW);
private:
	//
	// ������ ��� ���������� �������������. ���������� ��� ����� ������
	// ���� �������� ������ ������������� ������.
	//
	BDbTable::Buffer KeyBuf;
	BDbTable::Buffer DataBuf;
};
//
//
//
class SrImportParam {
public:
	enum {
		inpFlexiaModel = 1,
		inpMySpell
	};
	enum {
		fldAncodeFileName = 1,
		fldFlexiaModelFileName
	};
	enum {
		fTest = 0x0001
	};
	SrImportParam();
	int    SetField(int fld, const char * pVal);
	int    GetField(int fld, SString & rVal) const;

	int    InputKind;
	int    LangID;    // ������������� �����
	int    CpID;      // ������������� ������� �������� ������� ������
	long   Flags;     // @flags
	StrAssocArray StrItems; // ������ ��������� ����������
};

struct SrWordInfo {
	SrWordInfo();
	SrWordInfo & Clear();

	LEXID  BaseID;     // ������������� ������������ ���� �����
	LEXID  PrefixID;   // ������������� ��������� �����
	LEXID  AffixID;    // ������������� ��������� �����
	//
	int32  BaseFormID; // ������� ���������� ����������, ��������������� � BaseID
	int32  FormID;     // ������������� ���������� ���������� �����
	// ��� ����, ����� �������� �������� �������� ���������� ���������� ������� ���������� BaseFormID � FormID
	int32  WaID;       // ������������� ������������� ���������� �����
	double Score;      // �������� ������������� ������ �������������� ����� � ��������� (��� �������������� �����)
};

class SrDatabase {
public:
	enum {
		rcInstance = 1,
		rcSubclass,
		rcType
	};
	SrDatabase();
	~SrDatabase();
	int    Open(const char * pDbPath);
	int    Close();
	operator BDbDatabase *()
	{
		return P_Db;
	}

	CONCEPTID GetReservedConcept(int rc);

	int    GetWordInfo(const char * pWordUtf8, long flags, TSArray <SrWordInfo> & rInfo);
	int    WordInfoToStr(const SrWordInfo & rWi, SString & rBuf);
	int    Transform(const char * pWordUtf8, const SrWordForm * pDestForm, TSArray <SrWordInfo> & rResult);

	int    SearchWord(int special, const char * pWordUtf8, LEXID * pID);
	int    SearchNGram(const LongArray & rNg, NGID * pID);

	enum {
		ngclAnonymOnly = 0x0001
	};
	//
	// Descr: ���������� ������ ��������������� ���������, ��������������� � N-������� ngID.
	// ARG(ngID  IN): ������������� N-������, ��� ������� ������ ���������
	// ARG(flags IN): BDbDatabase::ngclXXX ����� ������
	// ARG(rConceptList OUT): ������ ��������� ��������������� ���������
	// Returns:
	//   >0 - ������� �� ������� ���� ���� ���������, ��������������� � ngID
	//   <0 - �� ������� �� ����� ���������, ��������������� ��������
	//   0  - ������
	//
	int    GetNgConceptList(NGID ngID, long flags, Int64Array & rConceptList);
	int    GetConceptHier(CONCEPTID cID, Int64Array & rConceptHier);
	int    GetConceptSymb(CONCEPTID cID, SString & rSymbUht8);
	int    GetPropDeclList(CONCEPTID cID, SrCPropDeclList & rPdl);
	int    GetConceptPropList(CONCEPTID cID, SrCPropList & rPl);

	int    GetPropType(CONCEPTID propID);

	int    ResolveConcept(const char * pSymbUtf8, CONCEPTID * pID);
	int    CreateAnonymConcept(CONCEPTID * pID);
	int    ResolveWord(const char * pWordUtf8, LEXID * pID);
	int    ResolveNGram(const LongArray & rList, NGID * pID);
	int    ResolveCPropSymb(const char * pSymbUtf8, LEXID * pID);

	int    MakeConceptPropC(const SrCPropDeclList & rPdl, const char * pPropSymb, SrCProp & rProp, const char * pConceptSymb);
	int    MakeConceptPropNg(const SrCPropDeclList & rPdl, const char * pPropSymb, SrCProp & rProp, const LongArray & rNg);
	int    MakeConceptPropN(const SrCPropDeclList & rPdl, const char * pPropSymb, SrCProp & rProp, double value);
	int    FormatProp(const SrCProp & rCp, long flags, SString & rBuf);

	int    ImportFlexiaModel(const SrImportParam & rParam);
	int    StoreGeoNodeList(const TSArray <PPOsm::Node> & rList, TSArray <PPOsm::NodeClusterStatEntry> * pStat);
	int    StoreGeoWayList(const TSCollection <PPOsm::Way> & rList, TSArray <PPOsm::WayStatEntry> * pStat);
	int    StoreGeoNodeWayRefList(const LLAssocArray & rList);
//private:
public:
	BDbDatabase      * P_Db;
	SrGrammarTbl     * P_GrT;
	SrWordTbl        * P_WdT;
	SrWordAssocTbl   * P_WaT;
	SrConceptTbl     * P_CT;
	SrConceptPropTbl * P_CpT;
	SrNGramTbl       * P_NgT;
	SrConceptNgTbl   * P_CNgT;
	SrGeoNodeTbl     * P_GnT;
	SrGeoWayTbl      * P_GwT;
	LEXID  ZeroWordID;
private:
	int    Helper_GetConceptHier(CONCEPTID cID, Int64Array & rConceptHier);
	int    Helper_MakeConceptProp(const SrCPropDeclList & rPdl, const char * pPropSymb, SrCProp & rProp, CONCEPTID cID);

	CONCEPTID PropInstance; // :crp_instance
	CONCEPTID PropSubclass; // :crp_subclass
	CONCEPTID PropType;     // :crp_type
};
