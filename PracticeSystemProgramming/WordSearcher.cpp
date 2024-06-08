
#include "WordSearcher.h"


WordSearcher* WordSearcher::ptr = nullptr;
wstring badWord;
bool wordTF;//флаг найденно словло

//Критическая секция для синхронизации потоков(Критическая секция-єто небольшой участок кода требующий монопольного доступа к каким-то общим данным)  
CRITICAL_SECTION cs;

WordSearcher::WordSearcher(void)
{
    ptr = this;
    InitializeCriticalSection(&cs); 
}


WordSearcher::~WordSearcher(void)
{
    DeleteCriticalSection(&cs); 
}


void WordSearcher::Cls_OnClose(HWND hwnd)
{
    EndDialog(hwnd, 0);
}





BOOL WordSearcher::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    //получение дескрипторов элементов управления
    hEdit1 = GetDlgItem(hwnd, IDC_EDIT1);
    hOK = GetDlgItem(hwnd, IDOK);
    hCANCEL = GetDlgItem(hwnd, IDCANCEL);
    hProgControl = GetDlgItem(hwnd, IDC_PROGRESS1);
    hDialog1 = GetDlgItem(hwnd, IDD_DIALOG1);
    hPause = GetDlgItem(hwnd, IDC_PAUSE);
    hResume = GetDlgItem(hwnd, IDC_RESUME);

    SendMessage(hProgControl, PBM_SETRANGE, 0, MAKELPARAM(0, 60)); // Установка интервала для индикатора прогресса
    SendMessage(hProgControl, PBM_SETSTEP, 1, 0); // Установка шага приращения  индикатора 
    SendMessage(hProgControl, PBM_SETPOS, 0, 0);

    EnableWindow(hResume, FALSE);


    return TRUE;
}




void removeFileRes() {
    ofstream resFile("res.txt", ios::out | ios::trunc);// запись + очистка 
    if (resFile.is_open()) {
        resFile.close();
       
    }
    else {
        wstring errorMsg = L"Ошибка при удалении файла!!! 'res.txt'!";
        MessageBox(NULL, errorMsg.c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
    }
}




void viewResSearch(HWND hwnd, bool foundW, const wstring& w) {
    wstring resMsg;
    if (foundW) {
        resMsg = L"Слово '" + w + L"' найдено в файле!";
    }
    else {
        resMsg = L"Слово '" + w + L"' НЕ найдено в файле!";
    }
    MessageBox(hwnd, resMsg.c_str(), L"Результат", MB_OK | MB_ICONINFORMATION);
}




wstring modifyStr(const wstring& l, const wstring& w, int& replN) {
    wstring res;
    int p = 0;
    int foundP;
    while ((foundP = l.find(w, p)) != -1) {
        res += l.substr(p, foundP - p); //l = "by William Shakespeare";substr(0, 12 - 0) = "by William"
        res.append(w.length(), L'*'); // res = L"by William ";  w = L"Shakespeare";res.append(w.length(), L'*'); = by William ***********

        p = foundP + w.length();//23 
        replN++;//1
       
    }
  
    res += l.substr(p);
    return res;
}

void transformFile(const wstring& filePath, const wstring& w, wofstream& resStream, int& replN, bool& foundW) {
    replN = 0;
    ifstream file(filePath);
    if (file.is_open()) {
        string l;
        bool repl_inFile = false; 
        while (getline(file, l)) {
            wstring wl(l.begin(), l.end());
            wstring modifiedL = modifyStr(wl, w, replN);
            if (modifiedL != wl) {
                repl_inFile = true; 


            }

            if (repl_inFile) {
                resStream << modifiedL << endl;
            }
        }
        file.close();
        if (repl_inFile) { 
            foundW = true;
            resStream << "----------------------" << endl;
            resStream << "File name: " << filePath << endl;
            resStream << "Count of bad words: " << replN << endl;
            resStream << "----------------------" << endl;
        }
    }
    else {
        wstring errorMsg = L"Файл '" + filePath + L"'не найден!";
        MessageBox(NULL, errorMsg.c_str(), L"Ошибка!", MB_OK | MB_ICONINFORMATION);
    }
}



void scanFilesForW(const wstring& folderF, const wstring& w) {
    bool foundW = false;
    int allRepl = 0;

    wofstream resFile("res.txt", ios::out | ios::trunc);
    if (!resFile.is_open()) {
        wstring errorMsg = L"Ошибка при открытии файла 'res.txt'!";
        MessageBox(NULL, errorMsg.c_str(), L"Ошибка!", MB_OK | MB_ICONERROR);
        return;
    }

    resFile << "------------------------------------" << endl;
    resFile << "Results for bad word: " << w << endl;
    resFile << "------------------------------------" << endl;
    // поиск всех файлов .txt в папке
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((folderF + L"\\*").c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        resFile.close();
        return;
    }

    do {
     
        wstring filePath = folderF + L"\\" + findFileData.cFileName;
        if (wcsstr(findFileData.cFileName, L".txt") && wcscmp(findFileData.cFileName, L"res.txt") != 0) {
            transformFile(filePath, w, resFile, allRepl, foundW);
          }

    } while (FindNextFile(hFind, &findFileData) != 0);

    wordTF = foundW;
    resFile.close();
}



////   считывает текст из текстового поля и сканирует файлы с ним
DWORD WINAPI Thread(LPVOID lp) {
    HWND hEdit1 = GetDlgItem((HWND)lp, IDC_EDIT1);
    int editLength = GetWindowTextLength(hEdit1);
    wchar_t* buf = new wchar_t[editLength + 1];
  

    GetWindowText(hEdit1, buf, editLength + 1);// получаем текст из єдита сохраняем в буфер 
  
    EnterCriticalSection(&cs);
    badWord = buf;
    LeaveCriticalSection(&cs);
    delete[] buf;
    scanFilesForW(L".", badWord);

    return 0;
}


//  запуск потока
void  InitiateThread(HWND hwnd) {
    HANDLE hThread = CreateThread(NULL, 0, Thread, hwnd, 0, NULL);
    if (hThread == NULL) {
        MessageBox(NULL, L"Ошибка при создании потока! ", L"Error", MB_OK | MB_ICONERROR);
    }
    else {
        CloseHandle(hThread);
    }
}





void WordSearcher::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDOK) {
        InitiateThread(hwnd);
        SetTimer(hwnd, 1, 30, NULL);
        
    }
    if (id == IDCANCEL) {

        removeFileRes();
        KillTimer(hwnd, 1);
        SendMessage(hProgControl, PBM_SETPOS, 0, 0);
      
    }
    if (id == IDC_PAUSE) {
        EnterCriticalSection(&cs);
        isPaused = true;
        LeaveCriticalSection(&cs); 
        // щтключение кнопочек
        EnableWindow(hPause, TRUE);
        EnableWindow(hResume, FALSE);
    }
    if (id == IDC_RESUME) {
        EnterCriticalSection(&cs); 
        isPaused = false;
        LeaveCriticalSection(&cs);
        // щтключение кнопочек
        EnableWindow(hPause, FALSE);
        EnableWindow(hResume, TRUE);
    }

}


void WordSearcher::Cls_OnTimer(HWND hwnd, UINT id)
{
    EnterCriticalSection(&cs);

    if (!isPaused) {
    
         if(SendMessage(hProgControl, PBM_GETPOS, 0, 0) >= 60) 
            {
            KillTimer(hwnd, 1);
            SendMessage(hProgControl, PBM_SETPOS, 0, 0);//сброс

            if (badWord.size() > 0 ) {
                viewResSearch(hwnd, wordTF, badWord);
            }
         }
         else {
             SendMessage(hProgControl, PBM_STEPIT, 0, 0);//увеличение
         }
    }
    LeaveCriticalSection(&cs);
}





INT_PTR CALLBACK WordSearcher::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
        HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
        HANDLE_MSG(hwnd, WM_TIMER, ptr->Cls_OnTimer);
    }


    return FALSE;
}

