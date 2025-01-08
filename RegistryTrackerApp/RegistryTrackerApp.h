#pragma once

#include "resource.h"
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <fstream>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <ctime>
#include <shellapi.h>
#include <psapi.h>
#include <shlwapi.h>
#include <cstdio>

using namespace std;

#define MAX_LOADSTRING  100
#define IDC_LISTVIEW    101

#define OnClearedList	1
#define OnCreateFile	2
#define OnRecordAction	3
#define OnOpenFile  	4
#define OnPickExe	    5
#define OnLoadDll	    6
#define OnStartTracking	7

#define TextBufferSize	1000

struct RegistryRecord {
    DWORD processId;       // ID ��������
    wstring processPath;   // ���� � ������������ ����� ��������
    wstring dateTime;      // ���� � ����� ��������
    wstring operationType; // ��� ��������
    wstring registryKey;   // ������ ���� ����� �������
    wstring details;       // ������ ��������

    // ����������� ��� ������������� ��������
    RegistryRecord(
        DWORD pid = 0,
        wstring path = L"",
        wstring dt = L"",
        wstring opType = L"",
        wstring regKey = L"",
        wstring det = L""
    )
        : processId(pid),
        processPath(path),
        dateTime(dt),
        operationType(opType),
        registryKey(regKey),
        details(det) {
    }

    // ������� ��� �������������� ��������� � ������
    wstring ToString() const {
        wstringstream ss;
        ss << processId << L";"
            << processPath << L";"
            << dateTime << L";"
            << operationType << L";"
            << registryKey << L";"
            << details << L";";
        return ss.str();
    }
};

vector<RegistryRecord> registryRecords;         // ���������� ���������� ��� �������� ������������������ ������� ������
wchar_t delimeter = L';';                       // ����������� ��� ������
HWND hEditControl;								// ���� �����
HWND hComboBox;									// ���������� ����
char txtFilePath[MAX_PATH];						// ���������� ����������� ��� �������� ���� � ���������� TXT-����� (���-����)
char dllFilePath[MAX_PATH];						// ���������� ����������� ��� �������� ���� � ���������� DLL-�����
char exeFilePath[MAX_PATH];						// ���������� ����������� ��� �������� ���� � ���������� EXE-�����
OPENFILENAMEA ofn;								// ��������� ��� ������� �������� �����
HWND hWnd;                                      // ��������� ���� ���������
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����
HWND hwndListView;                              // ���������� ��� ListView
HWND hwndDllFilePathLabel;                      // ��������� �� ������, ������������ ���� � DLL-�����
HWND hwndExePathLabel;                          // ��������� �� ������, ������������ ���� � DLL-�����

ATOM                    MyRegisterClass(HINSTANCE hInstance);
BOOL                    InitInstance(HINSTANCE, int);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
void                    MainWndAddMenues(HWND hwnd);
void                    MainWndAddWidgets(HWND hwnd);
void                    DefineColumns(HWND hwndLV);
void                    ListViewFilling();
wstring                 OpenRegLogFile();
void                    LoadRegistryData();
vector<RegistryRecord>  ParseRegistryLogData(const wstring& logContent);
void                    UpdateFilePathLabel(HWND hwndTemp, char* filenameTemp);
void                    SetOpenFileParamsTxt();
BOOL                    SelectTxtFile(HWND hwnd);
void                    SetOpenFileParamsDll();
BOOL                    SelectDllFile(HWND hwnd);
void                    SetOpenFileParamsExe();
BOOL                    SelectExeFile(HWND hwnd);
void                    ToggleWindow(HWND hWnd, bool inProgress);
void                    InjectWithCheck(HWND hWnd);
void                    LaunchAndInject();
void                    InjectDLL(HANDLE hProcess);
