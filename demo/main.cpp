#include <map>
#include <string>
#include "BinaryCookieReader.h"

int main()
{
	CBINARY_COOKIE_READER cReader;
	bool bIsValid = cReader.isValidCookieFile("D:\\1.binarycookies");
	std::vector<std::string> vecBinCookies;
	if (bIsValid)
	{
		vecBinCookies = cReader.getBinCookies();
	}
}