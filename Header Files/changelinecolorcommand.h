#ifndef CHANGELINECOLORCOMMAND_H
#define CHANGELINECOLORCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QColor>


class DrawingArea;
class ConnectionLine;

class ChangeLineColorCommand : public QUndoCommand
{
public:
    
    ChangeLineColorCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, const QColor& newColor, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<ConnectionLine*> m_targetLines; 
    QColor m_newColor;
    QList<QColor> m_oldColors; 
};

#endif 