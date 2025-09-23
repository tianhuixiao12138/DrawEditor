#pragma once

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include "app_globals.h"

class MenuBar : public QMenuBar {
    Q_OBJECT

public:
    explicit MenuBar(QUndoStack* undoStack, QWidget* parent = nullptr);

signals:
    void newWindowTriggered();
    void openTriggered();
    void saveTriggered();
    void aboutTriggered();
    void swichDrawingArea();
    void moveToTopLayer();
    void moveToBottomLayer();
    void moveUpOneLayer();
    void moveDownOneLayer();
    void changeBorderColorTriggered();
    void changeFillColorTriggered();
    void toggleFillTriggered();
    void changeBorderWidthTriggered();
    void changeLineColorTriggered();
    void changeLineWidthTriggered();
    void changeLineStyleTriggered();
    void changeTextFontTriggered();
    void changeTextColorTriggered();
    void copyTriggered();
    void pasteTriggered();
    void deleteTriggered();
    void cutTriggered();
    void alignTopTriggered();
    void alignVCenterTriggered();
    void alignBottomTriggered();

private slots:
    void onNewWindow();
    void onOpen();
    void onSave();
    void onAbout();
    void switchArea();
    void onMoveToTopLayer();
    void onMoveToBottomLayer();
    void onMoveUpOneLayer();
    void onMoveDownOneLayer();
    void onChangeBorderColor();
    void onChangeFillColor();
    void onToggleFill();
    void onChangeBorderWidth();
    void onChangeLineColor();
    void onChangeLineWidth();
    void onChangeLineStyle();
    void onChangeTextFont();
    void onChangeTextColor();
    void onSwitchLanguage();

private:
    void createFileMenu();
    void createEditMenu();
    void createHelpMenu();
    void createSwichMenu();
    void createLayerMenu();
    void createFormatMenu();
    void createAlignMenu();
    void createLanguageMenu();
    void retranslateUi();

    QUndoStack* m_undoStack;

protected:
    void changeEvent(QEvent* event) override;
};