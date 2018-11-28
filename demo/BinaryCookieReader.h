#ifndef _COMM_BINARY_COOKIE_READER_H
#define _COMM_BINARY_COOKIE_READER_H

#include <string>
#include <vector>
#include <sstream>

typedef struct _T_BIN_COOKIES_INFO
{
	std::string strCookieName;
	std::string strCookieValue;
	std::string strDomainValue;
	std::string strCookiePath;
	std::string strExpireDate;
	std::string strCookieFlag;
}T_BIN_COOKIES_INFO;

class CBINARY_COOKIE_READER
{
public:
	CBINARY_COOKIE_READER();
	~CBINARY_COOKIE_READER();

public:
	bool isValidCookieFile(const std::string &strFilePath);
	std::vector<std::string> getBinCookies();
private:
	void setPageNumber();
	int getNextIntergerFromMemory(const std::string &strMemory, int nPos, bool bIsBigEndian = false);
	double getNextDoubleFromMemory(const std::string &strMemory, int nPos);
	std::string convertCookieFlag(int nSrcFlag);
	std::string convertTimeFormat(double dSrcTime);
	std::string getDomainValue(const std::string &strCookie, int nOffset);
	void setPageSizes();
	void setPageContent();
	void setBinCookies();
	void writeBinCookiesVec(const T_BIN_COOKIES_INFO &tBinInfo);
private:
	std::stringstream m_streamBuf;
	int m_nPageNum;
	int m_nCurrentReadPos;
	std::vector<int> m_vecPageSizes;
	std::vector<std::string> m_vecEachPageContent;
	std::vector<std::string> m_vecBinCookies;
	std::string m_strFilePath;
};
#endif