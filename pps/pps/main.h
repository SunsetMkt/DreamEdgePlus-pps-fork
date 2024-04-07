#pragma once
#include "string"
#include "vector"
#include <windows.h>

std::string GetRunPath(); // 获取程序运行目录

DWORD RunProcess(std::string proPath, int argc, char *argv[]); // 运行指定程序，返回句柄

DWORD GetProcessIdByName(const std::string &processName); // 根据进程名获取进程的ID

std::vector<DWORD> GetChildProcessIds(DWORD parentProcessId); // 根据进程ID获取子进程ID列表

HWND FindWindowByProcessIdAndClassName(DWORD processId, const std::wstring &className); // 根据进程id与窗口类名获取窗口id

void DisableWindow(HWND hwnd); // 解除窗口禁止状态

void HideWindow(HWND hwnd); // 隐藏指定的窗口

void TerminateProcessByPID(DWORD dwProcessId); // 根据进程DWORD，关闭进程

std::string GetWindowTitle(HWND hwnd); // 根据窗口hwnd获取窗口标题

std::string GetProcessName(DWORD processId); // 获取进程名字

bool IsProcessRunning(DWORD dwProcessId); // 判断进程是否还在运行

bool FileExists(const std::string &filename); // 判断文件是否存在

void killSubProcess(std::vector<DWORD> subPros); // 结束子进程