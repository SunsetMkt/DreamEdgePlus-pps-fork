
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <fstream>
#include "main.h"

int main(int argc, char* argv[])
{
    std::string psPath = GetRunPath() + "Photoshop.exe"; //ps����������Ŀ¼
    
    if (!FileExists(psPath)) {
        std::cerr << "û�м�⵽PS����ѳ������PS��Ŀ¼��ÿ��ʹ�ô˳���������PS��" << std::endl;
        std::cout << "�����Ϸ�ͼƬ�������򣬱�������Դ��ݸ�PS������" << std::endl;
        system("pause");
        return 0;
    }

    DWORD proId = RunProcess(psPath, argc, argv);

    int num = 4; //��֤�������ٴ��������Ϊ3�Ρ�
    int sleepNum = 0; //����ʱ�䣬��ֹ�������

    while (true) {
        HWND psWindowId = FindWindowByProcessIdAndClassName(proId, L"Photoshop");
        if (psWindowId != nullptr) {
            //�����ӽ����Ƿ���Ч
            std::vector<DWORD> subPros = GetChildProcessIds(proId);
            for (DWORD proItem : subPros) {
                if (GetProcessName(proItem) == "adobe_licensing_wf.exe") {
                    HWND wfWindowId = FindWindowByProcessIdAndClassName(proItem, L"EmbeddedWB");
                    if (wfWindowId != nullptr) { //�ҵ�����Ŀ

                        if (num <= 0) {
                            goto GOOUT; //��������
                        }
                        num--;

                        HideWindow(wfWindowId); //���ع��ڴ���
                        DisableWindow(psWindowId); //���PS���ڽ�ֹ״̬
                    }
                }
            }
        }
        Sleep(100); //�ȴ�PS��������

        if (sleepNum >= 1200) { //120���������û�����������������
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


//��ȡ��������Ŀ¼
std::string GetRunPath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH); // ʹ��GetModuleFileNameW()��ȡ��ǰ��ִ���ļ�������·��
    std::wstring fullPath(buffer);
    std::wstring::size_type pos = fullPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        // ��ȡ��·���е�Ŀ¼����
        std::wstring directory = fullPath.substr(0, pos + 1);
        return std::string(directory.begin(), directory.end());
    }
    return "";
}

//����ָ�����򣬷��ؾ��
DWORD RunProcess(std::string proPath, int argc, char* argv[])
{
    // �������̵Ľṹ��
    STARTUPINFOA startupInfo = { 0 };
    startupInfo.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION processInfo = { 0 };

    // ���������в����ַ���
    std::string commandLine;
    for (int i = 0; i < argc; ++i)
    {
        commandLine += argv[i];
        commandLine += " ";
    }


    // ��������
    if (!CreateProcessA(const_cast<LPSTR>(proPath.c_str()), const_cast<LPSTR>(commandLine.c_str()), nullptr,
        nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo))
    {
        return 0;
    }

    // ��ȡ���� ID
    DWORD processId = GetProcessId(processInfo.hProcess);

    // �ر����õľ��
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return processId;
}

//���ݽ���ID��ȡ�ӽ���id��
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


//���ݽ���id�봰��������ȡ����id
HWND FindWindowByProcessIdAndClassName(DWORD processId, const std::wstring& className)
{
    HWND windowHandle = NULL;

    // ö�ٽ��������еĶ�������
    do
    {
        windowHandle = FindWindowEx(NULL, windowHandle, className.c_str(), NULL);

        // ����ҵ����ڣ��������Ľ��� ID �Ƿ���ָ���Ľ��� ID ƥ��
        if (windowHandle != NULL)
        {
            DWORD windowProcessId = 0;
            GetWindowThreadProcessId(windowHandle, &windowProcessId);
            if (windowProcessId == processId)
            {
                return windowHandle;  // �ҵ�ƥ��Ĵ���
            }
        }
    } while (windowHandle != NULL);

    return nullptr;  // û���ҵ�ƥ��Ĵ���
}


//������ڽ�ֹ״̬
void DisableWindow(HWND hwnd)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    SetWindowLongPtr(hwnd, GWL_STYLE, style & ~WS_DISABLED);
}

//����ָ���Ĵ���
void HideWindow(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
}

//���ݴ���hwnd��ȡ���ڱ���
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


//��ȡ������
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


//�ж��ļ��Ƿ����
bool FileExists(const std::string& filename)
{
    std::ifstream infile(filename);
    return infile.good();
}
