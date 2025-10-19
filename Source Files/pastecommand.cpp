#include "pastecommand.h"
#include "diagrameditor.h" // 为了访问 DrawingArea
#include "shape.h"
#include "ConnectionLine.h"
#include <QDebug>
#include <QApplication>

PasteCommand::PasteCommand(DrawingArea* drawingArea,
    const QList<Shape*>& shapesToPaste,
    const QList<ConnectionLine*>& linesToPaste,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(drawingArea),
    m_pastedShapes(shapesToPaste),   // 初始时获得所有权
    m_pastedLines(linesToPaste),     // 初始时获得所有权
    m_itemsAddedToArea(false)        // 尚未添加到绘图区
{
    // 设置命令的描述文本，会显示在“编辑”菜单的撤销/重做项中
    setText(QObject::tr("Paste %1 shapes, %2 lines")
        .arg(m_pastedShapes.count())
        .arg(m_pastedLines.count()));
}

PasteCommand::~PasteCommand()
{
    // 重要：仅当项目从未成功添加到绘图区时才删除它们
    // （即 redo 从未被调用，或 undo 被调用了）。
    if (!m_itemsAddedToArea) {
        qDebug() << QObject::tr("PasteCommand destructor is deleting")
            << m_pastedShapes.count()
            << QObject::tr("shapes and")
            << m_pastedLines.count()
            << QObject::tr("lines.");
        qDeleteAll(m_pastedShapes);
        qDeleteAll(m_pastedLines);
    }
    else {
        qDebug() << QObject::tr("PasteCommand destructor: Items are owned by DrawingArea.");
    }
}

// 撤销粘贴操作
void PasteCommand::undo()
{
    if (!m_drawingArea || !m_itemsAddedToArea) {
        qWarning(QObject::tr("PasteCommand::undo() is called in an invalid state.")
            .toUtf8().constData());
        return;
    }
    qDebug() << QObject::tr("Undoing paste...");

    // 1. 清除当前选择
    m_drawingArea->selectedShapes.clear();
    m_drawingArea->selectedLines.clear();

    // 2. 从 DrawingArea 的列表中移除项目
    for (Shape* shape : m_pastedShapes) {
        m_drawingArea->detachConnectionsAttachedTo(shape);
        m_drawingArea->shapes.removeOne(shape);
    }
    for (ConnectionLine* line : m_pastedLines) {
        m_drawingArea->allLines.removeOne(line);
    }

    // 3. 更新 UI 状态
    m_drawingArea->updateSelectionStates();
    m_drawingArea->showControlPoints = false;
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
    m_drawingArea->lineFormatChanged();

    // 4. 标记项目为已移除（这样析构函数可以删除它们）
    m_itemsAddedToArea = false;
    qDebug() << QObject::tr("Paste undo completed.");
}

// 重做粘贴操作（或首次执行粘贴）
void PasteCommand::redo()
{
    if (!m_drawingArea || m_itemsAddedToArea) {
        qWarning(QObject::tr("PasteCommand::redo() is called in an invalid state.")
            .toUtf8().constData());
        return;
    }
    qDebug() << QObject::tr("Redoing paste...");

    // 1. 清除当前选择（以防万一）
    m_drawingArea->selectedShapes.clear();
    m_drawingArea->selectedLines.clear();

    // 2. 将项目添加到 DrawingArea 的列表中
    for (Shape* shape : m_pastedShapes) {
        if (!m_drawingArea->shapes.contains(shape)) {
            m_drawingArea->shapes.append(shape);
        }
        m_drawingArea->expandCanvasIfNeeded(shape->getRect());
    }
    for (ConnectionLine* line : m_pastedLines) {
        if (!m_drawingArea->allLines.contains(line)) {
            m_drawingArea->allLines.append(line);
        }
        m_drawingArea->expandCanvasIfNeeded(line->getBoundingRect().toAlignedRect());
    }

    // 3. 选中新粘贴的项目
    m_drawingArea->selectedShapes = m_pastedShapes;
    m_drawingArea->selectedLines = m_pastedLines;

    // 4. 更新 UI 状态
    m_drawingArea->updateSelectionStates();
    m_drawingArea->showControlPoints = true;
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
    m_drawingArea->lineFormatChanged();

    // 5. 标记项目为已添加（所有权转移给 DrawingArea）
    m_itemsAddedToArea = true;
    qDebug() << QObject::tr("Paste redo completed.");
}