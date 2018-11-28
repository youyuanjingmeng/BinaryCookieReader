#include "BinaryCookieReader.h"
#include "Endian.h"
#include <fstream>
#include <time.h>

const std::string g_strFileHeader = "cook";
const int g_nHeaderLen = 4;
const int g_nNumPagesLen = 4;
const int g_nEachCookiesLen = 4;
const int g_nDateTypeLen = 8;
const int g_nCommTypeLen = 4;
const int g_nEndCookieLen = 8;

CBINARY_COOKIE_READER::CBINARY_COOKIE_READER() : m_nCurrentReadPos(0)
{

}

CBINARY_COOKIE_READER::~CBINARY_COOKIE_READER()
{

}

bool CBINARY_COOKIE_READER::isValidCookieFile(const std::string &strFilePath)
{
	if (strFilePath.empty() || strFilePath.length() < 2)
	{
		return false;
	}
	bool bIsValidFile = false;
	std::ifstream fStream(strFilePath, std::ios::binary);
	if (!fStream.is_open())
	{
		return bIsValidFile;
	}

	m_streamBuf << fStream.rdbuf();

	std::string strHeader;
	strHeader.assign(m_streamBuf.str().c_str(), g_nHeaderLen);
	if (strHeader == g_strFileHeader)
	{
		//头部四字节固定，为"cook"
		m_nCurrentReadPos += g_nHeaderLen;
		bIsValidFile = true;
	}
	fStream.close();

	return bIsValidFile;
}


// Number of pages
void CBINARY_COOKIE_READER::setPageNumber()
{
	// 大端存储
	m_nPageNum = getNextIntergerFromMemory(m_streamBuf.str(), m_nCurrentReadPos, true);
	m_nCurrentReadPos += g_nNumPagesLen;
}

// Each page size
void CBINARY_COOKIE_READER::setPageSizes()
{
	for (int i = 0; i < m_nPageNum; i++)
	{
		//大端存储
		int nPageSize = getNextIntergerFromMemory(m_streamBuf.str(), m_nCurrentReadPos, true);
		m_vecPageSizes.push_back(nPageSize);
		m_nCurrentReadPos += g_nNumPagesLen;
	}
}

// Each page content
void CBINARY_COOKIE_READER::setPageContent()
{
	int nPos = 0;
	for (auto iter : m_vecPageSizes)
	{
		std::string strPageContent;
		strPageContent.assign(m_streamBuf.str().c_str() + m_nCurrentReadPos + nPos, iter);
		nPos = iter;
		m_vecEachPageContent.push_back(strPageContent);
	}
}

int CBINARY_COOKIE_READER::getNextIntergerFromMemory(const std::string &strMemory, int nPos, bool bIsBigEndian /* = false */)
{
	int nNextInteger = 0;
	std::string strNextInteger;
	strNextInteger.assign(strMemory.c_str() + nPos, sizeof(int));
	int nTmpInteger = 0;
	memcpy(&nTmpInteger, strNextInteger.c_str(), sizeof(int));
	if (bIsBigEndian)
	{
		CENDIAN_CONVERT endianConvert;
		nNextInteger = endianConvert.swap32(nTmpInteger);
	}
	else
	{
		//小端存储
		nNextInteger = nTmpInteger;
	}

	return nNextInteger;
}

double CBINARY_COOKIE_READER::getNextDoubleFromMemory(const std::string &strMemory, int nPos)
{
	double nNextDouble = 0;
	std::string strNextDoule;
	strNextDoule.assign(strMemory.c_str() + nPos, sizeof(double));
	//小端存储
	memcpy(&nNextDouble, strNextDoule.c_str(), sizeof(double));
	return nNextDouble;
}

std::vector<std::string> CBINARY_COOKIE_READER::getBinCookies()
{
	setPageNumber();
	setPageSizes();
	setPageContent();
	setBinCookies();
	return m_vecBinCookies;
}

// 
void CBINARY_COOKIE_READER::setBinCookies()
{
	for (auto iter : m_vecEachPageContent)
	{
		//Every page starts with a 4 byte page header: 0x00000100.
		int nCurPos = g_nCommTypeLen;

		// Next four bytes is an integer specifying the number of cookies in the page.
		int nCookies = getNextIntergerFromMemory(iter, nCurPos);
		nCurPos += g_nCommTypeLen;

		std::vector<int> vecEachPageCookiesOff;  //每一个页的cookies偏移量
		for (int i = 0; i < nCookies; i++)
		{
			int nOffset = getNextIntergerFromMemory(iter, nCurPos);
			vecEachPageCookiesOff.push_back(nOffset);

			nCurPos += sizeof(int);
		}
		// end of page header: Always 00000000
		nCurPos += g_nCommTypeLen;

		for (auto iterCookie : vecEachPageCookiesOff)
		{
			T_BIN_COOKIES_INFO tBinCookies;
			// 遍历每一个cookies
			int nCookReadPos = iterCookie;
			// fetch cookie size
			int nCookieSize = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += sizeof(int);
			// read the complete cookie 
			std::string strCookie;
			strCookie.assign(iter.c_str() + nCookReadPos, nCookieSize);

			//4 bytes,unknown
			nCookReadPos += g_nCommTypeLen;

			// 4bytes, Cookie flags:  1=secure, 4=httponly, 5=secure+httponly
			int nCookieFlag = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += g_nCommTypeLen;
			tBinCookies.strCookieFlag = convertCookieFlag(nCookieFlag);

			// The next 4 bytes are unknown.
			nCookReadPos += g_nCommTypeLen;

			// The next 4 bytes is an integer specifying the start of the url field in bytes from the start of the cookie record.
			int nUrlOffset = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += g_nCommTypeLen;
			// The next 4 bytes is an integer specifying the start of the name field in bytes from the start of the cookie record.
			int nNameOffset = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += g_nCommTypeLen;

			// The next 4 bytes is an integer specifying the start of the path field in bytes from the start of the cookie record.
			int nPathOffset = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += g_nCommTypeLen;

			// The next 4 bytes is an integer specifying the start of the value field in bytes from the start of the cookie record.
			int nValueOffset = getNextIntergerFromMemory(iter, nCookReadPos);
			nCookReadPos += g_nCommTypeLen;

			// The next 8 bytes represents the end of the cookie and it is always 0x0000000000000000.
			nCookReadPos += g_nEndCookieLen;

			// The next 8 bytes are the cookie expiration date. Date is in Mac epoch format (Mac absolute time). Mac epoch format starts from Jan 2001.
			//T_BIN_COOKIES_INFO tBinCookies;
			double dExpireDate = getNextDoubleFromMemory(iter, nCookReadPos) + 978307200;
			tBinCookies.strExpireDate = convertTimeFormat(dExpireDate);
			nCookReadPos += g_nDateTypeLen;

			// The next 8 bytes are the cookie creation date.
			double dCreateDate = getNextDoubleFromMemory(iter, nCookReadPos) + 978307200;
			std::string strCreateDate = convertTimeFormat(dCreateDate);
			nCookReadPos += g_nDateTypeLen;

			// fetch domain value from url offset
			tBinCookies.strDomainValue = getDomainValue(strCookie, nUrlOffset);

			// fetch cookie name from path offset
			tBinCookies.strCookieName = getDomainValue(strCookie, nNameOffset);

			// fetch cookie path from path offset
			tBinCookies.strCookiePath = getDomainValue(strCookie, nPathOffset);

			// fetch cookie value from value offset
			tBinCookies.strCookieValue = getDomainValue(strCookie, nValueOffset);

			writeBinCookiesVec(tBinCookies);
		}
	}
}

void CBINARY_COOKIE_READER::writeBinCookiesVec(const T_BIN_COOKIES_INFO &tBinInfo)
{
	std::string strBinCookies = "Cookie:" + tBinInfo.strCookieName + "=" + tBinInfo.strCookieValue + ";";
	strBinCookies += "domain=" + tBinInfo.strDomainValue + ";";
	strBinCookies += "path=" + tBinInfo.strCookiePath + ";";
	strBinCookies += "expires=" + tBinInfo.strExpireDate + ";";
	strBinCookies += tBinInfo.strCookieFlag;

	m_vecBinCookies.push_back(strBinCookies);
}

std::string CBINARY_COOKIE_READER::getDomainValue(const std::string &strCookie, int nOffset)
{
	std::string strDomainVale;
	int nCharNumbers = 0;
	strDomainVale.assign(strCookie.c_str() + nOffset - 4, strCookie.length() - nOffset + 4);
	char *pTmp = (char *)strDomainVale.data();
	while (0 != memcmp(pTmp, "\0", 1))
	{
		nCharNumbers++;
		pTmp++;
	}
	strDomainVale.assign(strCookie.c_str() + nOffset - 4, nCharNumbers);
	return strDomainVale;
}

std::string CBINARY_COOKIE_READER::convertCookieFlag(int nSrcFlag)
{
	std::string strCookieFlag;

	switch (nSrcFlag)
	{
	case 1:
		strCookieFlag = "Secure";
		break;
	case 4:
		strCookieFlag = "HttpOnly";
		break;
	case 5:
		strCookieFlag = "Secure;HttpOnly";
		break;
	default:
		strCookieFlag = "Unknown";
	}
	return strCookieFlag;
}

std::string CBINARY_COOKIE_READER::convertTimeFormat(double dSrcTime)
{
	std::string strTime;
	time_t rawtime = dSrcTime;
	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);

	char szBufTime[80] = { 0 };
	strftime(szBufTime, 80, "%a, %d %b %Y", timeinfo);
	strTime = szBufTime;
	return strTime;
}