#include "movecommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include "ConnectionLine.h"
#include <QDebug>
#include <QApplication>

// --- 修改构造函数 ---
MoveCommand::MoveCommand(DrawingArea* drawingArea,
    const QMap<Shape*, QRectF>& oldShapeRects,
    const QMap<Shape*, QRectF>& newShapeRects,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& oldLinePoints,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& newLinePoints,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    m_oldShapeRects(oldShapeRects), // 复制 Map
    m_newShapeRects(newShapeRects),
    m_oldLinePoints(oldLinePoints),
    m_newLinePoints(newLinePoints)
{
    if (!myDrawingArea) {
        qWarning() << QObject::tr("MoveCommand Error: DrawingArea is null.");
        return;
    }

    int totalItems = m_oldShapeRects.count() + m_oldLinePoints.count();
    if (totalItems == 0) {
        qWarning() << QObject::tr("MoveCommand created with no items.");
        return;
    }

    qDebug() << QObject::tr("MoveCommand created for")
        << m_oldShapeRects.count()
        << QObject::tr("shapes and")
        << m_oldLinePoints.count()
        << QObject::tr("lines.");

    setText(QObject::tr("Move %n item(s)", "", totalItems)); // 支持复数形式
}

// --- 辅助函数：应用指定的状态 ---
void MoveCommand::applyState(const QMap<Shape*, QRectF>& shapeRects,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& linePoints)
{
    if (!myDrawingArea) return;

    // 应用图形状态 (设置矩形)
    for (auto it = shapeRects.constBegin(); it != shapeRects.constEnd(); ++it) {
        Shape* shape = it.key();
        const QRectF& rect = it.value();
        if (shape) {
            shape->setRect(rect.toAlignedRect()); // 转换为整数矩形
        }
        else {
            qWarning() << QObject::tr("MoveCommand::applyState found null Shape pointer in map.");
        }
    }

    // 应用连接线状态 (设置自由端点)
    for (auto it = linePoints.constBegin(); it != linePoints.constEnd(); ++it) {
        ConnectionLine* line = it.key();
        const QPointF& startPos = it.value().first;
        const QPointF& endPos = it.value().second;
        if (line) {
            // 只有当端点是自由时才设置
            if (!line->isStartAttached()) {
                line->setFreeStartPoint(startPos);
            }
            if (!line->isEndAttached()) {
                line->setFreeEndPoint(endPos);
            }
        }
        else {
            qWarning() << QObject::tr("MoveCommand::applyState found null ConnectionLine pointer in map.");
        }
    }

    // 应用状态后需要更新界面和可能的格式面板
    myDrawingArea->update();
    //myDrawingArea->shapeFormatChanged(); // 通知 FormatPanel
    //myDrawingArea->lineFormatChanged();
}

// --- undo(): 应用旧状态 ---
void MoveCommand::undo()
{
    qDebug() << QObject::tr("Undo MoveCommand: Applying old state.");
    applyState(m_oldShapeRects, m_oldLinePoints);
}

// --- redo(): 应用新状态 ---
void MoveCommand::redo()
{
    qDebug() << QObject::tr("Redo MoveCommand: Applying new state.");
    applyState(m_newShapeRects, m_newLinePoints);
    myDrawingArea->shapeFormatChanged();
    myDrawingArea->lineFormatChanged();
}

// --- mergeWith 对于状态命令通常不适用，保持默认或移除 ---
// bool MoveCommand::mergeWith(const QUndoCommand *other) { return false; }