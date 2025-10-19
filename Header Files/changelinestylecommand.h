#ifndef CHANGELINESTYLECOMMAND_H
#define CHANGELINESTYLECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPen> 


class DrawingArea;
class ConnectionLine;

class ChangeLineStyleCommand : public QUndoCommand
{
public:
    
    ChangeLineStyleCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, Qt::PenStyle newStyle, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<ConnectionLine*> m_targetLines; 
    Qt::PenStyle m_newStyle;
    QList<Qt::PenStyle> m_oldStyles;
};

#endif 