#ifndef __EMAILMESSAGE_H__
#define __EMAILMESSAGE_H__


class EmailMessage
{
public:
	void SetAddress(const std::string&);
	void SetTitle(const std::string&);
	void SetText(const std::string&);
	const std::string& Text() const
	{ return _txt; }
	const std::string& Title() const
	{ return _title; }
	const std::string& Address() const
	{ return _address; }
private:
	std::string _address;
	std::string _title;
	std::string _txt;

};

#endif
