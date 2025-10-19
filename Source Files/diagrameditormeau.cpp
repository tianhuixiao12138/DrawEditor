#include "diagrameditormeau.h"
#include <QMessageBox>
#include <QApplication>
#include <QMenu>     
#include <QAction>   

MenuBar::MenuBar(QUndoStack* undoStack, QWidget* parent)
    : QMenuBar(parent), m_undoStack(undoStack) // <-- 初始化 m_undoStack
{
    if (!m_undoStack) {
        qWarning("MenuBar: Received null QUndoStack pointer!");
        // 可以选择抛出异常或进行其他错误处理
    }

    createFileMenu();
    createEditMenu();   // <--- 调用新的编辑菜单创建函数
    createFormatMenu();
    createAlignMenu();   // <--- 调用对齐菜单创建
    createLayerMenu();
    createSwichMenu();
    createHelpMenu();
    createLanguageMenu();
}

void MenuBar::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(); // **调用 retranslateUi 更新文本**
    }
    QMenuBar::changeEvent(event);
}


void MenuBar::retranslateUi() {
    
    this->clear(); // 清除菜单栏上的所有顶级菜单和动作

    // 重新创建所有菜单，tr() 将使用新的翻译器
    createFileMenu();
    createEditMenu();
    createFormatMenu();
    createAlignMenu();
    createLayerMenu();
    createSwichMenu();  // "切换网格" 动作
    createHelpMenu();   // 包含 "切换语言"
    createLanguageMenu();
}


void MenuBar::createFileMenu() {
    QMenu* fileMenu = addMenu(tr("File"));
    // --- 添加“新建”项 ---
    QAction* newAction = new QAction(tr("New ..."), this);
    QAction* openAction = new QAction(tr("Open ..."), this); // 加 ... 表示会弹窗
    QAction* saveAction = new QAction(tr("Save ..."), this); // 加 ...
    QAction* exitAction = new QAction(tr("Exit"), this);

    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    connect(newAction, &QAction::triggered, this, &MenuBar::onNewWindow);
    connect(openAction, &QAction::triggered, this, &MenuBar::onOpen);
    connect(saveAction, &QAction::triggered, this, &MenuBar::onSave);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit); // 直接退出应用
}

// --- 实现 createEditMenu ---
void MenuBar::createEditMenu() {
    QMenu* editMenu = addMenu(tr("Edit"));

    // --- 添加撤销/重做动作 ---
    if (m_undoStack) { // 确保 undoStack 有效
        QAction* undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
        undoAction->setShortcut(QKeySequence::Undo); // 设置标准快捷键 Ctrl+Z
        editMenu->addAction(undoAction);

        QAction* redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
        redoAction->setShortcut(QKeySequence::Redo); // 设置标准快捷键 Ctrl+Y / Ctrl+Shift+Z
        editMenu->addAction(redoAction);

        editMenu->addSeparator(); // 分隔符
    }
    else {
        // 如果 undoStack 无效，可以添加禁用的动作或不添加
        QAction* undoAction = new QAction(tr("Undo"), this);
        undoAction->setEnabled(false);
        editMenu->addAction(undoAction);
        QAction* redoAction = new QAction(tr("Redo"), this);
        redoAction->setEnabled(false);
        editMenu->addAction(redoAction);
        editMenu->addSeparator();
    }
    // --- 添加剪切动作 ---
    QAction* cutAction = new QAction(tr("Cut"), this); // T 通常用于 Cut
    cutAction->setShortcut(QKeySequence::Cut); // 设置标准快捷键 Ctrl+X
    connect(cutAction, &QAction::triggered, this, &MenuBar::cutTriggered); // 连接到剪切信号
    editMenu->addAction(cutAction);

    // --- （推荐）将复制/粘贴移到这里 ---
    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MenuBar::copyTriggered);
    editMenu->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MenuBar::pasteTriggered);
    editMenu->addAction(pasteAction);

    // --- 添加删除动作 ---
    QAction* deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete); // 设置标准快捷键 Delete
    connect(deleteAction, &QAction::triggered, this, &MenuBar::deleteTriggered); // 连接到删除信号
    editMenu->addAction(deleteAction);

}



// --- 新增：创建格式菜单 ---
void MenuBar::createFormatMenu() {
    QMenu* formatMenu = addMenu(tr("Format"));

    // --- 图形/文本框 属性 ---
    QAction* changeBorderColorAction = new QAction(tr("Border Color ..."), this);
    QAction* changeFillColorAction = new QAction(tr("Fill Color ..."), this);
    QAction* toggleFillAction = new QAction(tr("Toggle Fill"), this); // 无需弹窗，直接切换
    QAction* changeBorderWidthAction = new QAction(tr("Border Width ..."), this);

    formatMenu->addAction(changeBorderColorAction);
    formatMenu->addAction(changeFillColorAction);
    formatMenu->addAction(toggleFillAction);
    formatMenu->addAction(changeBorderWidthAction);
    formatMenu->addSeparator(); // 分隔图形和线条属性

    // --- 连接线 属性 ---
    QAction* changeLineColorAction = new QAction(tr("Line Color ..."), this);
    QAction* changeLineWidthAction = new QAction(tr("Line Width ..."), this);
    QAction* changeLineStyleAction = new QAction(tr("Line Style..."), this);

    formatMenu->addAction(changeLineColorAction);
    formatMenu->addAction(changeLineWidthAction);
    formatMenu->addAction(changeLineStyleAction);
    formatMenu->addSeparator(); // 分隔线条和文本属性

    // --- 文本框 文本属性 ---
    QAction* changeTextFontAction = new QAction(tr("Text Font ..."), this);
    QAction* changeTextColorAction = new QAction(tr("Text Color ..."), this);

    formatMenu->addAction(changeTextFontAction);
    formatMenu->addAction(changeTextColorAction);

    // --- 连接信号到内部槽 ---
    connect(changeBorderColorAction, &QAction::triggered, this, &MenuBar::onChangeBorderColor);
    connect(changeFillColorAction, &QAction::triggered, this, &MenuBar::onChangeFillColor);
    connect(toggleFillAction, &QAction::triggered, this, &MenuBar::onToggleFill);
    connect(changeBorderWidthAction, &QAction::triggered, this, &MenuBar::onChangeBorderWidth);
    connect(changeLineColorAction, &QAction::triggered, this, &MenuBar::onChangeLineColor);
    connect(changeLineWidthAction, &QAction::triggered, this, &MenuBar::onChangeLineWidth);
    connect(changeLineStyleAction, &QAction::triggered, this, &MenuBar::onChangeLineStyle);
    connect(changeTextFontAction, &QAction::triggered, this, &MenuBar::onChangeTextFont);
    connect(changeTextColorAction, &QAction::triggered, this, &MenuBar::onChangeTextColor);
}

void MenuBar::createAlignMenu() {
    QMenu* alignMenu = addMenu(tr("Align"));

    // --- 垂直对齐 ---
    QAction* alignTopAction = new QAction(tr("Align to Top"), this);
    // 可选：添加图标 QIcon(":/icons/align_top.png")
    connect(alignTopAction, &QAction::triggered, this, &MenuBar::alignTopTriggered);
    alignMenu->addAction(alignTopAction);

    QAction* alignVCenterAction = new QAction(tr("Align Vertically Center"), this);
    // 可选：图标
    connect(alignVCenterAction, &QAction::triggered, this, &MenuBar::alignVCenterTriggered);
    alignMenu->addAction(alignVCenterAction);

    QAction* alignBottomAction = new QAction(tr("Align to Bottom"), this);
    // 可选：图标
    connect(alignBottomAction, &QAction::triggered, this, &MenuBar::alignBottomTriggered);
    alignMenu->addAction(alignBottomAction);

}


void MenuBar::createLayerMenu() {
    QMenu* layerMenu = addMenu(tr("Layer"));
    QAction* moveToTopAction = new QAction(tr("Move to Top Layer"), this);
    QAction* moveToBottomAction = new QAction(tr("Move to Bottom Layer"), this);
    QAction* moveUpOneAction = new QAction(tr("Move Up One Layer"), this);
    QAction* moveDownOneAction = new QAction(tr("Move Down One Layer"), this);

    layerMenu->addAction(moveToTopAction);
    layerMenu->addAction(moveToBottomAction);
    layerMenu->addSeparator();
    layerMenu->addAction(moveUpOneAction);
    layerMenu->addAction(moveDownOneAction);

    connect(moveToTopAction, &QAction::triggered, this, &MenuBar::onMoveToTopLayer);
    connect(moveToBottomAction, &QAction::triggered, this, &MenuBar::onMoveToBottomLayer);
    connect(moveUpOneAction, &QAction::triggered, this, &MenuBar::onMoveUpOneLayer);
    connect(moveDownOneAction, &QAction::triggered, this, &MenuBar::onMoveDownOneLayer);
}

void MenuBar::createSwichMenu() {
    QAction* switchAction = new QAction(tr("Toggle Grid"), this); // 更明确的名称
    this->addAction(switchAction); // 直接添加到菜单栏
    connect(switchAction, &QAction::triggered, this, &MenuBar::switchArea);
}

void MenuBar::createLanguageMenu() {
    QAction*  m_switchLanguageAction = new QAction(tr("Switch Language"), this); // "切换语言"
    this->addAction(m_switchLanguageAction);
    connect(m_switchLanguageAction, &QAction::triggered, this, &MenuBar::onSwitchLanguage);
}

void MenuBar::createHelpMenu() {
    QMenu* helpMenu = addMenu(tr("Help"));
    QAction* aboutAction = new QAction(tr("About"), this);

    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MenuBar::onAbout);
}

// --- 槽函数实现 ---

// --- 添加“新建”槽的实现 ---
void MenuBar::onNewWindow() {
    emit newWindowTriggered(); // 发射 newWindowTriggered 信号
}

void MenuBar::onOpen() {
    emit openTriggered();
}

void MenuBar::onSave() {
    emit saveTriggered();
}



void MenuBar::onAbout() {

    QMessageBox::about(this, tr("About"), tr("Major Assignment: A Graphic Editor Developed Based on QT"));
    // emit aboutTriggered(); // 如果想让主窗口处理
}

void MenuBar::switchArea() {
    emit swichDrawingArea();
}

void MenuBar::onMoveToTopLayer() {
    emit moveToTopLayer();
}

void MenuBar::onMoveToBottomLayer() {
    emit moveToBottomLayer();
}

void MenuBar::onMoveUpOneLayer() {
    emit moveUpOneLayer();
}

void MenuBar::onMoveDownOneLayer() {
    emit moveDownOneLayer();
}

// --- 新增：格式菜单槽函数的实现 ---
void MenuBar::onChangeBorderColor() {
    emit changeBorderColorTriggered();
}

void MenuBar::onChangeFillColor() {
    emit changeFillColorTriggered();
}

void MenuBar::onToggleFill() {
    emit toggleFillTriggered();
}

void MenuBar::onChangeBorderWidth() {
    emit changeBorderWidthTriggered();
}

void MenuBar::onChangeLineColor() {
    emit changeLineColorTriggered();
}

void MenuBar::onChangeLineWidth() {
    emit changeLineWidthTriggered();
}

void MenuBar::onChangeLineStyle() {
    emit changeLineStyleTriggered();
}

void MenuBar::onChangeTextFont() {
    emit changeTextFontTriggered();
}

void MenuBar::onChangeTextColor() {
    emit changeTextColorTriggered();
}

void MenuBar::onSwitchLanguage() {
    if (currentLanguage == AppLanguage::Chinese) { // 直接使用 currentLanguage
        loadLanguage(AppLanguage::English); // 直接调用 loadLanguage
    }
    else {
        loadLanguage(AppLanguage::Chinese);
    }
}