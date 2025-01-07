#include "framework.h"
#include "RegistryTrackerApp.h"
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

// Инициаоищация главной функции программы
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Устанавливаем имя для приложения
    wcscpy_s(szTitle, L"Наблюдатель за реестром");
    wcscpy_s(szWindowClass, L"Наблюдатель за реестром");

    MyRegisterClass(hInstance);

    // Инициализация общих элементов управления с помощью библиотеки Common Controls
    InitCommonControls();

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGISTRYTRACKERAPP));
    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

// Определение класса программы
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REGISTRYTRACKERAPP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

// Инициализация окна программы
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // Сохраняем маркер экземпляра в глобальной переменной
    hInst = hInstance;

    hWnd = CreateWindowW(

        // имя класса
        szWindowClass,

        // заголовок окна
        szTitle,

        // стиль окна
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,

        // Центрирование по горизонтали
        (GetSystemMetrics(SM_CXSCREEN) - 1200) / 2,

        // Центрирование по вертикали
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2,

        // ширина окна
        1215,

        // высота окна
        500,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Создание списка ListView
    hwndListView = CreateWindowW(
        WC_LISTVIEW,
        L"",
        WS_BORDER | WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_VSCROLL | WS_HSCROLL,
        0,
        70,
        1200,
        371,
        hWnd,
        (HMENU)IDC_LISTVIEW,
        hInstance,
        nullptr
    );

    // Добавление колонок в список ListView
    DefineColumns(hwndListView);

    return TRUE;
}

// Процедура окна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case OnClearedField:
            SetWindowTextA(hEditControl, "");
            break;
        case OnClearedList:
            registryRecords.clear();
            ListView_DeleteAllItems(hwndListView);
            break;
        case OnPickExe:
            SelectExeFile(hWnd);
            UpdateFilePathLabel(hwndExePathLabel, exeFilePath);
            break;
        case OnLoadDll:
            SelectDllFile(hWnd);
            UpdateFilePathLabel(hwndDllFilePathLabel, dllFilePath);
            break;
        case OnStartTracking:
            InjectWithCheck(hWnd);
            LoadRegistryData();
            ListViewFilling();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
        MainWndAddMenues(hWnd);
        MainWndAddWidgets(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Добавление пунктов меню
void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    AppendMenu(RootMenu, MF_STRING, OnPickExe, L"Выбрать исполняемый файл");
    AppendMenu(RootMenu, MF_STRING, OnLoadDll, L"Загрузить библиотеку");
    AppendMenu(RootMenu, MF_STRING, OnClearedList, L"Очистить список");
    AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Выход");
    SetMenu(hwnd, RootMenu);
}

// Добавление виджетов в рабочую область приложения
void MainWndAddWidgets(HWND hwnd) {

    // Кнопка для запуска отслеживания действий с регистром
    CreateWindowA("button", "Начать отслеживание", WS_VISIBLE | WS_CHILD | ES_CENTER, 25, 15, 250, 40, hwnd, (HMENU)OnStartTracking, NULL, NULL);

    // Метки для отображения пути к exe-файлу
    hwndExePathLabel = CreateWindowA("static", "Путь к EXE-файлу не задан", WS_VISIBLE | WS_CHILD | SS_LEFT, 325, 10, 800, 20, hwnd, NULL, NULL, NULL);

    // Метки для отображения пути к dll-файлу
    hwndDllFilePathLabel = CreateWindowA("static", "Путь к DLL-файлу не задан", WS_VISIBLE | WS_CHILD | SS_LEFT, 325, 40, 800, 20, hwnd, NULL, NULL, NULL);
}

// Функция для обновления виджета с путем к файлу
void UpdateFilePathLabel(HWND hwndTemp, char* filenameTemp) {
    if (hwndTemp) {

        // Формируем текст для отображения
        char labelText[512];
        snprintf(labelText, sizeof(labelText), "Файл: %s", filenameTemp[0] ? filenameTemp : "Не выбран");

        // Обновляем текст в виджете
        SetWindowTextA(hwndTemp, labelText);
    }
}

// Коллбэк-функция для установки начальной папки
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {

        // Устанавливаем начальную папку
        SendMessage(hwnd, BFFM_SETSELECTIONA, TRUE, lpData);
    }
    return 0;
}

// Определение столбцов таблицы
void DefineColumns(HWND hwndLV)
{
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // Описание колонок: текст и ширина
    struct ColumnInfo {
        int width;
        const WCHAR* name;
    };

    ColumnInfo columns[] = {
        { 45,  L"PID" },
        { 300, L"Путь к процессу" },
        { 150, L"Дата и время действия" },
        { 150, L"Тип операции" },
        { 350, L"Ключ реестра" },
        { 200, L"Детали" }
    };

    int columnCount = sizeof(columns) / sizeof(columns[0]);

    // Добавление колонок в таблицу
    for (int i = 0; i < columnCount; ++i)
    {
        lvColumn.pszText = const_cast<LPWSTR>(columns[i].name);
        lvColumn.cx = columns[i].width;
        ListView_InsertColumn(hwndLV, i, &lvColumn);
    }
}

// Заполнение ListView в прямом порядке
void ListViewFilling() {

    // Удаляем все элементы перед добавлением новых
    ListView_DeleteAllItems(hwndListView);

    // Получаем количество записей
    int recordCount = static_cast<int>(registryRecords.size());

    // Показываем количество записей в MessageBox
    wchar_t message[256];
    swprintf_s(message, sizeof(message) / sizeof(wchar_t), L"Количество записей: %d", recordCount);
    MessageBox(hWnd, message, L"Информация", MB_OK);

    // Заполняем таблицу данными в прямом порядке
    for (size_t i = 0; i < registryRecords.size(); ++i) {
        const auto& record = registryRecords[i];

        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;

        // Индекс для добавления в конец списка
        lvItem.iItem = static_cast<int>(i);

        // Индекс первого столбца (ID процесса)
        lvItem.iSubItem = 0;

        // Заполнение первого столбца (ID процесса)
        wstring processIdText = to_wstring(record.processId);
        lvItem.pszText = const_cast<LPWSTR>(processIdText.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        // Заполнение пути к процессу (столбец 1)
        ListView_SetItemText(hwndListView, lvItem.iItem, 1, const_cast<LPWSTR>(record.processPath.c_str()));

        // Заполнение даты и времени действия (столбец 2)
        ListView_SetItemText(hwndListView, lvItem.iItem, 2, const_cast<LPWSTR>(record.dateTime.c_str()));

        // Заполнение типа операции (столбец 3)
        ListView_SetItemText(hwndListView, lvItem.iItem, 3, const_cast<LPWSTR>(record.operationType.c_str()));

        // Заполнение ключа реестра (столбец 4)
        ListView_SetItemText(hwndListView, lvItem.iItem, 4, const_cast<LPWSTR>(record.registryKey.c_str()));

        // Заполнение типа данных (столбец 5)
        ListView_SetItemText(hwndListView, lvItem.iItem, 5, const_cast<LPWSTR>(record.details.c_str()));
    }
}

// Функция для управления окном
void ToggleWindow(HWND hWnd, bool inProgress) {
    if (inProgress) {

        // Сворачиваем окно
        ShowWindow(hWnd, SW_MINIMIZE);
    }
    else {

        // Разворачиваем окно
        ShowWindow(hWnd, SW_RESTORE);
    }
}

// Введение инъекции DLL-библиотеки в дочернюю программу с проверкой на наличие путей к файлам
void InjectWithCheck(HWND hWnd) {
    if (dllFilePath[0] == '\0' || exeFilePath[0] == '\0') {
        MessageBoxA(hWnd, "Пожалуйста, укажите путь к DLL и EXE файлам перед выполнением!", "Ошибка", MB_ICONERROR);
        return;
    }
    else {

        // Сворачивание окна программы-перехватчика
        ToggleWindow(hWnd, true);

        // Инъекция DLL-библиотеки в дочернюю программу
        LaunchAndInject();

        // Раскрытие окна программы-перехватчика
        ToggleWindow(hWnd, false);
    }
}

// Выгрузка данных из лог-файла в массив
void LoadRegistryData() {

    // Получение содержимого лог-файла
    wstring logContent = OpenRegLogFile();

    // Если содержимое пусто, показать диалоговое окно
    if (logContent.empty()) {
        int response = MessageBoxW(NULL, L"Файл пуст или недоступен. Хотите выбрать другой файл?", L"Ошибка", MB_YESNO | MB_ICONQUESTION);

        if (response == IDYES) {

            // Если выбрано "Да", вызываем функцию SelectTxtFile
            SelectTxtFile(hWnd);
            logContent = OpenRegLogFile();
        }
        else {

            // Если выбрано "Нет", выходим
            MessageBoxW(NULL, L"Операция отменена пользователем.", L"Информация", MB_ICONINFORMATION);
        }
    }

    // Заполнение массива после завершения работы с дочерней программой
    registryRecords = ParseRegistryLogData(logContent);
}

// Считывание содержимого лог-файла
wstring OpenRegLogFile() {

    // Извлекаем директорию из полного пути к дочернему EXE-файлу
    string directory = exeFilePath;
    size_t pos = directory.find_last_of("\\/");
    string logPath = (pos == string::npos) ? "shufladka_registry.log" : directory.substr(0, pos + 1) + "shufladka_registry.log";

    // Открываем файл в бинарном режиме
    ifstream file(logPath, ios::binary);
    if (!file.is_open()) {
        return L"";
    }

    // Читаем содержимое файла
    ostringstream contentStream;
    contentStream << file.rdbuf();
    string content = contentStream.str();

    // Преобразуем содержимое из ANSI в wstring
    int ansiContentSize = MultiByteToWideChar(CP_ACP, 0, content.c_str(), static_cast<int>(content.size()), NULL, 0);
    wstring ansiContent(ansiContentSize, L'\0');
    MultiByteToWideChar(CP_ACP, 0, content.c_str(), static_cast<int>(content.size()), &ansiContent[0], ansiContentSize);

    return ansiContent;
}

// Десериализация записей в массив
vector<RegistryRecord> ParseRegistryLogData(const wstring& logContent) {
    vector<RegistryRecord> registryData;
    wistringstream stream(logContent);
    wstring line;

    while (getline(stream, line)) {

        // Пропускаем пустые строки
        if (line.empty()) continue;

        wstringstream ss(line);
        wstring processIdStr, processPath, dateTime, operationType, registryKey, details;

        // Разделяем строку по символу delimeter
        getline(ss, processIdStr, delimeter);
        getline(ss, processPath, delimeter);
        getline(ss, dateTime, delimeter);
        getline(ss, operationType, delimeter);
        getline(ss, registryKey, delimeter);
        getline(ss, details, delimeter);

        // Извлекаем processId
        DWORD processId = stoul(processIdStr.substr(processIdStr.find(L":") + 1));

        // Создаём объект RegistryRecord
        RegistryRecord record = {
            processId, processPath, dateTime, operationType, registryKey, details
        };

        registryData.push_back(record);
    }

    return registryData;
}

// Инициализация структуры OPENFILENAME для открытия TXT-файла
void SetOpenFileParamsTxt() {
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = txtFilePath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(txtFilePath);
    ofn.lpstrFilter = "TXT Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

// Инициализация структуры OPENFILENAME для открытия DLL-файла
void SetOpenFileParamsDll() {
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = dllFilePath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(dllFilePath);
    ofn.lpstrFilter = "DLL Files\0*.DLL\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

// Инициализация структуры OPENFILENAME для открытия EXE-файла
void SetOpenFileParamsExe() {
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = exeFilePath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(exeFilePath);
    ofn.lpstrFilter = "EXE Files\0*.EXE\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

// Функция для выбора файла TXT и сохранения его пути в глобальную переменную
BOOL SelectTxtFile(HWND hwnd) {
    SetOpenFileParamsTxt();
    if (!GetOpenFileNameA(&ofn)) {

        // Пользователь отменил выбор файла
        MessageBoxA(hwnd, "Выбор файла TXT отменен.", "Информация", MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    return TRUE;
}

// Функция для выбора файла DLL и сохранения его пути в глобальную переменную
BOOL SelectDllFile(HWND hwnd) {
    SetOpenFileParamsDll();
    if (!GetOpenFileNameA(&ofn)) {

        // Пользователь отменил выбор файла
        MessageBoxA(hwnd, "Выбор файла DLL отменен.", "Информация", MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    return TRUE;
}

// Функция для выбора файла EXE и сохранения его пути в глобальную переменную
BOOL SelectExeFile(HWND hwnd) {
    SetOpenFileParamsExe();
    if (! GetOpenFileNameA(&ofn)) {

        // Пользователь отменил выбор файла
        MessageBoxA(hwnd, "Выбор файла exe отменен.", "Информация", MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    return TRUE;
}

// Инъекция DLL-библиотеки в дочерний процесс
void InjectDLL(HANDLE hProcess) {
    SIZE_T bytesWritten;

    // Выделяем память в адресном пространстве целевого процесса
    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, strlen(dllFilePath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteMemory == NULL) {
        MessageBoxA(NULL, "Не удалось выделить память в адресном пространстве целевого процесса.",
            "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Записываем путь к DLL в выделенную память
    if (!WriteProcessMemory(hProcess, remoteMemory, dllFilePath, strlen(dllFilePath) + 1, &bytesWritten)) {
        MessageBoxA(NULL, "Не удалось записать путь к DLL в память целевого процесса.",
            "Ошибка", MB_OK | MB_ICONERROR);
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        return;
    }

    // Получаем адрес функции LoadLibraryA из kernel32.dll
    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (!hKernel32) {
        MessageBoxA(NULL, "Не удалось получить дескриптор kernel32.dll.",
            "Ошибка", MB_OK | MB_ICONERROR);
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        return;
    }

    FARPROC loadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryA");
    if (!loadLibraryAddr) {
        MessageBoxA(NULL, "Не удалось получить адрес функции LoadLibraryA.",
            "Ошибка", MB_OK | MB_ICONERROR);
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        return;
    }

    // Создаем поток в целевом процессе для вызова LoadLibraryA
    HANDLE remoteThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)loadLibraryAddr,
        remoteMemory, 0, NULL);
    if (remoteThread == NULL) {
        MessageBoxA(NULL, "Не удалось создать поток в целевом процессе.",
            "Ошибка", MB_OK | MB_ICONERROR);
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        return;
    }

    // Ожидаем завершения выполнения потока
    WaitForSingleObject(remoteThread, INFINITE);

    // Закрываем поток и освобождаем выделенную память
    CloseHandle(remoteThread);
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
}

void LaunchAndInject() {
    STARTUPINFOA si = { sizeof(STARTUPINFOA) }; // Используем STARTUPINFOA для CreateProcessA
    PROCESS_INFORMATION pi = { 0 };

    // Попытка запустить процесс
    if (!CreateProcessA(
        exeFilePath,   // Путь к исполняемому файлу
        NULL,          // Аргументы
        NULL,          // Атрибуты безопасности процесса
        NULL,          // Атрибуты безопасности потока
        FALSE,         // Унаследовать дескрипторы
        0,             // Флаги создания
        NULL,          // Среда
        NULL,          // Текущая директория
        &si,           // Информация о запуске
        &pi            // Информация о процессе
    )) {
        MessageBoxA(NULL, "Не удалось запустить процесс.", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Инъекция DLL
    InjectDLL(pi.hProcess);

    // Возобновляем поток и ждем завершения процесса
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Закрываем дескрипторы
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
