#include"WordSearcher.h"




int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{

    WordSearcher dlg;
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WordSearcher::DlgProc);
}