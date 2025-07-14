/*
    FileToByteArray - Конвертер любого файла в массив байт на C++
    Copyright (C) 2024 Zenoyui

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

HWND hwndSelectButton, hwndConvertButton, hwndCopyButton, hwndOutputEdit;
std::wstring selectedFilePath;
std::string byteOutput;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hwndSelectButton = CreateWindow(
            L"BUTTON", L"Select File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 120, 30, hwnd, (HMENU)1001, NULL, NULL);

        hwndConvertButton = CreateWindow(
            L"BUTTON", L"Convert", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            140, 10, 100, 30, hwnd, (HMENU)1002, NULL, NULL);

        hwndOutputEdit = CreateWindow(
            L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 50, 460, 200, hwnd, NULL, NULL, NULL);

        hwndCopyButton = CreateWindow(
            L"BUTTON", L"Copy to Clipboard", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            250, 10, 120, 30, hwnd, (HMENU)1003, NULL, NULL);

        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1001: {
            OPENFILENAMEW ofn = { 0 };
            wchar_t szFile[260] = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            // Показываем по умолчанию все файлы
            ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                selectedFilePath = ofn.lpstrFile;
                SetWindowTextW(hwndOutputEdit, L"File selected. Click Convert to process.");
            }
            break;
        }

        case 1002: {
            if (selectedFilePath.empty()) {
                MessageBoxW(hwnd, L"Please select a file first!", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }

            std::ifstream file(selectedFilePath, std::ios::binary);
            if (!file) {
                MessageBoxW(hwnd, L"Failed to open the file!", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }

            std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            std::stringstream ss;
            ss << "unsigned char fileBytes[] = {";
            for (size_t i = 0; i < buffer.size(); ++i) {
                if (i % 16 == 0) ss << "\n    ";
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
                if (i < buffer.size() - 1) ss << ", ";
            }
            ss << "\n};";
            byteOutput = ss.str();

            std::wstring wOutput(byteOutput.begin(), byteOutput.end());
            SetWindowTextW(hwndOutputEdit, wOutput.c_str());
            break;
        }

        case 1003: {
            if (byteOutput.empty()) {
                MessageBoxW(hwnd, L"No data to copy!", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }

            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                size_t size = byteOutput.size() + 1;
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                if (hMem) {
                    char* pMem = (char*)GlobalLock(hMem);
                    memcpy(pMem, byteOutput.c_str(), size);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_TEXT, hMem);
                }
                CloseClipboard();
            }
            MessageBoxW(hwnd, L"Data copied to clipboard!", L"Success", MB_OK | MB_ICONINFORMATION);
            break;
        }
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FileToByteArrayClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(
        L"FileToByteArrayClass",
        L"File to Byte Array Converter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}
