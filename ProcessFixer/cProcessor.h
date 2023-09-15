#pragma once

#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>

#include "Utilities.h"

class cProcessor
{
public:
	cProcessor();
	~cProcessor();

	BOOL IsTargetAvailable();
	void GetArguments();
	BOOL IsINIAvailable();
	UINT ItemsAvailable();

	BOOL FileExist(std::wstring file);

	BOOL TargetExist();

	std::wstring GetFullTargetPath();

	std::wstring GetItemOffset(size_t index);
	std::wstring GetItemValue(size_t index);

	void WriteStructureFile();

	std::wstring Target;
	std::wstring Arguments;
	UINT Items;
	BOOL Target32;

	std::wstring File;
	std::wstring Path;
	std::wstring FullPath;
};