#pragma once

#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>

#include <sstream> //stringstream
#include <algorithm> //std::transform, search

// Structure to store the address process infromation.
struct ProcessAddressInformation
{
	LPVOID lpProcessPEBAddress;
	LPVOID lpProcessImageBaseAddress;
};


#define REMOVE_SPACES(x) x.erase(std::remove(x.begin(), x.end(), ' '), x.end())

#define RTRIM(x) x.erase(x.begin(), std::find_if(x.begin(), x.end(), [](unsigned char ch) \
																					{ \
																						return !std::isspace(ch); \
																					}));

#define LTRIM(x) x.erase(std::find_if(x.rbegin(), x.rend(), [](unsigned char ch) \
																			{ \
																				return !std::isspace(ch); \
																			}).base(), x.end());

#define TRIM(x) RTRIM(x) LTRIM(x)

std::wstring GetExePathW();
std::wstring GetINIInfo(std::wstring lpAppName, std::wstring lpKeyName, std::wstring lpDefault, std::wstring File);

BOOL EnableDebugPrivileges();

HANDLE GetFileContent(const std::wstring lpFilePath);
BOOL IsValidPE(const LPVOID lpImage);
ProcessAddressInformation GetProcessAddressInformation32(const PPROCESS_INFORMATION lpPI);
ProcessAddressInformation GetProcessAddressInformation64(const PPROCESS_INFORMATION lpPI);
void CleanAndExitProcess(const LPPROCESS_INFORMATION lpPI, const HANDLE hFileContent);
void CleanProcess(const LPPROCESS_INFORMATION lpPI, const HANDLE hFileContent);

template<typename StringType>
std::vector<StringType>& split(const StringType& s, StringType delim, std::vector<StringType>& elems)
{
	std::stringstream ss(s);
	StringType item;

	while (getline(ss, item, delim))
	{
		elems.push_back(item);
	}

	return elems;
}

template<typename StringType>
std::vector<StringType> split(const StringType& s, StringType delim)
{
	std::vector<StringType> elems;

	split(s, delim, elems);

	return elems;
}

template <typename StringType>
std::vector<StringType> Split(const StringType& s, const StringType& delim, const bool keep_empty)
{
	std::vector<StringType> result;

	if (delim.empty())
	{
		result.push_back(s);

		return result;
	}

	//std::string::const_iterator substart = s.begin(), subend;
	auto substart = s.begin();  // Utiliza auto para deducir el tipo de iterador
	auto subend = substart;

	while (true)
	{
		subend = std::search(substart, s.end(), delim.begin(), delim.end());
		StringType temp(substart, subend);

		if (keep_empty || !temp.empty())
		{
			result.push_back(temp);
		}

		if (subend == s.end())
		{
			break;
		}

		substart = subend + delim.size();
	}

	return result;
}