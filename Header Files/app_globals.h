// app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <QObject> // Ϊ���ڸ���������ʹ�� tr()

// ��������ö��
enum class AppLanguage {
    English,
    Chinese
};

extern AppLanguage currentLanguage;

void loadLanguage(AppLanguage lang);

#endif // APP_GLOBALS_H