#ifndef RESIZESHAPECOMMAND_H
#define RESIZESHAPECOMMAND_H

#include <QUndoCommand>
#include <QRectF>

class DrawingArea;
class Shape;

class ResizeShapeCommand : public QUndoCommand
{
public:
    ResizeShapeCommand(DrawingArea* area, Shape* shape, const QRectF& newRect, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    Shape* m_targetShape;
    QRectF m_oldRect;
    QRectF m_newRect;
};

#endif // RESIZESHAPECOMMAND_H