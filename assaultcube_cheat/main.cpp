#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>

DWORD getModuleBaseAddress(const wchar_t* lpszModuleName, DWORD processID) {
	DWORD moduleBaseAddress = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
	MODULEENTRY32 moduleEntry32 = { 0 };
	moduleEntry32.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(snapshot, &moduleEntry32))
	{
		do {
			if (_tcscmp(moduleEntry32.szModule, lpszModuleName) == 0)
			{
				moduleBaseAddress = (DWORD)moduleEntry32.modBaseAddr;
				break;
			}
		} while (Module32Next(snapshot, &moduleEntry32));
	}
	CloseHandle(snapshot);
	return moduleBaseAddress;
}

DWORD getPointerAddress(HWND windowHandle, DWORD gameBaseAddress, DWORD address, std::vector<DWORD> offsets)
{
	DWORD processID = NULL;
	GetWindowThreadProcessId(windowHandle, &processID);
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (processHandle == NULL)
	{
		std::cout << "error: processHandle returned NULL\n";
		return EXIT_FAILURE;
	}

	DWORD offset_null = NULL;
	ReadProcessMemory(processHandle, (LPVOID*)(gameBaseAddress + address), &offset_null, sizeof(offset_null), 0);
	DWORD pointeraddress = offset_null;
	for (int i = 0; i < offsets.size() - 1; i++)
	{
		ReadProcessMemory(processHandle, (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
	}
	return pointeraddress += offsets.at(offsets.size() - 1);
}

int main()
{
	HWND windowHandle = FindWindowA(NULL, ("AssaultCube"));
	if (windowHandle == NULL)
	{
		std::cout << "error: windowHandle returned NULL\n";
		return EXIT_FAILURE;
	}

	DWORD processID;
	GetWindowThreadProcessId(windowHandle, &processID);

	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (processHandle == 0)
	{
		std::cout << "error: processHandle returned 0\n";
		return EXIT_FAILURE;
	}

	uintptr_t baseAddress = getModuleBaseAddress(L"ac_client.exe", processID);
	DWORD ammoAddress = 0x00183828;
	std::vector<DWORD> ammoOffsets{ 0x8, 0x748, 0x30, 0x8F4 };

	DWORD ammoPointerAddress = getPointerAddress(windowHandle, baseAddress, ammoAddress, ammoOffsets);

	while (true)
	{
		int ammo = 20;
		WriteProcessMemory(processHandle, (LPVOID*)(ammoPointerAddress), &ammo, 4, 0);
	}
	return EXIT_SUCCESS;
}
