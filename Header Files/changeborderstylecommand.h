
#ifndef CHANGEBORDERSTYLECOMMAND_H
#define CHANGEBORDERSTYLECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>


class DrawingArea; 
class Shape;       

class ChangeBorderStyleCommand : public QUndoCommand
{
public:
    ChangeBorderStyleCommand(DrawingArea* area,
        const QList<Shape*>& shapes,
        Qt::PenStyle newStyle,
        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<Shape*> m_targetShapes; 
    Qt::PenStyle m_newStyle;
    QMap<Shape*, Qt::PenStyle> m_oldStyles;
};

#endif