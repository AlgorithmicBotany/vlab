#ifndef __VLBOPTIONS_H__
#define __VLBOPTIONS_H__


namespace VLB
{

class Options
{
public:
	Options(int, const std::string&, int);
	Options();
	void Load(const std::string&);
	int IconWidth() const
	{ return _IconWidth; }
	int FontSize() const
	{ return _FontSize; }
	const std::string& FontName() const
	{ return _FontName; }
	const std::string& Host() const
	{ return _Host; }
	const std::string& Oofs() const
	{ return _Oofs; }
	const std::string& User() const
	{ return _User; }
	const std::string& Password() const
	{ return _Paswd; }
	const string_buffer& Crlf() const
	{ return _Crlf; }
	const string_buffer& Ignored() const
	{ return _Ignored; }
	COLORREF BgColor() const
	{ return _BgColor; }
private:

	enum eLabel
	{
		lblUnknown = -1,
		lblCrlf = 0,
		lblIgnored,
		lblLogon,
		lblBackgroundColor,

		lblLastLabel
	};

	eLabel Label(const std::string&, int&) const;
	void ReadCrLf(const std::string&);
	void ReadIgnored(const std::string&);
	void AddBuffer(const std::string&, string_buffer&);
	void ReadLogon(const std::string&);
	COLORREF ReadColor(const std::string&);

	int _IconWidth;
	int _FontSize;
	COLORREF _BgColor;
	std::string _FontName;
	std::string _Host;
	std::string _Oofs;
	std::string _User;
	std::string _Paswd;
	string_buffer _Crlf;
	string_buffer _Ignored;
};

extern Options options;

}

#endif
