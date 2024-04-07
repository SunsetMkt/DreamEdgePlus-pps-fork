#pragma once
#include "string"
#include "vector"
#include <windows.h>

std::string GetRunPath(); // ��ȡ��������Ŀ¼

DWORD RunProcess(std::string proPath, int argc, char *argv[]); // ����ָ�����򣬷��ؾ��

DWORD GetProcessIdByName(const std::string &processName); // ���ݽ�������ȡ���̵�ID

std::vector<DWORD> GetChildProcessIds(DWORD parentProcessId); // ���ݽ���ID��ȡ�ӽ���ID�б�

HWND FindWindowByProcessIdAndClassName(DWORD processId, const std::wstring &className); // ���ݽ���id�봰��������ȡ����id

void DisableWindow(HWND hwnd); // ������ڽ�ֹ״̬

void HideWindow(HWND hwnd); // ����ָ���Ĵ���

void TerminateProcessByPID(DWORD dwProcessId); // ���ݽ���DWORD���رս���

std::string GetWindowTitle(HWND hwnd); // ���ݴ���hwnd��ȡ���ڱ���

std::string GetProcessName(DWORD processId); // ��ȡ��������

bool IsProcessRunning(DWORD dwProcessId); // �жϽ����Ƿ�������

bool FileExists(const std::string &filename); // �ж��ļ��Ƿ����

void killSubProcess(std::vector<DWORD> subPros); // �����ӽ���