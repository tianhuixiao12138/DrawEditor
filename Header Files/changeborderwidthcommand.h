
#ifndef CHANGEBORDERWIDTHCOMMAND_H
#define CHANGEBORDERWIDTHCOMMAND_H

#include <QUndoCommand>
#include <QMap>
#include <QList>
#include <QPointF>

class DrawingArea;
class Shape;
class ChangeBorderWidthCommand : public QUndoCommand {
public:
    explicit ChangeBorderWidthCommand(DrawingArea* area,
        const QList<Shape*>& shapes,
        qreal newWidth,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* myDrawingArea;
    QMap<Shape*, qreal> myOldWidths;     
    QMap<Shape*, QColor> myOldColors;    
    qreal myNewWidth;
};

#endif 