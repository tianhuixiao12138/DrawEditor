#include <QCoreApplication>
#include <QApplication> 
#include <QWidget>
#include <QVBoxLayout>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QLibraryInfo> 
#include "diagrameditor.h"
#include <QFileInfo>
#include "app_globals.h" // <--- 包含新的共享头文件

// --- 用于翻译的全局对象 ---
QTranslator appTranslator;
QTranslator qtTranslator;

// --- 全局变量的定义 ---
AppLanguage currentLanguage = AppLanguage::Chinese; // <--- 定义 currentLanguage
QApplication* g_app = nullptr;

// --- loadLanguage 函数的定义 (保持不变，因为它需要 g_app) ---
void loadLanguage(AppLanguage lang) { // 函数签名与 app_globals.h 中的声明匹配
    if (!g_app) return;

    g_app->removeTranslator(&appTranslator);
    g_app->removeTranslator(&qtTranslator);

    ::currentLanguage = lang; // 使用 :: 作用域解析符强调是全局的 (可选)
    if (lang == AppLanguage::Chinese) {
        QString qtTranslationsPath = QApplication::applicationDirPath() + "/translations/qtbase_zh_CN.qm"; 
       
        bool qtLoaded = qtTranslator.load(qtTranslationsPath);
        if (!qtLoaded) {
            qtLoaded = qtTranslator.load("zh_CN.qm", QApplication::applicationDirPath() + "/translations");
        }
        if (qtLoaded) {
            g_app->installTranslator(&qtTranslator);
        }

        QString appTranslationsPath = QApplication::applicationDirPath() + "/translations/zh_CN.qm";
        if (appTranslator.load(appTranslationsPath)) {
            g_app->installTranslator(&appTranslator);
        }
    }
}


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    g_app = &app;

    // --- 初始语言加载 ---
    loadLanguage(currentLanguage); // 使用上面定义的 currentLanguage
    // --- 创建窗口 ---
    QWidget mainWindow;

    mainWindow.setWindowTitle(QObject::tr("Qt Diagramming Software"));
    mainWindow.resize(1500, 1200);

    DiagramEditor* editor = new DiagramEditor();
    QVBoxLayout* layout = new QVBoxLayout(&mainWindow);
    layout->addWidget(editor);
    layout->setContentsMargins(0, 0, 0, 0); // 通常主布局会设置边距，设为0可以让内容占满窗口
    mainWindow.setLayout(layout);

    // --- **修改:** 调用 showFullScreen() 代替 show() ---
    mainWindow.showMaximized(); // 将窗口显示为最大化状态
    mainWindow.show();
    return app.exec();
}