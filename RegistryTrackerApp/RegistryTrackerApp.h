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
    DWORD processId;       // ID процесса
    wstring processPath;   // Путь к исполняемому файлу процесса
    wstring dateTime;      // Дата и время операции
    wstring operationType; // Тип операции
    wstring registryKey;   // Полный путь ключа реестра
    wstring details;       // Детали операции

    // Конструктор для инициализации значений
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

    // Функция для преобразования структуры в строку
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

vector<RegistryRecord> registryRecords;         // Глобальная переменная для хранения десериализованного массива данных
wchar_t delimeter = L';';                       // Разделитель для записи
HWND hEditControl;								// Поле ввода
HWND hComboBox;									// Выпадающее меню
char txtFilePath[MAX_PATH];						// Глобальная пепременная для хранения пути к выбранному TXT-файлу (лог-файл)
char dllFilePath[MAX_PATH];						// Глобальная пепременная для хранения пути к выбранному DLL-файлу
char exeFilePath[MAX_PATH];						// Глобальная пепременная для хранения пути к выбранному EXE-файлу
OPENFILENAMEA ofn;								// Структура для диалога открытия файла
HWND hWnd;                                      // Экземпляр окна программы
HINSTANCE hInst;                                // Текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // Имя класса главного окна
HWND hwndListView;                              // Переменная для ListView
HWND hwndDllFilePathLabel;                      // Указатель на виджет, отображающий путь к DLL-файлу
HWND hwndExePathLabel;                          // Указатель на виджет, отображающий путь к DLL-файлу

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
