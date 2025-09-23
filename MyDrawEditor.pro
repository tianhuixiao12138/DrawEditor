# diagrameditor.pro - Qt 项目配置文件
QT += core gui widgets  
TARGET = DiagramEditor
TEMPLATE = app

# 设置编码格式（推荐）
CODECFORTR = UTF-8

# 主程序入口
SOURCES += main.cpp

# 源文件列表
SOURCES += \
    additemcommand.cpp \
    alignshapescommand.cpp \
    changebordercolorcommand.cpp \
    changeborderstylecommand.cpp\
    changeborderwidthcommand.cpp \
    changefillcolorcommand.cpp\
    changefontcommand.cpp \
    changelinecolorcommand.cpp \
    changelinestylecommand.cpp \
    changelinewidthcommand.cpp\
    changetextcolorcommand.cpp \
    changetextcommand.cpp \
    connectionline.cpp \
    diagrameditor.cpp \
    diagrameditormeau.cpp\
    formatpanel.cpp \
    main.cpp\
    movecommand.cpp \
    pastecommand.cpp \
    removeitemscommand.cpp \
    resizeshapecommand.cpp \
    # 添加其他 .cpp 文件...

# 头文件列表
HEADERS += \
    additemcommand.h \
    alignshapescommand.h \
    changebordercolorcommand.h \
    changeborderstylecommand.h\
    changeborderwidthcommand.h \
    changefillcolorcommand.h\
    changefontcommand.h \
    changelinecolorcommand.h \
    changelinestylecommand.h \
    changelinewidthcommand.h\
    changetextcolorcommand.h \
    changeTextCommand.h \
    connectionline.h \
    diagrameditor.h \
    diagrameditormeau.h\
    diagrameditortool.h\
    formatpanel.h \
    MoveCommand.h \
    pastecommand.h \
    removeitemscommand.h \
    resizeshapecommand.h \
    shape.h \
    app_globals.h\
    # 添加其他 .h 文件...

# 国际化支持
TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/en_US.ts \
    translations/ja_JP.ts \
    translations/es_ES.ts

# RESOURCES += resources.qrc
# 构建选项
DEFINES += QT_DEPRECATED_WARNINGS QT_NO_KEYWORDS

# 如果你使用了 UI 文件（.ui），请取消注释下面这行
# FORMS += \
#         mainwindow.ui \

# 如果你需要发布时嵌入图标等资源
# RC_FILE = diagrameditor.rc