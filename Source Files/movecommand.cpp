#include "movecommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include "ConnectionLine.h"
#include <QDebug>
#include <QApplication>

// --- �޸Ĺ��캯�� ---
MoveCommand::MoveCommand(DrawingArea* drawingArea,
    const QMap<Shape*, QRectF>& oldShapeRects,
    const QMap<Shape*, QRectF>& newShapeRects,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& oldLinePoints,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& newLinePoints,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    m_oldShapeRects(oldShapeRects), // ���� Map
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

    setText(QObject::tr("Move %n item(s)", "", totalItems)); // ֧�ָ�����ʽ
}

// --- ����������Ӧ��ָ����״̬ ---
void MoveCommand::applyState(const QMap<Shape*, QRectF>& shapeRects,
    const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& linePoints)
{
    if (!myDrawingArea) return;

    // Ӧ��ͼ��״̬ (���þ���)
    for (auto it = shapeRects.constBegin(); it != shapeRects.constEnd(); ++it) {
        Shape* shape = it.key();
        const QRectF& rect = it.value();
        if (shape) {
            shape->setRect(rect.toAlignedRect()); // ת��Ϊ��������
        }
        else {
            qWarning() << QObject::tr("MoveCommand::applyState found null Shape pointer in map.");
        }
    }

    // Ӧ��������״̬ (�������ɶ˵�)
    for (auto it = linePoints.constBegin(); it != linePoints.constEnd(); ++it) {
        ConnectionLine* line = it.key();
        const QPointF& startPos = it.value().first;
        const QPointF& endPos = it.value().second;
        if (line) {
            // ֻ�е��˵�������ʱ������
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

    // Ӧ��״̬����Ҫ���½���Ϳ��ܵĸ�ʽ���
    myDrawingArea->update();
    //myDrawingArea->shapeFormatChanged(); // ֪ͨ FormatPanel
    //myDrawingArea->lineFormatChanged();
}

// --- undo(): Ӧ�þ�״̬ ---
void MoveCommand::undo()
{
    qDebug() << QObject::tr("Undo MoveCommand: Applying old state.");
    applyState(m_oldShapeRects, m_oldLinePoints);
}

// --- redo(): Ӧ����״̬ ---
void MoveCommand::redo()
{
    qDebug() << QObject::tr("Redo MoveCommand: Applying new state.");
    applyState(m_newShapeRects, m_newLinePoints);
    myDrawingArea->shapeFormatChanged();
    myDrawingArea->lineFormatChanged();
}

// --- mergeWith ����״̬����ͨ�������ã�����Ĭ�ϻ��Ƴ� ---
// bool MoveCommand::mergeWith(const QUndoCommand *other) { return false; }