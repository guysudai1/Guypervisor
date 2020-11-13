// Guypervisor_usermode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <string>

#define DEVICE_NAME L"\\\\.\\Guypervisor"
#define VMX_ENTER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)


int main()
{
	auto hDevice = CreateFileW(DEVICE_NAME, 
							   NULL,
							   0, 
							   NULL,
							   OPEN_EXISTING,
							   FILE_FLAG_NO_BUFFERING, 
							   NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		std::cout << "CreateFileW failed with error: " << std::to_string(GetLastError()) << std::endl;
		exit(-1);
	}

	BOOL ioctl = DeviceIoControl(hDevice,
								VMX_ENTER,
								NULL,
								0,
								NULL,
								0,
								NULL, 
								NULL);

	if (!ioctl) {

		std::cout << "DeviceIoControl failed with error: " << std::to_string(GetLastError()) << std::endl;
		exit(-1);
	}

	std::cout << "Press any key to exit...";
	getchar();
}
