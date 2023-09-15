#include "cProcessor.h"

cProcessor::cProcessor()
{
	this->Path = GetExePathW();
	this->File = L"Fixer.ini";

	this->FullPath = this->Path + this->File;

	this->Items = 0;
}

cProcessor::~cProcessor()
{

}

BOOL cProcessor::FileExist(std::wstring file)
{
	return (GetFileAttributes(file.c_str()) != INVALID_FILE_ATTRIBUTES);
}

BOOL cProcessor::IsINIAvailable()
{
	return this->FileExist(this->FullPath);
}

BOOL cProcessor::TargetExist()
{
	return this->FileExist(this->Path + this->Target);
}

std::wstring cProcessor::GetFullTargetPath()
{
	return this->Path + this->Target;
}

BOOL cProcessor::IsTargetAvailable()
{
	/*
	[INIT]
	Target=target.exe
	*/
	this->Target = GetINIInfo(L"INIT", L"Target", L"", this->FullPath.c_str());

	TRIM(this->Target);

	return this->Target.size();
}

void cProcessor::GetArguments()
{
	/*
	[INIT]
	Arguments=/im explorer.exe /f
	*/
	this->Arguments = GetINIInfo(L"INIT", L"Arguments", L"", this->FullPath.c_str());

	TRIM(this->Arguments);
}

std::wstring cProcessor::GetItemOffset(size_t index)
{
	/*
	[Item1]
	Offset=0x9B43C
	*/

	std::wstring ItemTag = L"Item" + std::to_wstring(index);

	std::wstring sRet = GetINIInfo(ItemTag.c_str(), L"Offset", L"", this->FullPath.c_str());

	TRIM(sRet);

	return sRet;
}

std::wstring cProcessor::GetItemValue(size_t index)
{
	/*
	[Item1]
	Value=0x90,0x90,0x90,0x90,0x90,0x90
	*/

	std::wstring ItemTag = L"Item" + std::to_wstring(index);

	std::wstring sRet = GetINIInfo(ItemTag.c_str(), L"Value", L"", this->FullPath.c_str());

	TRIM(sRet);

	return sRet;
}

void cProcessor::WriteStructureFile()
{
	/*
	[INIT]
	Target=target.exe
	Items=1
	*/
	WritePrivateProfileString(L"INIT", L"Target", L"calc.exe", this->FullPath.c_str());
	WritePrivateProfileString(L"INIT", L"Items", L"1", this->FullPath.c_str());

	/*
	[Item1]
	Offset=0x9B43C
	Value=0x14,0x0,0x0,0x0
	*/
	WritePrivateProfileString(L"Item1", L"Offset", L"0x9B43C", this->FullPath.c_str());
	WritePrivateProfileString(L"Item1", L"Value", L"0x14,0x0,0x0,0x0", this->FullPath.c_str());
}

UINT cProcessor::ItemsAvailable()
{
	/*
	[INIT]
	Items=1
	*/

	this->Items = GetPrivateProfileInt(L"INIT", L"Items", 0, this->FullPath.c_str());

	
	return this->Items;
}