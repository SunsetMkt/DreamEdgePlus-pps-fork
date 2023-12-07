
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <fstream>
#include "main.h"

#define EXENAME "Photoshop.exe"


int main(int argc, char* argv[])
{
    std::string psPath = GetRunPath() + EXENAME; //ps主程序运行目录
    
    if (!FileExists(psPath)) {
        std::cerr << "没有检测到PS，请把程序放在PS根目录。每次使用此程序来启动PS。" << std::endl;
        std::cout << "可以拖放图片到本程序，本程序可以传递给PS主程序" << std::endl;
        system("pause");
        return 0;
    }


    DWORD proId = RunProcess(psPath, argc, argv);

    int num = 4; //验证窗口销毁次数，最低为3次。
    while (true) {
        HWND psWindowId = FindWindowByProcessIdAndClassName(proId, L"Photoshop");
        if (psWindowId != nullptr) {
            //查找子进程是否有效
            std::vector<DWORD> subPros = GetChildProcessIds(proId); //获取子进程的ID组
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

        if (!IsProcessRunning(proId)) { //如果进程不存在了，则跟随推出
            //看看是否已经存在运行的PS了
            proId = GetProcessIdByName(EXENAME);
            if (proId == 0) {
                return 0;
            }
            /*std::cout << "存在已经运行的程序" << proId << std::endl;
            std::cin >> proId;*/
            num+=4;
        }
    }
GOOUT:

    std::vector<DWORD> subPros = GetChildProcessIds(proId); //获取子进程的ID组

    //for (DWORD proItem : subPros) {
    //    std::string subProName = GetProcessName(proItem);
    //    std::cout << subProName << std::endl;
    //}


    //结束子进程
    //int killSubProLoop = 30; //循环中止2次
    bool killSub = false;
    int killNum = 4;
    while (true) {
        
        std::vector<DWORD> subPros = GetChildProcessIds(proId); //获取子进程的ID组

        if (subPros.size() > 10) { //多个附加进程被启动
            killSub = true;
            killNum = 4;
        }

        if (killSub) {
            killSubProcess(subPros);
            killNum--;
        }

        if (killNum < 0) {
            break;
        }

        Sleep(100);

        if (!IsProcessRunning(proId)) { //如果进程不存在了，则跟随推出
            return 0;
        }
    }
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

//根据进程名获取进程的ID
DWORD GetProcessIdByName(const std::string& processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    DWORD dwProcessId = 0;
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);
    if (Process32First(hSnapshot, &processEntry))
    {
        do
        {
            processEntry.szExeFile;
            std::wstring we(processEntry.szExeFile);
            std::string currentProcessName(we.begin(), we.end());
            if (currentProcessName == processName)
            {
                dwProcessId = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }
    CloseHandle(hSnapshot);
    return dwProcessId;
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

            std::vector<DWORD> sub = GetChildProcessIds(processEntry.th32ProcessID);
            for (DWORD subId : sub) { //递归加入孙子进程
                childProcessIds.push_back(subId);
            }
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

//根据进程DWORD，关闭进程
void TerminateProcessByPID(DWORD dwProcessId)
{
    HANDLE hProcess;
    unsigned int tryNum = 0;
    do {
        hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
        tryNum++;
    } while (hProcess == NULL && tryNum < 3);
    if (hProcess == NULL)
    {
        return;
    }
    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
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

//判断进程是否还在运行
bool IsProcessRunning(DWORD dwProcessId)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

    if (hProcess == NULL)
    {
        return false;
    }

    DWORD dwExitCode;
    GetExitCodeProcess(hProcess, &dwExitCode);
    CloseHandle(hProcess);
    return (dwExitCode == STILL_ACTIVE);
}


//判断文件是否存在
bool FileExists(const std::string& filename)
{
    std::ifstream infile(filename);
    return infile.good();
}


//结束子进程
void killSubProcess(std::vector<DWORD> subPros)
{
    for (DWORD proItem : subPros) {
        std::string subProName = GetProcessName(proItem);
        if (subProName == "adobe_licensing_wf_helper.exe" ||
            subProName == "Adobe Crash Processor.exe" ||
            subProName == "msedgewebview2.exe" ||
            subProName == "CCXProcess.exe" ||
            subProName == "node.exe"
            ) {
            TerminateProcessByPID(proItem);
        }
    }
}
