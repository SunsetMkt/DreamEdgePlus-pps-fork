
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <fstream>
#include "main.h"

int main(int argc, char* argv[])
{
    std::string psPath = GetRunPath() + "Photoshop.exe"; //ps主程序运行目录
    
    if (!FileExists(psPath)) {
        std::cerr << "没有检测到PS，请把程序放在PS根目录。每次使用此程序来启动PS。" << std::endl;
        std::cout << "可以拖放图片到本程序，本程序可以传递给PS主程序" << std::endl;
        system("pause");
        return 0;
    }

    DWORD proId = RunProcess(psPath, argc, argv);

    int num = 4; //验证窗口销毁次数，最低为3次。
    int sleepNum = 0; //休眠时间，防止多次休眠

    while (true) {
        HWND psWindowId = FindWindowByProcessIdAndClassName(proId, L"Photoshop");
        if (psWindowId != nullptr) {
            //查找子进程是否有效
            std::vector<DWORD> subPros = GetChildProcessIds(proId);
            for (DWORD proItem : subPros) {
                if (GetProcessName(proItem) == "adobe_licensing_wf.exe") {
                    HWND wfWindowId = FindWindowByProcessIdAndClassName(proItem, L"EmbeddedWB");
                    if (wfWindowId != nullptr) { //找到子项目

                        if (num <= 0) {
                            goto GOOUT; //结束程序
                        }
                        num--;

                        HideWindow(wfWindowId); //隐藏过期窗口
                        DisableWindow(psWindowId); //解除PS窗口禁止状态
                    }
                }
            }
        }
        Sleep(100); //等待PS窗口启动

        if (sleepNum >= 1200) { //120秒过后依旧没有启动，则结束进程
            return 0;
        }
    }
    GOOUT:

    //std::cout << FindWindowByProcessIdAndClassName(proId, L"Photoshop");
    //std::cout << FindWindowByProcessIdAndClassName(15448, L"abc");
    //DisableWindow(FindWindowByProcessIdAndClassName(23312, L"Photoshop"));
    //HideWindow(FindWindowByProcessIdAndClassName(18092, L"EmbeddedWB"));
    return 0;
}


//获取程序运行目录
std::string GetRunPath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH); // 使用GetModuleFileNameW()获取当前可执行文件的完整路径
    std::wstring fullPath(buffer);
    std::wstring::size_type pos = fullPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        // 提取出路径中的目录部分
        std::wstring directory = fullPath.substr(0, pos + 1);
        return std::string(directory.begin(), directory.end());
    }
    return "";
}

//运行指定程序，返回句柄
DWORD RunProcess(std::string proPath, int argc, char* argv[])
{
    // 创建进程的结构体
    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION processInfo = { 0 };

    // 构建命令行参数字符串
    std::string commandLine;
    for (int i = 0; i < argc; ++i)
    {
        commandLine += argv[i];
        commandLine += " ";
    }


    // 创建进程
    if (!CreateProcessA(const_cast<LPSTR>(proPath.c_str()), const_cast<LPSTR>(commandLine.c_str()), nullptr,
        nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo))
    {
        return 0;
    }

    // 获取进程 ID
    DWORD processId = GetProcessId(processInfo.hProcess);

    // 关闭无用的句柄
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return processId;
}

//根据进程ID获取子进程id组
std::vector<DWORD> GetChildProcessIds(DWORD parentProcessId)
{
    std::vector<DWORD> childProcessIds;
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE)
    {
        return childProcessIds;
    }

    if (!Process32First(processSnapshot, &processEntry))
    {
        CloseHandle(processSnapshot);
        return childProcessIds;
    }

    do
    {
        if (processEntry.th32ParentProcessID == parentProcessId)
        {
            childProcessIds.push_back(processEntry.th32ProcessID);
        }
    } while (Process32Next(processSnapshot, &processEntry));

    CloseHandle(processSnapshot);
    return childProcessIds;
}


//根据进程id与窗口类名获取窗口id
HWND FindWindowByProcessIdAndClassName(DWORD processId, const std::wstring& className)
{
    HWND windowHandle = NULL;

    // 枚举进程中所有的顶级窗口
    do
    {
        windowHandle = FindWindowEx(NULL, windowHandle, className.c_str(), NULL);

        // 如果找到窗口，则检查它的进程 ID 是否与指定的进程 ID 匹配
        if (windowHandle != NULL)
        {
            DWORD windowProcessId = 0;
            GetWindowThreadProcessId(windowHandle, &windowProcessId);
            if (windowProcessId == processId)
            {
                return windowHandle;  // 找到匹配的窗口
            }
        }
    } while (windowHandle != NULL);

    return nullptr;  // 没有找到匹配的窗口
}


//解除窗口禁止状态
void DisableWindow(HWND hwnd)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    SetWindowLongPtr(hwnd, GWL_STYLE, style & ~WS_DISABLED);
}

//隐藏指定的窗口
void HideWindow(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
}

//根据窗口hwnd获取窗口标题
std::string GetWindowTitle(HWND hwnd)
{
    int titleLength = GetWindowTextLengthW(hwnd);
    if (titleLength == 0)
    {
        return "";
    }

    wchar_t* titleBuffer = new wchar_t[titleLength + 1];
    GetWindowTextW(hwnd, titleBuffer, titleLength + 1);

    std::wstring windowTitleW(titleBuffer);

    delete[] titleBuffer;

    std::string windowTitle(windowTitleW.begin(), windowTitleW.end());

    return windowTitle;
}


//获取进程名
std::string GetProcessName(DWORD processId)
{
    std::string processName;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess != NULL)
    {
        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = sizeof(buffer) / sizeof(buffer[0]);
        if (QueryFullProcessImageName(hProcess, 0, buffer, &bufferSize) != 0)
        {
            std::wstring imageName(buffer);
            size_t position = imageName.find_last_of(L"\\");
            if (position != std::wstring::npos)
            {
                processName = std::string(imageName.begin() + position + 1, imageName.end());
            }
        }
        CloseHandle(hProcess);
    }
    return processName;
}


//判断文件是否存在
bool FileExists(const std::string& filename)
{
    std::ifstream infile(filename);
    return infile.good();
}
