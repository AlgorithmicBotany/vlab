#ifndef __LSTUDIOLOAD_H__
#define __LSTUDIOLOAD_H__


/* a4b5c086-142e-48b3-8fd5-06237cabb984 */
DEFINE_GUID(CLSID_Lstudio, 
    0xa4b5c086,
    0x142e,
    0x48b3,
    0x8f, 0xd5, 0x06, 0x23, 0x7c, 0xab, 0xb9, 0x84);

/* 255bd095-6f94-470e-8b61-d8b7604e90f4 */
DEFINE_GUID(IID_ILoadObject, 
    0x255bd095,
    0x6f94,
    0x470e,
    0x8b, 0x61, 0xd8, 0xb7, 0x60, 0x4e, 0x90, 0xf4);


#ifdef INTERFACE
#undef INTERFACE
#endif

#define INTERFACE ILoadObject

DECLARE_INTERFACE_(ILoadObject, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) = 0;
	STDMETHOD_(ULONG, AddRef)(THIS) = 0;
	STDMETHOD_(ULONG, Release)(THIS) = 0;

	STDMETHOD_(bool, Load)(THIS_ const char*) = 0;
};


#else
	#error File already included
#endif
