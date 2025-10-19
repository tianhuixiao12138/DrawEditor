#include "diagrameditormeau.h"
#include <QMessageBox>
#include <QApplication>
#include <QMenu>     
#include <QAction>   

MenuBar::MenuBar(QUndoStack* undoStack, QWidget* parent)
    : QMenuBar(parent), m_undoStack(undoStack) // <-- ��ʼ�� m_undoStack
{
    if (!m_undoStack) {
        qWarning("MenuBar: Received null QUndoStack pointer!");
        // ����ѡ���׳��쳣���������������
    }

    createFileMenu();
    createEditMenu();   // <--- �����µı༭�˵���������
    createFormatMenu();
    createAlignMenu();   // <--- ���ö���˵�����
    createLayerMenu();
    createSwichMenu();
    createHelpMenu();
    createLanguageMenu();
}

void MenuBar::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(); // **���� retranslateUi �����ı�**
    }
    QMenuBar::changeEvent(event);
}


void MenuBar::retranslateUi() {
    
    this->clear(); // ����˵����ϵ����ж����˵��Ͷ���

    // ���´������в˵���tr() ��ʹ���µķ�����
    createFileMenu();
    createEditMenu();
    createFormatMenu();
    createAlignMenu();
    createLayerMenu();
    createSwichMenu();  // "�л�����" ����
    createHelpMenu();   // ���� "�л�����"
    createLanguageMenu();
}


void MenuBar::createFileMenu() {
    QMenu* fileMenu = addMenu(tr("File"));
    // --- ��ӡ��½����� ---
    QAction* newAction = new QAction(tr("New ..."), this);
    QAction* openAction = new QAction(tr("Open ..."), this); // �� ... ��ʾ�ᵯ��
    QAction* saveAction = new QAction(tr("Save ..."), this); // �� ...
    QAction* exitAction = new QAction(tr("Exit"), this);

    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    connect(newAction, &QAction::triggered, this, &MenuBar::onNewWindow);
    connect(openAction, &QAction::triggered, this, &MenuBar::onOpen);
    connect(saveAction, &QAction::triggered, this, &MenuBar::onSave);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit); // ֱ���˳�Ӧ��
}

// --- ʵ�� createEditMenu ---
void MenuBar::createEditMenu() {
    QMenu* editMenu = addMenu(tr("Edit"));

    // --- ��ӳ���/�������� ---
    if (m_undoStack) { // ȷ�� undoStack ��Ч
        QAction* undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
        undoAction->setShortcut(QKeySequence::Undo); // ���ñ�׼��ݼ� Ctrl+Z
        editMenu->addAction(undoAction);

        QAction* redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
        redoAction->setShortcut(QKeySequence::Redo); // ���ñ�׼��ݼ� Ctrl+Y / Ctrl+Shift+Z
        editMenu->addAction(redoAction);

        editMenu->addSeparator(); // �ָ���
    }
    else {
        // ��� undoStack ��Ч��������ӽ��õĶ��������
        QAction* undoAction = new QAction(tr("Undo"), this);
        undoAction->setEnabled(false);
        editMenu->addAction(undoAction);
        QAction* redoAction = new QAction(tr("Redo"), this);
        redoAction->setEnabled(false);
        editMenu->addAction(redoAction);
        editMenu->addSeparator();
    }
    // --- ��Ӽ��ж��� ---
    QAction* cutAction = new QAction(tr("Cut"), this); // T ͨ������ Cut
    cutAction->setShortcut(QKeySequence::Cut); // ���ñ�׼��ݼ� Ctrl+X
    connect(cutAction, &QAction::triggered, this, &MenuBar::cutTriggered); // ���ӵ������ź�
    editMenu->addAction(cutAction);

    // --- ���Ƽ���������/ճ���Ƶ����� ---
    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MenuBar::copyTriggered);
    editMenu->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MenuBar::pasteTriggered);
    editMenu->addAction(pasteAction);

    // --- ���ɾ������ ---
    QAction* deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete); // ���ñ�׼��ݼ� Delete
    connect(deleteAction, &QAction::triggered, this, &MenuBar::deleteTriggered); // ���ӵ�ɾ���ź�
    editMenu->addAction(deleteAction);

}



// --- ������������ʽ�˵� ---
void MenuBar::createFormatMenu() {
    QMenu* formatMenu = addMenu(tr("Format"));

    // --- ͼ��/�ı��� ���� ---
    QAction* changeBorderColorAction = new QAction(tr("Border Color ..."), this);
    QAction* changeFillColorAction = new QAction(tr("Fill Color ..."), this);
    QAction* toggleFillAction = new QAction(tr("Toggle Fill"), this); // ���赯����ֱ���л�
    QAction* changeBorderWidthAction = new QAction(tr("Border Width ..."), this);

    formatMenu->addAction(changeBorderColorAction);
    formatMenu->addAction(changeFillColorAction);
    formatMenu->addAction(toggleFillAction);
    formatMenu->addAction(changeBorderWidthAction);
    formatMenu->addSeparator(); // �ָ�ͼ�κ���������

    // --- ������ ���� ---
    QAction* changeLineColorAction = new QAction(tr("Line Color ..."), this);
    QAction* changeLineWidthAction = new QAction(tr("Line Width ..."), this);
    QAction* changeLineStyleAction = new QAction(tr("Line Style..."), this);

    formatMenu->addAction(changeLineColorAction);
    formatMenu->addAction(changeLineWidthAction);
    formatMenu->addAction(changeLineStyleAction);
    formatMenu->addSeparator(); // �ָ��������ı�����

    // --- �ı��� �ı����� ---
    QAction* changeTextFontAction = new QAction(tr("Text Font ..."), this);
    QAction* changeTextColorAction = new QAction(tr("Text Color ..."), this);

    formatMenu->addAction(changeTextFontAction);
    formatMenu->addAction(changeTextColorAction);

    // --- �����źŵ��ڲ��� ---
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

    // --- ��ֱ���� ---
    QAction* alignTopAction = new QAction(tr("Align to Top"), this);
    // ��ѡ�����ͼ�� QIcon(":/icons/align_top.png")
    connect(alignTopAction, &QAction::triggered, this, &MenuBar::alignTopTriggered);
    alignMenu->addAction(alignTopAction);

    QAction* alignVCenterAction = new QAction(tr("Align Vertically Center"), this);
    // ��ѡ��ͼ��
    connect(alignVCenterAction, &QAction::triggered, this, &MenuBar::alignVCenterTriggered);
    alignMenu->addAction(alignVCenterAction);

    QAction* alignBottomAction = new QAction(tr("Align to Bottom"), this);
    // ��ѡ��ͼ��
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
    QAction* switchAction = new QAction(tr("Toggle Grid"), this); // ����ȷ������
    this->addAction(switchAction); // ֱ����ӵ��˵���
    connect(switchAction, &QAction::triggered, this, &MenuBar::switchArea);
}

void MenuBar::createLanguageMenu() {
    QAction*  m_switchLanguageAction = new QAction(tr("Switch Language"), this); // "�л�����"
    this->addAction(m_switchLanguageAction);
    connect(m_switchLanguageAction, &QAction::triggered, this, &MenuBar::onSwitchLanguage);
}

void MenuBar::createHelpMenu() {
    QMenu* helpMenu = addMenu(tr("Help"));
    QAction* aboutAction = new QAction(tr("About"), this);

    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MenuBar::onAbout);
}

// --- �ۺ���ʵ�� ---

// --- ��ӡ��½����۵�ʵ�� ---
void MenuBar::onNewWindow() {
    emit newWindowTriggered(); // ���� newWindowTriggered �ź�
}

void MenuBar::onOpen() {
    emit openTriggered();
}

void MenuBar::onSave() {
    emit saveTriggered();
}



void MenuBar::onAbout() {

    QMessageBox::about(this, tr("About"), tr("Major Assignment: A Graphic Editor Developed Based on QT"));
    // emit aboutTriggered(); // ������������ڴ���
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

// --- ��������ʽ�˵��ۺ�����ʵ�� ---
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
    if (currentLanguage == AppLanguage::Chinese) { // ֱ��ʹ�� currentLanguage
        loadLanguage(AppLanguage::English); // ֱ�ӵ��� loadLanguage
    }
    else {
        loadLanguage(AppLanguage::Chinese);
    }
}