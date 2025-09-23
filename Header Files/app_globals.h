// app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <QObject> // 为了在辅助函数中使用 tr()

// 定义语言枚举
enum class AppLanguage {
    English,
    Chinese
};

extern AppLanguage currentLanguage;

void loadLanguage(AppLanguage lang);

#endif // APP_GLOBALS_H