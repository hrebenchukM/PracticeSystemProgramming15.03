#pragma once

#include"libs.h"


class WordSearcher
{
public:
    WordSearcher(void);
    ~WordSearcher(void);
    static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static WordSearcher* ptr;

    void Cls_OnClose(HWND hwnd);
    BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void Cls_OnTimer(HWND hwnd, UINT id);
    HWND hOK, hCANCEL, hProgControl, hEdit1, hDialog1,hPause,hResume;
    bool isPaused = false;
    bool isStopped = false;



};
