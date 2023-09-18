#include <iostream>
#include <Windows.h>

#include "cProcessor.h"
#include "Utilities.h"

int main(const int argc, char* argv[])
{
	cProcessor MainProcessor;
	UINT Items;

	STARTUPINFO SI;
	PROCESS_INFORMATION PI;

	ZeroMemory(&SI, sizeof(SI));
	SI.cb = sizeof(SI);
	ZeroMemory(&PI, sizeof(PI));

	bool silentMode = false;
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];
			std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);

			if (arg == "/silent")
			{
				silentMode = true;
				break;
			}
		}
	}

	EnableDebugPrivileges();

	if (!MainProcessor.IsINIAvailable())
	{
		std::wcout << L"[-] INI file does not exist, proceed to create a sample one." << std::endl;

		MainProcessor.WriteStructureFile();

		return EXIT_FAILURE;
	}

	if (!MainProcessor.IsTargetAvailable())
	{
		std::wcout << L"[-] Missing Target! Add like" << std::endl;
		std::wcout << L"\t" << L"[INIT]" << std::endl;
		std::wcout << L"\t" << L"Target=taskkill" << std::endl;
		std::wcout << L"\t" << L"Arguments=/im explorer.exe /f" << std::endl;

		return EXIT_FAILURE;
	}

	MainProcessor.GetArguments();

	Items = MainProcessor.ItemsAvailable();

	if (Items == 0)
	{
		std::wcout << L"[-] Missing Items! Add like" << std::endl;
		std::wcout << L"\t" << L"[INIT]" << std::endl;
		std::wcout << L"\t" << L"Items=1" << std::endl << std::endl;
		std::wcout << L"\t" << L"[Item1]" << std::endl;
		std::wcout << L"\t" << L"Offset=0x9B43C" << std::endl;
		std::wcout << L"\t" << L"Value=0x14,0x0,0x0,0x0" << std::endl;

		return EXIT_FAILURE;
	}

	std::wcout << L"[+] Target: " << MainProcessor.Target.c_str() << std::endl;
	std::wcout << L"[+] Arguments: " << MainProcessor.Arguments.c_str() << std::endl;
	std::wcout << L"[+] Items: " << Items << std::endl << std::endl;

	if (!MainProcessor.TargetExist())
	{
		std::wcout << L"[-] Target does not exist in " << MainProcessor.Path.c_str() << std::endl;

		return EXIT_FAILURE;
	}

	const LPVOID hFileContent = GetFileContent(MainProcessor.GetFullTargetPath().c_str());

	if (hFileContent == nullptr)
	{
		std::wcout << L"[-] Could not get File Content!" << std::endl;

		return EXIT_FAILURE;
	}

	if (!IsValidPE(hFileContent))
	{
		std::wcout << L"[-] The PE file is not valid!" << std::endl << std::endl;

		CleanAndExitProcess(&PI, hFileContent);

		return EXIT_FAILURE;
	}

	std::wcout << L"[+] Target file has a valid PE." << std::endl << std::endl;

	std::wstring FullPathArgs = MainProcessor.GetFullTargetPath();
	FullPathArgs += L" ";
	FullPathArgs += MainProcessor.Arguments;

	wchar_t cmdLine[MAX_PATH];
	wcscpy_s(cmdLine, MAX_PATH, FullPathArgs.c_str());

	//const BOOL bProcessCreation = CreateProcessW(MainProcessor.GetFullTargetPath().c_str(), nullptr, nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &SI, &PI);
	const BOOL bProcessCreation = CreateProcessW(nullptr, cmdLine, nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &SI, &PI);

	if (!bProcessCreation)
	{
		std::wcout << GetLastError() << L" (0x" << std::hex << GetLastError() << L")" << std::endl;
		std::wcout << L"[-] An error is occured when trying to create the target process!" << std::endl;

		CleanAndExitProcess(&PI, hFileContent);

		return EXIT_FAILURE;
	}

	IsWow64Process(PI.hProcess, &MainProcessor.Target32);

	std::wcout << L"[+] Target architecture " << (MainProcessor.Target32 ? L"x86" : L"x64") << std::endl << std::endl;

	ProcessAddressInformation ProcessAddressInformation = { nullptr, nullptr };

	if (MainProcessor.Target32)
	{
		ProcessAddressInformation = GetProcessAddressInformation32(&PI);

		if (ProcessAddressInformation.lpProcessImageBaseAddress == nullptr || ProcessAddressInformation.lpProcessPEBAddress == nullptr)
		{
			printf("[-] An error is occured when trying to get the image base address of the target process!\n");

			CleanAndExitProcess(&PI, hFileContent);

			return EXIT_FAILURE;
		}
	}
	else
	{
		ProcessAddressInformation = GetProcessAddressInformation64(&PI);

		if (ProcessAddressInformation.lpProcessImageBaseAddress == nullptr || ProcessAddressInformation.lpProcessPEBAddress == nullptr)
		{
			printf("[-] An error is occured when trying to get the image base address of the target process!\n");

			CleanAndExitProcess(&PI, hFileContent);

			return EXIT_FAILURE;
		}
	}

	printf("[+] Target Process PEB : 0x%p\n", ProcessAddressInformation.lpProcessPEBAddress);
	printf("[+] Target Process Image Base : 0x%p\n", ProcessAddressInformation.lpProcessImageBaseAddress);

	std::cout << std::endl;

	for (UINT i = 1; i <= Items; i++)
	{
		/*
		[Item1]
		Address=0x9B43C
		Value=0x14,0x0,0x0,0x0
		*/

		std::wstring Offset = MainProcessor.GetItemOffset(i);
		size_t Address = (size_t)ProcessAddressInformation.lpProcessImageBaseAddress;

		std::wstringstream sstream(Offset);
		size_t ddOffset;
		size_t bValue = 0;
		size_t FinalAddress = 0;

		sstream >> std::hex >> ddOffset;

		if (Offset.size())
		{
			sstream.clear();
			sstream << Offset;

			std::wcout << L"[+] Index " << i << L" Offset: " << Offset.c_str() << std::endl;
			//std::wcout << L"[+] Index " << i << L" Offset: " << ddOffset << std::endl;

			std::wstring Value;
			std::wstring delim = L",";

			Value = MainProcessor.GetItemValue(i);

			if (Value.size())
			{
				std::wcout << L"[+] Index " << i << L" Value: " << Value.c_str() << std::endl;

				std::vector<std::wstring> splt;

				splt = Split(Value, delim, true);

				sstream.clear();

				for (size_t i = 0; i < splt.size(); i++)
				{
					TRIM(splt[i]);
					sstream.str(splt[i]);
					sstream >> std::hex >> bValue;

					bValue &= 0xFF;

					//std::wcout << bValue << std::endl;

					FinalAddress = (Address + ddOffset + i);

					std::wcout << L"[+] Final Address: 0x" << std::hex << FinalAddress << L" - Writting 0x" << bValue << std::dec << std::endl;

					const BOOL bWriteHeaders = WriteProcessMemory(PI.hProcess, (LPVOID)(FinalAddress), &bValue, 0x01, nullptr);
					if (!bWriteHeaders)
					{
						std::wcout << GetLastError() << L" (0x" << std::hex << GetLastError() << L")" << std::endl;
						printf("[-] An error is occured when trying to write memory.\n");
						//break;
					}

					sstream.clear();
				}
			}
			else
			{
				std::wcout << L"[-] Index " << i << L" INVALID Value" << std::endl;
			}
		}
		else
		{
			std::wcout << L"[-] Index " << i << L" INVALID Offset" << std::endl;
		}

		std::cout << std::endl;
	}

	if (!silentMode)
	{
		std::wcout << "[+] Wait... Press any key to resume process" << std::endl;
		std::cin.get();
	}

	ResumeThread(PI.hThread);

	CleanProcess(&PI, hFileContent);

	std::wcout << "[+] Exit" << std::endl;

	return EXIT_SUCCESS;
}
