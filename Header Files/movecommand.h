#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QMap>
#include <QPair>

class DrawingArea;
class Shape;
class ConnectionLine;

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(DrawingArea* drawingArea,
        const QMap<Shape*, QRectF>& oldShapeRects,
        const QMap<Shape*, QRectF>& newShapeRects,
        const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& oldLinePoints,
        const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& newLinePoints,
        QUndoCommand* parent = nullptr);

    ~MoveCommand() override = default;

    void undo() override;
    void redo() override;

private:
    DrawingArea* myDrawingArea;

    QMap<Shape*, QRectF> m_oldShapeRects;
    QMap<Shape*, QRectF> m_newShapeRects;

    QMap<ConnectionLine*, QPair<QPointF, QPointF>> m_oldLinePoints;
    QMap<ConnectionLine*, QPair<QPointF, QPointF>> m_newLinePoints;

    void applyState(const QMap<Shape*, QRectF>& shapeRects,
        const QMap<ConnectionLine*, QPair<QPointF, QPointF>>& linePoints);
};

#endif // MOVECOMMAND_H