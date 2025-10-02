#ifndef __TGUID_H__
#define __TGUID_H__


#include <ostream>

namespace VLB
{

class tGUID
{
public:
	tGUID();

	void Generate();

	void FromString(const std::string& inString);
	void ToString(std::string& outString) const;

	bool Compare(const tGUID& rhs) const;
	bool IsNull() const;
private:
	::GUID __uuid;
};


} // namespace VLB

inline bool operator==(const VLB::tGUID& lhs, const VLB::tGUID& rhs)
{
	return lhs.Compare(rhs);
}

#endif
