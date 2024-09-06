#ifndef __HELPER_FUNCTIONS_H__
#define __HELPER_FUNCTIONS_H__

#include <cmath>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

inline std::string getParentPath(const std::string &path)
{
    std::filesystem::path fsPath(path);
    if (fsPath.has_parent_path())
    {
        return fsPath.parent_path().string();
    }
    else
    {
        return "";
    }
}

inline std::string getParentPath(size_t index, const std::string &path)
{
    if (index < 1)
    {
        return {};
    }

    std::string ret = path;

    for (int i = 0; i < index; ++i)
    {
        ret = getParentPath(ret);
    }

    return ret;
}

inline int get_char()
{
    static std::mutex mtx; // 声明一个静态互斥锁，用于保护标准输入的读取
#ifdef IS_WINDOWS_OS
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); // 获取标准输入的句柄
    DWORD  prev_mode;
    GetConsoleMode(hStdin, &prev_mode); // 获取当前控制台模式
    SetConsoleMode(hStdin, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_LINE_INPUT); // 设置新模式

    char  ch = 0;
    DWORD bytesRead;
    if (ReadFile(hStdin, &ch, 1, &bytesRead, NULL) && bytesRead > 0)
    {
        // 成功读取字符
        return ch;
    }

    // 恢复原始控制台模式
    SetConsoleMode(hStdin, prev_mode);
    return -1; // 表示没有读取到字符
#else
    fd_set         rfds;
    struct timeval tv;
    int            ch = 0;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 10; // 设置等待超时时间

    // 检测键盘是否有输入
    if (select(1, &rfds, NULL, NULL, &tv) > 0)
    {
        std::lock_guard< std::mutex > lock(mtx); // 锁定互斥锁
        ch = getchar();
    }
    return ch;
#endif
}

#endif