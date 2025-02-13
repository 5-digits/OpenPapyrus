// Sha256Reg.cpp

#include <7z-internal.h>
#pragma hdrstop
#include <7z-ifcs.h>

class CSha256Hasher : public IHasher, public CMyUnknownImp {
	CSha256 _sha;
	Byte mtDummy[1 << 7];
public:
	CSha256Hasher() 
	{
		Sha256_Init(&_sha);
	}
	MY_UNKNOWN_IMP1(IHasher)
	INTERFACE_IHasher(; )
};

STDMETHODIMP_(void) CSha256Hasher::Init() throw() { Sha256_Init(&_sha); }
STDMETHODIMP_(void) CSha256Hasher::Update(const void * data, uint32 size) throw() { Sha256_Update(&_sha, (const Byte*)data, size); }
STDMETHODIMP_(void) CSha256Hasher::Final(Byte *digest) throw() { Sha256_Final(&_sha, digest); }

REGISTER_HASHER(CSha256Hasher, 0xA, "SHA256", SHA256_DIGEST_SIZE)
