#include <Windows.h>

#include "tGuid.h"

#include <cassert>
#include <iomanip>
#include <objbase.h>


VLB::tGUID::tGUID()
{
	__uuid.Data1 = 0;
	__uuid.Data2 = 0;
	__uuid.Data3 = 0;
	__uuid.Data4[0] = __uuid.Data4[1] = __uuid.Data4[2] = __uuid.Data4[3] = 
	__uuid.Data4[4] = __uuid.Data4[5] = __uuid.Data4[6] = __uuid.Data4[7] = 0;
}


void VLB::tGUID::Generate()
{
	::CoCreateGuid(&__uuid);
}


bool VLB::tGUID::Compare(const VLB::tGUID& rhs) const
{
	if (__uuid.Data1 != rhs.__uuid.Data1)
		return false;
	if (__uuid.Data2 != rhs.__uuid.Data2)
		return false;
	if (__uuid.Data3 != rhs.__uuid.Data3)
		return false;
	if (__uuid.Data4[0] != rhs.__uuid.Data4[0])
		return false;
	if (__uuid.Data4[1] != rhs.__uuid.Data4[1])
		return false;
	if (__uuid.Data4[2] != rhs.__uuid.Data4[2])
		return false;
	if (__uuid.Data4[3] != rhs.__uuid.Data4[3])
		return false;
	if (__uuid.Data4[4] != rhs.__uuid.Data4[4])
		return false;
	if (__uuid.Data4[5] != rhs.__uuid.Data4[5])
		return false;
	if (__uuid.Data4[6] != rhs.__uuid.Data4[6])
		return false;
	if (__uuid.Data4[7] != rhs.__uuid.Data4[7])
		return false;

	return true;
}

bool VLB::tGUID::IsNull() const
{
	if (__uuid.Data1 != 0)
		return false;
	if (__uuid.Data2 != 0)
		return false;
	if (__uuid.Data3 != 0)
		return false;
	for (int iItem=0; iItem<8; ++iItem)
	{
		if (__uuid.Data4[iItem] != 0)
			return false;
	}

	return true;
}


void VLB::tGUID::ToString(std::string& outString) const
{
	const int kOutputLength = 38;
	char buffer[kOutputLength+1];
	int iResult = sprintf(buffer, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		__uuid.Data1,		__uuid.Data2,		__uuid.Data3,
		__uuid.Data4[0],	__uuid.Data4[1],	__uuid.Data4[2],	__uuid.Data4[3], 
		__uuid.Data4[4],	__uuid.Data4[5],	__uuid.Data4[6],	__uuid.Data4[7]); 
	assert(iResult == kOutputLength);
	outString = buffer;
}

void VLB::tGUID::FromString(const std::string& inString)
{
	unsigned int data2, data3;
	unsigned int data4[8];
	unsigned int iResult = sscanf(inString.c_str(), "{%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x}",
		&__uuid.Data1,		&data2,		&data3,
		&data4[0],	&data4[1],	&data4[2],	&data4[3], 
		&data4[4],	&data4[5],	&data4[6],	&data4[7]); 

	if (iResult == 11)
	{
		__uuid.Data2 = static_cast<unsigned short>(data2);
		__uuid.Data3 = static_cast<unsigned short>(data3);

		for (int iItem=0; iItem<8; ++iItem)
		{
			__uuid.Data4[iItem] = static_cast<unsigned char>(data4[iItem]);
		}
	}
}
