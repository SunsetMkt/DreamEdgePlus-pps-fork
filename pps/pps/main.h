#pragma once
#include "string"
#include "vector"
#include <windows.h>

std::string GetRunPath(); //��ȡ��������Ŀ¼

DWORD RunProcess(std::string proPath, int argc, char* argv[]); //����ָ�����򣬷��ؾ��

std::vector<DWORD> GetChildProcessIds(DWORD parentProcessId); //���ݽ���ID��ȡ�ӽ���ID�б�

HWND FindWindowByProcessIdAndClassName(DWORD processId, const std::wstring& className); //���ݽ���id�봰��������ȡ����id

void DisableWindow(HWND hwnd); //������ڽ�ֹ״̬

void HideWindow(HWND hwnd); //����ָ���Ĵ���

std::string GetWindowTitle(HWND hwnd); //���ݴ���hwnd��ȡ���ڱ���

std::string GetProcessName(DWORD processId); //��ȡ��������

bool FileExists(const std::string& filename); //�ж��ļ��Ƿ����