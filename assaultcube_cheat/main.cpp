#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>


DWORD GetModuleBaseAddress(const wchar_t* lpszModuleName, DWORD processID) {
	DWORD moduleBaseAddress = 0;
	// takes a snapshot of all modules and more with in the specified processes
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "error: invalid handle value\n";
		return EXIT_FAILURE;
	}
	MODULEENTRY32 moduleEntry32 = { 0 };
	// if you do not initialize dwSize, Module32First fails
	moduleEntry32.dwSize = sizeof(MODULEENTRY32);
	// retrieves and stores information about the first module associated with the process
	if (Module32First(snapshot, &moduleEntry32))
	{
		do {
			// if found module matches module we are looking for, get base address
			if (_tcscmp(moduleEntry32.szModule, lpszModuleName) == 0)
			{
				moduleBaseAddress = (DWORD)moduleEntry32.modBaseAddr;
				break;
			}
			// retrieves and store information about the next module in Module32Next
		} while (Module32Next(snapshot, &moduleEntry32));
	}
	CloseHandle(snapshot);
	return moduleBaseAddress;
}

DWORD GetPointerAddress(HWND windowHandle, DWORD baseAddress, DWORD address, std::vector<DWORD> offsets)
{
	DWORD processID = NULL;
	GetWindowThreadProcessId(windowHandle, &processID);
	if (processID == 0)
	{
		std::cout << "processID return 0\n";
	}

	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (processHandle == NULL)
	{
		std::cout << "error: processHandle returned NULL\n";
		return EXIT_FAILURE;
	}

	DWORD pointerAddress = NULL;
	// copies the data in the specified address range from the address space of the specified process
	ReadProcessMemory(processHandle, (LPVOID*)(baseAddress + address), &pointerAddress, sizeof(pointerAddress), 0);
	// we don't want to change the last offset value so we do -1
	for (int i = 0; i < offsets.size() - 1; i++)
	{
		ReadProcessMemory(processHandle, (LPVOID*)(pointerAddress + offsets.at(i)), &pointerAddress, sizeof(pointerAddress), 0);
	}
	return pointerAddress += offsets.at(offsets.size() - 1);
}

int main()
{	// retrieves a handle to the top-level window
	HWND windowHandle = FindWindowA(NULL, ("AssaultCube"));
	if (windowHandle == NULL)
	{
		std::cout << "error: windowHandle returned NULL\n";
		return EXIT_FAILURE;
	}

	DWORD processID = NULL;
	// retrieves the identifier of the thread that created the specified window
	GetWindowThreadProcessId(windowHandle, &processID);
	if (processID == 0)
	{
		std::cout << "processID return 0\n";
	}

	// opens an existing local process object
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (processHandle == NULL)
	{
		std::cout << "error: processHandle returned NULL\n";
		return EXIT_FAILURE;
	}

	uintptr_t baseAddress = GetModuleBaseAddress(L"ac_client.exe", processID);

	// base ammo address (should do dynamically)
	DWORD ammoAddress = 0x0017E0A8;

	// pointer offsets
	std::vector<DWORD> ammoOffsets{ 0x140 };

	// adds offsets to base addresss
	DWORD ammoPointerAddress = GetPointerAddress(windowHandle, baseAddress, ammoAddress, ammoOffsets);

	while (true)
	{
		int ammo = 20;
		WriteProcessMemory(processHandle, (LPVOID*)(ammoPointerAddress), &ammo, 4, 0);
	}
	return EXIT_SUCCESS;
}
