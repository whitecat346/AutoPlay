// Project Name: Auto Play
// Build Date: 2024/1/1 (UTC+8)
// Author: WhiteCAT
// Repository URL: https://github.com/whitecat346/AutoPlay

// Head File

// System
#include <chrono>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <shobjidl_core.h>
#include <locale>
#include <codecvt>
#include <io.h>
#include <strsafe.h>
#include <thread>

// User's
#include "json.hpp"

// Function Define
inline void GetSystemTime(tm& tmd);
inline std::wstring string2wstring(const std::string& mbs);
inline std::string wstring2string(const std::wstring& wstr);
void ErrorExit(LPTSTR lpszFunction);

// Config Struce
struct cfg_time
{
	int out_year;
	int out_month;
	int out_day;
	int out_hour;
	int out_minute;
	int out_second;

	int in_hour;
	int in_minute;
	int in_second;
};

// Software Access
int main(int argc, char** argv)
{
	// Work Tree
	// Load config.json -> Get Time -> If at time: Run ffplay ( while )

	// Load Config
	std::ifstream fileCfg("config.json");
	if (fileCfg.is_open() == false)
	{
		MessageBox(NULL,
			L"It's look like we meet some problems.\nReason: config.json was not opened!", L"Error!",
			MB_OK | MB_ICONSTOP | MB_APPLMODAL
		);
		std::cout << "It's look like we meet some problems.\nReason: config.json was not opened!";
		return 3;
	}

	nlohmann::json cfg = nlohmann::json::parse(fileCfg);

	// Enable
	if (cfg.at("autoplay") == false)
	{
		MessageBox(NULL,
			L"AutoPlay is not enable.", L"Info",
			MB_OK | MB_ICONINFORMATION | MB_APPLMODAL
		);
		std::cout << "AutoPlay is not enable.";
		return 1;
	}

	std::cout << "AutoPlay Release 0.2.4 \nRepository URL: https://github.com/whitecat346/AutoPlay" << std::endl;

	// Hide Command Window
	{
		HWND hWnd = GetForegroundWindow();
		ShowWindow(hWnd, SW_HIDE);
		HRESULT hr;
		ITaskbarList* p_t_taskbar_list;
		hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&p_t_taskbar_list);

		std::this_thread::sleep_for(std::chrono::milliseconds(400));
		if (SUCCEEDED(hr))
		{
			p_t_taskbar_list->HrInit();
			p_t_taskbar_list->DeleteTab(hWnd);
		}
	}

	// Out Of Date
	cfg_time ct = {
		cfg.at("out-date").at("year"),
		cfg.at("out-date").at("month"),
		cfg.at("out-date").at("day"),
		cfg.at("out-date").at("hour"),
		cfg.at("out-date").at("minute"),
		cfg.at("out-date").at("second"),
		cfg.at("time").at("hour"),
		cfg.at("time").at("minute"),
		cfg.at("time").at("second")
	};
	tm tnnd;

	GetSystemTime(tnnd);
	if ((tnnd.tm_year + 1900) >= ct.out_year
		&& (tnnd.tm_mon + 1) >= ct.out_month
		&& tnnd.tm_mday >= ct.out_month
		&& tnnd.tm_hour >= ct.out_hour
		&& tnnd.tm_min >= ct.out_minute
		&& tnnd.tm_sec >= ct.out_second)
	{
		MessageBox(NULL,
			L"Software was out of date!", L"Error",
			MB_OK | MB_ICONERROR | MB_APPLMODAL
		);
		std::cout << "Software was out of date!";
		return 2;
	}

	// Get Time
	while (!GetSystemTime(tnnd))
	{
		if (tnnd.tm_hour == ct.in_hour
			&& tnnd.tm_min == ct.in_minute
			&& tnnd.tm_sec == ct.in_second)
		{
			if (std::ifstream((std::string(cfg.at("file-path"))).c_str()).is_open())
			{
				std::string vpath;
				std::wstring fpath(string2wstring(cfg.at("ffplay-path")));
				vpath.append(" \"").append(cfg.at("file-path")).append("\" ").append(cfg.at("command"));

				STARTUPINFO si{ 0 };
				PROCESS_INFORMATION pi;

				if(CreateProcess(fpath.c_str(),
					(LPWSTR)((string2wstring(vpath)).c_str()),
					0, 0, 0, 0, 0, 0,
					&si, &pi
				))
				{
					while (true)
					{
						HWND hffplay = FindWindow(L"TXGuiFoundation", NULL);
						if(!hffplay)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
							SendMessageA(hffplay, WM_CLOSE, 0, 0);
						}
						return 0;
					}
				}
				else
				{
					ErrorExit((LPTSTR)L"CreateProcess");
					return 2;
				}
			}
			else
			{
				MessageBox(NULL,
					L"Can not load video file!", L"Error",
					MB_OK | MB_ICONERROR | MB_APPLMODAL
				);
				std::cout << "Can not load video file!";
				return 3;
			}
			return 0;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}

inline void GetSystemTime(tm& tmd)
{
	time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	localtime_s(&tmd, &tt);
}

inline std::string wstring2string(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

inline std::wstring string2wstring(const std::string& mbs) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(mbs);
}

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}
